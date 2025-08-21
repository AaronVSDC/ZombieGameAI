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
	UpdateInventoryInfo();
	UpdateHouseMemory(); 
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
		m_FrontierWanderTimer = 0.f;
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
		if (m_FrontierWanderTimer <= 0.f)
			m_FrontierWanderTimer = m_FrontierWanderDuration;
	}

	if (m_FrontierWanderTimer > 0.f)
	{
		m_FrontierWanderTimer -= dt;
		steering.LinearVelocity = m_pSteeringBehaviour->Wander(m_pBB->agent, 2.f, 0.f) * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
		if (m_FrontierWanderTimer <= 0.f)
			m_Target = m_pGrid->GetNextFrontierTarget();
		return steering;
	} 

	Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(m_Target);
	Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
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

	constexpr float houseArriveDistance = 3.f;
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

	// Find closest enemy in sight
	const EnemyInfo* closest = nullptr;
	float bestDist = FLT_MAX;
	for (auto const& e : m_pBB->enemies)
	{
		float d = (e.Location - m_pBB->agent.Position).MagnitudeSquared();
		if (d < bestDist) { bestDist = d; closest = &e; }
	}

	steering.AutoOrient = false;
	steering.LinearVelocity = -m_pBB->agent.LinearVelocity;

	if (closest)
	{
		// we have a target
		m_pBB->lastEnemy = *closest;
		m_pBB->lastEnemyValid = true;

		Elite::Vector2 toEnemy = closest->Location - m_pBB->agent.Position;
		float desiredOrientation = atan2f(toEnemy.y, toEnemy.x);
		float orientationDiff = atan2f(sinf(desiredOrientation - m_pBB->agent.Orientation),
			cosf(desiredOrientation - m_pBB->agent.Orientation));

		if (fabsf(orientationDiff) > 0.05f)
		{
			steering.AngularVelocity =
				Elite::Clamp(orientationDiff, -1.f, 1.f) * m_pBB->agent.MaxAngularSpeed;
		}
		else
		{
			steering.AngularVelocity = 0.f;

			// shoot if we can 
			if (m_pBB->weaponSlot >= 0 && m_pBB->weaponAmmo > 0)
			{
				m_pInterface->Inventory_UseItem(static_cast<UINT>(m_pBB->weaponSlot));
				m_pBB->attackLatched = false;
			}
		}
	}
	else
	{
	   // No enemy in sight: rotate to search
		const float omega = m_pBB->agent.MaxAngularSpeed;
		steering.AngularVelocity = omega;

		m_SearchRotationAccumulation += fabsf(omega) * dt;

		if (m_SearchRotationAccumulation >= Elite::ToRadians(360.f))
		{
			m_pBB->attackLatched = false;
			m_SearchRotationAccumulation = 0.f; 
		}
	}

	return steering;
}
#pragma endregion

#pragma region EVADE_ENEMY
SteeringPlugin_Output FiniteStateMachine::UpdateEvadeEnemy(float dt)
{
	SteeringPlugin_Output steering{};

	//FLEE FROM CLOSEST ENEMY (OR LAST KNOWN ENEMY WHEN NOT IN FOV)
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
		// Remember enemy in case it leaves the FOV next frame
		m_pBB->lastEnemy = *closest;
		m_pBB->lastEnemyValid = true;
	}
	else if (m_pBB->agent.WasBitten) 
	{
		float angle = m_pBB->agent.Orientation;
		Elite::Vector2 forward{ cosf(angle), sinf(angle) };
		m_pBB->lastEnemy.Location = m_pBB->agent.Position - forward;
		m_pBB->lastEnemy.LinearVelocity = Elite::ZeroVector2;
		m_pBB->lastEnemyValid = true;
		closest = &m_pBB->lastEnemy;
	}
	else if (m_pBB->lastEnemyValid)
	{
		closest = &m_pBB->lastEnemy;
	}

	if (closest)
	{
		// Calculate a target position in the opposite direction of the threat and
		// seek that position using the navmesh. This prevents the agent from
		// continuously pushing against obstacles when evading. 
		Elite::Vector2 fleeDir = m_pSteeringBehaviour->Evade(
			m_pBB->agent, closest->Location, closest->LinearVelocity);

		// Move far enough away from the threat
		constexpr float fleeTargetDistance = 15.f;
		Elite::Vector2 fleeTarget = m_pBB->agent.Position + fleeDir * fleeTargetDistance;

		Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(fleeTarget);
		Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);

		steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
		if (m_WantsToRun)
		{
			if (m_pBB->agent.Stamina <= 0.1f)
				m_WantsToRun = false;   
		}
		else
		{
			if (m_pBB->agent.Stamina >= 9.9f)
				m_WantsToRun = true;    
		}

		steering.RunMode = m_WantsToRun;
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
				if (m_pBB->freeSlot >= 0 && m_pBB->freeSlot < static_cast<int>(m_pBB->inventory.size()))
				{
					m_pBB->inventory[m_pBB->freeSlot] = item.Type;
					m_pBB->freeSlot = -1;
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
}
void FiniteStateMachine::InitAndUpdateGrid()
{
	if (!m_pGrid->IsInitialized()) m_pGrid->InitGrid(m_pBB.get());
	if (!m_HasEnteredFirstState) OnEnter(), m_HasEnteredFirstState = true;
	m_pGrid->UpdateFOVGrid();

}
SteeringPlugin_Output FiniteStateMachine::UpdateStates(float dt)
{


	AgentState next = m_pStateDecider->Decide(m_CurrentState, m_pBB.get(), dt);
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



void FiniteStateMachine::UpdateInventoryInfo()
{
	m_pBB->hasWeapon = false;
	m_pBB->weaponSlot = -1;
	m_pBB->weaponAmmo = 0;
	m_pBB->freeSlot = -1;

	const int invCap = static_cast<int>(m_pInterface->Inventory_GetCapacity());
	if (m_pBB->inventory.size() != static_cast<size_t>(invCap))
		m_pBB->inventory.assign(invCap, eItemType::GARBAGE);

	for (int i = 0; i < invCap; ++i)
	{
		if (m_pBB->inventory[i] != eItemType::GARBAGE)
		{
			ItemInfo invItem{};

			if (m_pInterface->Inventory_GetItem(static_cast<UINT>(i), invItem))
			{
				m_pBB->inventory[i] = invItem.Type;

				if (invItem.Value == 0)
				{
					m_pInterface->Inventory_RemoveItem(static_cast<UINT>(i));
					m_pBB->inventory[i] = eItemType::GARBAGE;
				}

				if (invItem.Type == eItemType::PISTOL || invItem.Type == eItemType::SHOTGUN)
				{

					if (m_pBB->weaponSlot == -1)
					{
						m_pBB->hasWeapon = true;
						m_pBB->weaponSlot = i;
						m_pBB->weaponAmmo = invItem.Value;
					}
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

	constexpr float lowThreshold{ 5.f };
	if (m_pBB->agent.Energy < lowThreshold)
	{
		for (int i = 0; i < invCap; ++i)
		{
			if (m_pBB->inventory[i] == eItemType::FOOD)
			{
				m_pInterface->Inventory_UseItem(static_cast<UINT>(i));
				m_pBB->inventory[i] = eItemType::GARBAGE;
				break;
			}
		}
	}
	if (m_pBB->agent.Health < lowThreshold)
	{
		for (int i = 0; i < invCap; ++i)
		{
			if (m_pBB->inventory[i] == eItemType::MEDKIT)
			{
				m_pInterface->Inventory_UseItem(static_cast<UINT>(i));
				m_pBB->inventory[i] = eItemType::GARBAGE;
				break;
			}
		}
	}
}

void FiniteStateMachine::UpdateHouseMemory()
{
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
#pragma endregion