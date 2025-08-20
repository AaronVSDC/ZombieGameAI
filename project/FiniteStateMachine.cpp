#include "stdafx.h"
#include "FiniteStateMachine.h"

FiniteStateMachine::FiniteStateMachine(IExamInterface* pInterface) :
	m_pInterface{ pInterface },
	m_pBB{ std::make_unique<Blackboard>() },
	m_pStateDecider{ std::make_unique<StateDecider>() },
	m_pGrid{ std::make_unique<Grid>() },
	m_pSteeringBehaviour{ std::make_unique<SteeringBehaviour>() },
	m_CurrentState{ AgentState::Explore }
{
}

SteeringPlugin_Output FiniteStateMachine::Update(float dt)
{
	PopulateBlackboard();
	InitAndUpdateGrid();
	return UpdateStates(dt);

}
void FiniteStateMachine::DebugRender() const
{
	m_pGrid->DebugDraw(m_pInterface);
	m_pInterface->Draw_SolidCircle(m_Target, 1.f, {}, { 1, 0, 0 });

}

void FiniteStateMachine::OnEnter()
{
	switch (m_CurrentState)
	{
	case AgentState::Explore:
	{
		std::cout << "Explore" << std::endl;

		m_pGrid->UpdateFOVGrid();
		m_Target = m_pGrid->GetNextFrontierTarget();
		break;
	}
	case AgentState::GoToHouse:
	{
		std::cout << "GoToHouse" << std::endl;

		const auto& houses = m_pBB->houses;
		auto& agentPos = m_pBB->agent.Position;

		float bestDistSqr = FLT_MAX;
		Elite::Vector2 bestHouse{};

		for (auto const& h : houses)
		{

			float d = (h.Center - agentPos).MagnitudeSquared();
			if (d < bestDistSqr)
			{
				bestDistSqr = d;
				bestHouse = h.Center;
			}
		}
		m_Target = bestHouse;
		break;
	}
	case AgentState::Attack:
	{
		std::cout << "Attack" << std::endl;
		break;
	}
	case AgentState::EvadeEnemy:
	{
		std::cout << "EvadeEnemy" << std::endl;
		break;
	}
	case AgentState::PickupLoot:
	{
		std::cout << "PickupLoot" << std::endl;
		break;
	}
	}
}

void FiniteStateMachine::OnExit()
{
}

#pragma region EXPLORE
SteeringPlugin_Output FiniteStateMachine::UpdateExplore(float dt)
{
	SteeringPlugin_Output steering{};


	const float arriveDist = m_pGrid->GetCellSize() * 0.5f;
	if ((m_pBB->agent.Position - m_Target).Magnitude() < arriveDist)
	{
		m_Target = m_pGrid->GetNextFrontierTarget();
	}

	////if no frontiers -> wander
	//if ((m_Target - m_pBB->agent.Position).MagnitudeSquared() < 1e-4f)
	//{
	//	steering.LinearVelocity = m_pSteeringBehaviour->Wander(m_pBB->agent);
	//	return steering;
	//}

	Elite::Vector2  navPt = m_pInterface->NavMesh_GetClosestPathPoint(m_Target);
	Elite::Vector2  desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
	steering.AutoOrient = true;
	steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;

	return steering;
}
#pragma endregion
#pragma region GO_TO_HOUSE
SteeringPlugin_Output FiniteStateMachine::UpdateGoToHouse(float dt)
{
	SteeringPlugin_Output steering{};
	const auto agentInfo = m_pInterface->Agent_GetInfo();

	const Elite::Vector2 currentWaypoint =
		m_pInterface->NavMesh_GetClosestPathPoint(m_pBB->currentHouseTarget);

	constexpr float waypointArriveRadius = 1.5f;
	const float distToWP2 = (currentWaypoint - agentInfo.Position).MagnitudeSquared();
	if (distToWP2 < waypointArriveRadius * waypointArriveRadius)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}
	else
	{
		Elite::Vector2 desiredDir = m_pSteeringBehaviour->Seek(agentInfo, currentWaypoint);
		steering.LinearVelocity = desiredDir * agentInfo.MaxLinearSpeed;
	}
	steering.AutoOrient = true;

	constexpr float houseArriveDistance = 0.5f;
	if (Elite::Distance(agentInfo.Position, m_pBB->currentHouseTarget) < houseArriveDistance)
	{
		m_pBB->visitedHouseCenters.push_back(m_pBB->currentHouseTarget);
		m_pBB->hasHouseTarget = false;
	}
	return steering;
}
#pragma endregion
#pragma region ATTACK
SteeringPlugin_Output FiniteStateMachine::UpdateAttack(float dt)
{
	SteeringPlugin_Output steering{};
	if (m_pBB->enemies.empty())
		return steering;

	//FIND CLOSEST ENEMY
	const EnemyInfo* closest = nullptr;
	float bestDist = FLT_MAX;
	for (auto const& e : m_pBB->enemies)
	{
		float d = (e.Location - m_pBB->agent.Position).MagnitudeSquared();
		if (d < bestDist)
		{
			bestDist = d; 
			closest = &e;
		}
	}

	if (closest)
	{
		//IF CLOSE ENOUGH, STOP AND SHOOT
		const float attackRange = m_pBB->agent.GrabRange * 2.f;
		if (bestDist < attackRange * attackRange)
		{
			steering.LinearVelocity = Elite::ZeroVector2;
			steering.AutoOrient = true;
			if (m_pBB->weaponSlot >= 0)
				m_pInterface->Inventory_UseItem(static_cast<UINT>(m_pBB->weaponSlot));
		}
		else
		{
			Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(closest->Location);
			Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt); //TODO: maybe change to pursuit, might be better
			steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
			steering.AutoOrient = true;
		}
	}
	return steering;
}
#pragma endregion

#pragma region EVADE_ENEMY
SteeringPlugin_Output FiniteStateMachine::UpdateEvadeEnemy(float dt)
{
	SteeringPlugin_Output steering{};
	if (m_pBB->enemies.empty())
		return steering;

	//FLEE FROM CLOSEST ENEMY
	const EnemyInfo* closest = nullptr;
	float bestDist = FLT_MAX;
	for (auto const& e : m_pBB->enemies)
	{
		float d = (e.Location - m_pBB->agent.Position).MagnitudeSquared();
		if (d < bestDist)
		{
			bestDist = d;
			closest = &e;
		}
	}

	if (closest)
	{
		Elite::Vector2 fleeDir = m_pSteeringBehaviour->Flee(m_pBB->agent, closest->Location);
		steering.LinearVelocity = fleeDir * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
	}
	return steering;
}
#pragma endregion 
#pragma region PICKUP_LOOT
SteeringPlugin_Output FiniteStateMachine::PickupLoot(float dt)
{
	SteeringPlugin_Output steering{};
	if (m_pBB->items.empty() || m_pBB->freeSlot < 0)
		return steering;

	//FIND CLOSEST NON GARBAGE ITEM
	const ItemInfo* closest = nullptr;
	float bestDist = FLT_MAX;
	for (auto const& i : m_pBB->items)
	{
		if (i.Type == eItemType::GARBAGE)
			continue;
		float d = (i.Location - m_pBB->agent.Position).MagnitudeSquared();
		if (d < bestDist)
		{
			bestDist = d;
			closest = &i;
		}
	}

	if (!closest)
		return steering; 

	if (bestDist < m_pBB->agent.GrabRange * m_pBB->agent.GrabRange)
	{
		ItemInfo item = *closest;
		if (m_pInterface->GrabItem(item))
		{
			if (m_pInterface->Inventory_AddItem(static_cast<UINT>(m_pBB->freeSlot), item))
			{
				//Track added item so we do not try to use the same slot again
				if (m_pBB->freeSlot >= 0 && m_pBB->freeSlot < static_cast<int>(m_pBB->inventory.size()))
				{
					m_pBB->inventory[m_pBB->freeSlot] = item.Type;
				}
			}
		}
	}
	else
	{
		Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(closest->Location);
		Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
		steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
	}
	return steering;
}
#pragma endregion


#pragma region HELPER
void FiniteStateMachine::PopulateBlackboard()
{
	m_pBB->agent = m_pInterface->Agent_GetInfo();
	m_pBB->enemies = m_pInterface->GetEnemiesInFOV();
	m_pBB->items = m_pInterface->GetItemsInFOV();
	m_pBB->houses = m_pInterface->GetHousesInFOV();
	m_pBB->worldInfo = m_pInterface->World_GetInfo();

	m_pBB->hasWeapon = false;
	m_pBB->weaponSlot = -1;
	m_pBB->freeSlot = -1;
	const int invCap = static_cast<int>(m_pInterface->Inventory_GetCapacity());

	if (m_pBB->inventory.size() != static_cast<size_t>(invCap))
		m_pBB->inventory.assign(invCap, eItemType::GARBAGE);

	//Only query slots that we believe contain an item
	for (int i = 0; i < invCap; ++i)
	{
		if (m_pBB->inventory[i] != eItemType::GARBAGE)
		{
			ItemInfo invItem{};
			if (m_pInterface->Inventory_GetItem(static_cast<UINT>(i), invItem))
			{
				m_pBB->inventory[i] = invItem.Type;

				if ((invItem.Type == eItemType::PISTOL || invItem.Type == eItemType::SHOTGUN) && m_pBB->weaponSlot == -1)
				{
					m_pBB->hasWeapon = true;
					m_pBB->weaponSlot = i;
				}
			}
			else
			{
				m_pBB->inventory[i] = eItemType::GARBAGE;
			}
		}

		if (m_pBB->inventory[i] == eItemType::GARBAGE && m_pBB->freeSlot == -1)
			m_pBB->freeSlot = i;
	}
	for (auto const& h : m_pBB->houses)
	{
		bool seen = false;
		for (auto const& known : m_pBB->knownHouseCenters)
			if ((known - h.Center).MagnitudeSquared() < 0.01f)
			{
				seen = true;
				break;
			}
		if (!seen)
			m_pBB->knownHouseCenters.push_back(h.Center);
	}
}
void FiniteStateMachine::InitAndUpdateGrid()
{
	if (!m_pGrid->IsInitialized()) m_pGrid->InitGrid(m_pBB.get());
	if (!m_HasEnteredFirstState) OnEnter(), m_HasEnteredFirstState = true;
	m_pGrid->UpdateFOVGrid();

}
SteeringPlugin_Output FiniteStateMachine::UpdateStates(float dt)
{


	AgentState next = m_pStateDecider->Decide(m_CurrentState, m_pBB.get());
	if (next != m_CurrentState)
		OnExit(), m_CurrentState = next, OnEnter();

	switch (m_CurrentState)
	{
	case AgentState::Explore:
	{
		return UpdateExplore(dt);
		break;
	}
	case AgentState::GoToHouse:
	{
		return UpdateGoToHouse(dt);
		break;
	}
	case AgentState::Attack:
	{
		return UpdateAttack(dt);
		break;
	}
	case AgentState::EvadeEnemy:
	{
		return UpdateEvadeEnemy(dt);
		break;
	}
	case AgentState::PickupLoot:
	{
		return PickupLoot(dt);
		break;
	}
	}
	return SteeringPlugin_Output();


}
#pragma endregion