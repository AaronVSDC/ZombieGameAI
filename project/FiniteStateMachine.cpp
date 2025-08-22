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
	UpdatePurgeZoneMemory(dt);
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
		 
		if (m_RadialTargets.empty())
		{
			m_ExploreOrigin = m_pBB->worldInfo.Center;
			GenerateRadialTargets();
		}

		SetNextExploreTarget();
		m_FrontierWanderTimer = 0.f;
		break;
	}
	case AgentState::EvadePurgeZone:
	{
		std::cout << "EvadePurgeZone" << std::endl;
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
	case AgentState::ExploreHouse:
	{
		std::cout << "ExploreHouse" << std::endl;
		if (!m_IsExploringHouse)
		{
			m_IsExploringHouse = true;
			m_HouseItems.clear();
			m_HouseExploreTargets.clear();
			m_CurrentHouseExploreIndex = 0;
			m_HouseExplorationComplete = false;

			const auto& houses = m_pBB->houses;
			float bestDistSqr = FLT_MAX;
			HouseInfo current{};
			for (auto const& h : houses)
			{
				float d = (h.Center - m_pBB->agent.Position).MagnitudeSquared();
				if (d < bestDistSqr)
				{
					bestDistSqr = d;
					current = h;
				}
			}

			// Record this house so we don't explore it again
			m_pBB->currentHouseTarget = current.Center;
			m_pBB->visitedHouseCenters.push_back(current.Center);
			m_pBB->hasHouseTarget = false;

			Elite::Vector2 half = current.Size * 0.5f;
			Elite::Vector2 offset = half * 0.5f;
			m_HouseExploreTargets.push_back(current.Center + Elite::Vector2{ -offset.x, -offset.y });
			m_HouseExploreTargets.push_back(current.Center + Elite::Vector2{ offset.x, -offset.y });
			m_HouseExploreTargets.push_back(current.Center + Elite::Vector2{ offset.x, offset.y });
			m_HouseExploreTargets.push_back(current.Center + Elite::Vector2{ -offset.x, offset.y });
			m_HouseExitTarget = current.Center + Elite::Vector2{ current.Size.x, 0.f };

			if (!m_HouseExploreTargets.empty())
				m_Target = m_HouseExploreTargets[0];
		}
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
	case AgentState::UseItem:
	{ 
		std::cout << "UseItem" << std::endl;
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

	if (IsPointInPurgeZone(m_Target))
		SetNextExploreTarget();

	const float arriveDist = m_pGrid->GetCellSize() * 0.5f;
	if ((m_pBB->agent.Position - m_Target).Magnitude() < arriveDist)
	{
		++m_RadialTargetIndex;
		if (m_RadialTargetIndex >= m_RadialTargets.size())
		{
			Elite::Vector2 frontier = m_pGrid->GetNextFrontierTarget();
			float frontierRadius = (frontier - m_ExploreOrigin).Magnitude();
			m_CurrentRadius = max(m_CurrentRadius + m_RadiusIncrement, frontierRadius);
			GenerateRadialTargets();
		}
		SetNextExploreTarget();
	} 

	Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(m_Target);
	Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
	steering.AutoOrient = true;
	steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
	EnableSprint(steering);
	return steering;
}
void FiniteStateMachine::SetNextExploreTarget()
{
	size_t checked = 0;
	while ((m_pGrid->IsCellVisited(m_RadialTargets[m_RadialTargetIndex]) ||
		IsPointInPurgeZone(m_RadialTargets[m_RadialTargetIndex])) &&
		checked < m_NumRadialTargets)
	{
		++m_RadialTargetIndex;
		++checked;
		if (m_RadialTargetIndex >= m_RadialTargets.size())
		{
			Elite::Vector2 frontier = m_pGrid->GetNextFrontierTarget();
			float frontierRadius = (frontier - m_ExploreOrigin).Magnitude();
			m_CurrentRadius = max(m_CurrentRadius + m_RadiusIncrement, frontierRadius);
			GenerateRadialTargets();
			checked = 0;
		}
	}

	m_Target = m_RadialTargets[m_RadialTargetIndex];
}
void FiniteStateMachine::GenerateRadialTargets()
{
	m_RadialTargets.clear(); 

	Elite::Vector2 center = m_ExploreOrigin;
	float angleStep = Elite::ToRadians(360.f / static_cast<float>(m_NumRadialTargets));
	for (size_t i = 0; i < m_NumRadialTargets; ++i)
	{ 
		float angle = angleStep * static_cast<float>(i);
		Elite::Vector2 offset{ cosf(angle), sinf(angle) };
		m_RadialTargets.push_back(center + offset * m_CurrentRadius);
	}

	m_RadialTargetIndex = 0;

}
#pragma endregion
#pragma region EVADE_PURGE_ZONE
SteeringPlugin_Output FiniteStateMachine::UpdateEvadePurgeZone(float /*dt*/)
{
	SteeringPlugin_Output steering{};
	if (m_pBB->purgeZones.empty())
		return steering;

	const TrackedPurgeZone* closest = nullptr;
	float bestDist = FLT_MAX;
	for (auto const& z : m_pBB->purgeZones)
	{
		float d = (m_pBB->agent.Position - z.info.Center).Magnitude();
		if (d < z.info.Radius && d < bestDist)
		{
			bestDist = d;
			closest = &z;
		}
	}

	if (closest)
	{
		Elite::Vector2 dir = (m_pBB->agent.Position - closest->info.Center).GetNormalized();
		float targetDist = closest->info.Radius + 1.f;
		Elite::Vector2 fleeTarget = closest->info.Center + dir * targetDist;
		Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(fleeTarget);
		Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
		steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
		EnableSprint(steering);
	}
	return steering;
}
#pragma endregion

#pragma region GO_TO_HOUSE
SteeringPlugin_Output FiniteStateMachine::UpdateGoToHouse(float dt)
{
	SteeringPlugin_Output steering{};

	if (IsPointInPurgeZone(m_pBB->currentHouseTarget))
	{
		m_pBB->hasHouseTarget = false;
		return steering;
	}

	const auto agentInfo = m_pInterface->Agent_GetInfo();

	const Elite::Vector2 currentWaypoint =
		m_pInterface->NavMesh_GetClosestPathPoint(m_pBB->currentHouseTarget);

	Elite::Vector2 desiredDir = m_pSteeringBehaviour->Seek(agentInfo, currentWaypoint);
	steering.LinearVelocity = desiredDir * agentInfo.MaxLinearSpeed;
	steering.AutoOrient = true;
	EnableSprint(steering);
	return steering;
}
#pragma endregion
#pragma region EXPLORE_HOUSE
SteeringPlugin_Output FiniteStateMachine::UpdateExploreHouse(float dt)
{
	SteeringPlugin_Output steering{};
	m_pBB->hasHouseTarget = false;

	if (!m_HouseExplorationComplete && m_CurrentHouseExploreIndex < m_HouseExploreTargets.size())
	{
		Elite::Vector2 target = m_HouseExploreTargets[m_CurrentHouseExploreIndex];
		Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(target);
		Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
		steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;

		if (Elite::Distance(m_pBB->agent.Position, target) < 1.f)
		{
			++m_CurrentHouseExploreIndex;
			if (m_CurrentHouseExploreIndex >= m_HouseExploreTargets.size())
				m_HouseExplorationComplete = true;
		}
		return steering;
	}

	if (m_pBB->agent.IsInHouse) 
	{
		Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(m_HouseExitTarget);
		Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
		steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
		return steering;
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

	//FLEE FROM ALL ENEMIES (OR LAST KNOWN ENEMY WHEN NOT IN FOV)
	const EnemyInfo* closest = nullptr;
	float bestDist = FLT_MAX;
	Elite::Vector2 fleeDir{};

	if (!m_pBB->enemies.empty())
	{
		for (auto const& e : m_pBB->enemies)
		{
			float d = (e.Location - m_pBB->agent.Position).MagnitudeSquared();
			if (d < bestDist)
			{
				bestDist = d;
				closest = &e;
			}

			Elite::Vector2 away = m_pSteeringBehaviour->Evade(
				m_pBB->agent, e.Location, e.LinearVelocity);
			float weight = 1.f / (d + 0.001f);
			fleeDir += away * weight;
		}

		// combine influences of all enemies. When they cancel out the
		// resulting fleeDir becomes {0,0} which causes the agent to
		// stand still. In that case fall back to moving directly away
		// from the closest enemy.
		if (fleeDir.MagnitudeSquared() < 0.001f && closest)
			fleeDir = m_pBB->agent.Position - closest->Location;

		if (fleeDir.MagnitudeSquared() > 0.f)
			fleeDir.Normalize();
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
		fleeDir = m_pSteeringBehaviour->Evade(
			m_pBB->agent, closest->Location, closest->LinearVelocity);
	}
	else if (m_pBB->lastEnemyValid)
	{
		closest = &m_pBB->lastEnemy;
		fleeDir = m_pSteeringBehaviour->Evade(
			m_pBB->agent, closest->Location, closest->LinearVelocity);
	}

	if (closest)
	{
		constexpr float fleeTargetDistance = 25.f;
		Elite::Vector2 fleeTarget = m_pBB->agent.Position + fleeDir * fleeTargetDistance;

		Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(fleeTarget);
		Elite::Vector2 toNav = navPt - m_pBB->agent.Position;
		Elite::Vector2 desired{};
		if (toNav.MagnitudeSquared() < 0.001f)
			desired = fleeDir;
		else
			desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);

		steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
		EnableSprint(steering);

	}
	return steering;



}
#pragma endregion

#pragma region PICKUP_LOOT
SteeringPlugin_Output FiniteStateMachine::PickupLoot(float dt)
{
	SteeringPlugin_Output steering{};

	ItemInfo target{};
	bool hasTarget = false;

	if (!m_HouseItems.empty())
	{
		while (!m_HouseItems.empty() && IsPointInPurgeZone(m_HouseItems.front().Location))
			m_HouseItems.erase(m_HouseItems.begin());
		if (!m_HouseItems.empty())
		{
			target = m_HouseItems.front();
			hasTarget = true;
		}
	}
	if (!hasTarget)
	{
		const ItemInfo* closest = nullptr;
		float bestDist = FLT_MAX;
		for (auto const& i : m_pBB->items)
		{
			if (i.Type == eItemType::GARBAGE)
				continue;
			if (IsPointInPurgeZone(i.Location))
				continue;
			float d = (i.Location - m_pBB->agent.Position).MagnitudeSquared();
			if (d < bestDist)
			{
				bestDist = d;
				closest = &i;
			}
		}
		if (closest)
		{
			target = *closest;
			hasTarget = true;
		}
	}

	if (!hasTarget)
		return steering;

	if (IsPointInPurgeZone(target.Location))
		return steering;

	int slot = m_pBB->freeSlot;
	if (slot < 0)
		slot = DetermineReplacementSlot(target);
	if (slot < 0)
		return steering;

	float distSqr = (target.Location - m_pBB->agent.Position).MagnitudeSquared();
	if (distSqr < m_pBB->agent.GrabRange * m_pBB->agent.GrabRange)
	{
		ItemInfo item = target;
		if (m_pInterface->GrabItem(item))
		{
			if (m_pBB->inventory[slot].Type != eItemType::GARBAGE)
				m_pInterface->Inventory_RemoveItem(static_cast<UINT>(slot));

			if (m_pInterface->Inventory_AddItem(static_cast<UINT>(slot), item))
			{
				if (slot >= 0 && slot < static_cast<int>(m_pBB->inventory.size()))
				{
					m_pBB->inventory[slot] = item;
					m_pBB->freeSlot = -1;

					if (!m_HouseItems.empty() && (m_HouseItems.front().Location - target.Location).MagnitudeSquared() < 0.01f)
						m_HouseItems.erase(m_HouseItems.begin());
				}
			}
		}
	}
	else
	{
		Elite::Vector2 navPt = m_pInterface->NavMesh_GetClosestPathPoint(target.Location);
		Elite::Vector2 desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
		steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
		steering.AutoOrient = true;
	}
	return steering;
}


int FiniteStateMachine::DetermineReplacementSlot(const ItemInfo& newItem) const
{
	const auto& inv = m_pBB->inventory;
	int slotSameType = -1;
	int lowestSameValue = INT_MAX; 
	int slotFood = -1, slotMed = -1, slotGun = -1;
	int lowestFood = INT_MAX;
	int lowestMed = INT_MAX;
	int lowestGun = INT_MAX;
	int countFood = 0, countMed = 0, countGun = 0;

	for (int i = 0; i < static_cast<int>(inv.size()); ++i)
	{
		const ItemInfo& it = inv[i];
		if (it.Type == newItem.Type)
		{
			if (it.Value < lowestSameValue)
			{
				lowestSameValue = it.Value;
				slotSameType = i;
			}
		}

		switch (it.Type)
		{
		case eItemType::FOOD:
			++countFood;
			if (it.Value < lowestFood)
				lowestFood = it.Value, slotFood = i;
			break;
		case eItemType::MEDKIT:
			++countMed;
			if (it.Value < lowestMed)
				lowestMed = it.Value, slotMed = i;
			break;
		case eItemType::PISTOL:
		case eItemType::SHOTGUN:
			++countGun;
			if (it.Value < lowestGun)
				lowestGun = it.Value, slotGun = i;
			break;
		default:
			break;
		}
	}

	switch (newItem.Type)
	{
	case eItemType::FOOD:
		if (slotSameType != -1 && newItem.Value > lowestSameValue)
			return slotSameType;
		if (countFood == 0)
		{
			if (countGun >= countMed)
			{
				if (slotGun != -1)
					return slotGun;
			}
			else if (slotMed != -1)
			{
				return slotMed;
			}
		}
		else if (slotFood != -1 && newItem.Value > lowestFood)
			return slotFood;
		break;
	case eItemType::MEDKIT:
		if (slotSameType != -1 && newItem.Value > lowestSameValue)
			return slotSameType;
		if (countMed == 0)
		{
			if (countGun >= countFood)
			{
				if (slotGun != -1)
					return slotGun;
			}
			else if (slotFood != -1)
			{
				return slotFood;
			}
		}
		else if (slotMed != -1 && newItem.Value > lowestMed)
			return slotMed;
		break;
	case eItemType::PISTOL:
	case eItemType::SHOTGUN:
		if (slotSameType != -1 && newItem.Value > lowestSameValue)
			return slotSameType;
		break;
	default:
		break;
	}
	return -1;
}
#pragma endregion

#pragma region USE_ITEM
SteeringPlugin_Output FiniteStateMachine::UseItem(float /*dt*/)
{
	SteeringPlugin_Output steering{};

	const float maxStat{ 10.f };
	const int invCap = static_cast<int>(m_pBB->inventory.size());
	for (int i = 0; i < invCap; ++i)
	{
		ItemInfo& item = m_pBB->inventory[i];
		if (item.Type == eItemType::MEDKIT)
		{
			float threshold = maxStat - static_cast<float>(item.Value);
			if (m_pBB->agent.Health <= threshold)
			{
				m_pInterface->Inventory_UseItem(static_cast<UINT>(i));
				m_pInterface->Inventory_RemoveItem(static_cast<UINT>(i));
				item = { eItemType::GARBAGE };
				m_pBB->freeSlot = i;
				break;
			}
		}
		else if (item.Type == eItemType::FOOD)
		{
			float threshold = maxStat - static_cast<float>(item.Value);
			if (m_pBB->agent.Energy <= threshold)
			{
				m_pInterface->Inventory_UseItem(static_cast<UINT>(i));
				m_pInterface->Inventory_RemoveItem(static_cast<UINT>(i));
				item = { eItemType::GARBAGE };
				m_pBB->freeSlot = i;
				break;
			}
		}
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
	}
	case AgentState::GoToHouse:
	{
		return UpdateGoToHouse(dt);
	}
	case AgentState::ExploreHouse:
	{
		return UpdateExploreHouse(dt);
	}
	case AgentState::Attack:
	{
		return UpdateAttack(dt);
	}
	case AgentState::EvadeEnemy:
	{
		return UpdateEvadeEnemy(dt);
	}
	case AgentState::PickupLoot:
	{
		return PickupLoot(dt);
	}
	case AgentState::EvadePurgeZone:
	{
		return UpdateEvadePurgeZone(dt);
	}
	case AgentState::UseItem:
	{
		return UseItem(dt);
		break;
	}
	}
	return SteeringPlugin_Output();


}

void FiniteStateMachine::UpdatePurgeZoneMemory(float dt)
{
	const float memoryTime = 10.f;
	auto zones = m_pInterface->GetPurgeZonesInFOV();
	for (auto const& z : zones)
	{
		auto it = std::find_if(m_pBB->purgeZones.begin(), m_pBB->purgeZones.end(),
			[&](const TrackedPurgeZone& p) { return p.info.ZoneHash == z.ZoneHash; });
		if (it != m_pBB->purgeZones.end())
		{
			it->info = z;
			it->info.Radius += 8.f; //add some padding
			it->remainingTime = memoryTime;
		}
		else
		{
			m_pBB->purgeZones.push_back({ z, memoryTime });
		}
	}
	for (auto it = m_pBB->purgeZones.begin(); it != m_pBB->purgeZones.end(); )
	{
		it->remainingTime -= dt;
		if (it->remainingTime <= 0.f)
			it = m_pBB->purgeZones.erase(it);
		else
			++it;
	}

	m_pBB->inPurgeZone = IsPointInPurgeZone(m_pBB->agent.Position);
}

void FiniteStateMachine::UpdateInventoryInfo()
{
	m_pBB->hasWeapon = false;
	m_pBB->weaponSlot = -1;
	m_pBB->weaponAmmo = 0;
	m_pBB->freeSlot = -1;
	m_pBB->needsMedkit = false;
	m_pBB->needsFood = false;

	const int invCap = static_cast<int>(m_pInterface->Inventory_GetCapacity());
	if (m_pBB->inventory.size() != static_cast<size_t>(invCap))
		m_pBB->inventory.assign(invCap, ItemInfo{ eItemType::GARBAGE });

	for (int i = 0; i < invCap; ++i)
	{
		if (m_pBB->inventory[i].Type != eItemType::GARBAGE)
		{
			ItemInfo invItem{};
			if (m_pInterface->Inventory_GetItem(static_cast<UINT>(i), invItem))
			{
				m_pBB->inventory[i] = invItem;

				if (invItem.Value == 0)
				{
					m_pInterface->Inventory_RemoveItem(static_cast<UINT>(i));
					m_pBB->inventory[i] = { eItemType::GARBAGE };
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
				else if (invItem.Type == eItemType::MEDKIT)
				{
					float threshold = 10.f - static_cast<float>(invItem.Value);
					if (m_pBB->agent.Health <= threshold)
						m_pBB->needsMedkit = true;
				}
				else if (invItem.Type == eItemType::FOOD)
				{
					float threshold = 10.f - static_cast<float>(invItem.Value);
					if (m_pBB->agent.Energy <= threshold)
						m_pBB->needsFood = true;
				}
			}
			else
			{
				m_pBB->inventory[i] = { eItemType::GARBAGE };
			}
		}

		if (m_pBB->inventory[i].Type == eItemType::GARBAGE && m_pBB->freeSlot == -1)
			m_pBB->freeSlot = i;
	}

	bool hasNonGarbage = std::any_of(m_pBB->items.begin(), m_pBB->items.end(),
		[this](const ItemInfo& item) {
			return item.Type != eItemType::GARBAGE && !IsPointInPurgeZone(item.Location);
		});
	if (!hasNonGarbage && m_HouseExplorationComplete)
		hasNonGarbage = std::any_of(m_HouseItems.begin(), m_HouseItems.end(),
			[this](const ItemInfo& item) {
				return item.Type != eItemType::GARBAGE && !IsPointInPurgeZone(item.Location);
			});
	m_pBB->hasNonGarbage = hasNonGarbage;

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
		{
			m_pBB->knownHouseCenters.push_back(h.Center);

			Elite::Vector2 half = h.Size * 0.5f;
			float margin = m_pGrid->GetCellSize() + 15.f; 
			float offsetX = half.x + margin;
			float offsetY = half.y + margin;

		
		}
	}
	if (m_pBB->agent.IsInHouse)
	{
		float bestDistSqr = FLT_MAX;
		Elite::Vector2 currentCenter{};
		for (auto const& h : m_pBB->houses)
		{
			float d = (h.Center - m_pBB->agent.Position).MagnitudeSquared();
			if (d < bestDistSqr)
			{
				bestDistSqr = d;
				currentCenter = h.Center;
			}
		}

		if (bestDistSqr < FLT_MAX)
		{
			bool visited = false;
			for (auto const& v : m_pBB->visitedHouseCenters)
			{
				if ((v - currentCenter).MagnitudeSquared() < 0.1f * 0.1f)
				{
					visited = true;
					break;
				}
			}

			m_pBB->inUnvisitedHouse = !visited;
			if (!visited)
			{
				m_pBB->currentHouseTarget = currentCenter;
				m_pBB->hasHouseTarget = true;
			}
			else
			{
				m_pBB->hasHouseTarget = false;
			}
		}
	}
	else
	{
		m_pBB->inUnvisitedHouse = false;
		m_IsExploringHouse = false;
		m_HouseExploreTargets.clear();
		m_HouseItems.clear();
		m_CurrentHouseExploreIndex = 0;
		m_HouseExplorationComplete = false;

		float bestDistSqr = FLT_MAX;
		Elite::Vector2 bestCenter{};
		for (auto const& center : m_pBB->knownHouseCenters)
		{
			bool visited = false;
			for (auto const& v : m_pBB->visitedHouseCenters)
			{
				if ((v - center).MagnitudeSquared() < 0.1f * 0.1f)
				{
					visited = true;
					break;
				}
			}
			if (visited) continue;
			if (IsPointInPurgeZone(center)) continue;
			float d = (center - m_pBB->agent.Position).MagnitudeSquared();
			if (d < bestDistSqr)
			{
				bestDistSqr = d;
				bestCenter = center;
			}
		}

		if (bestDistSqr < FLT_MAX)
		{
			m_pBB->currentHouseTarget = bestCenter;
			m_pBB->hasHouseTarget = true;
		}
		else
		{
			m_pBB->hasHouseTarget = false;
		}
	}
}

void FiniteStateMachine::EnableSprint(SteeringPlugin_Output& steering)
{
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
bool FiniteStateMachine::IsPointInPurgeZone(const Elite::Vector2& pos) const
{
	for (auto const& z : m_pBB->purgeZones)
		if ((pos - z.info.Center).MagnitudeSquared() < z.info.Radius * z.info.Radius)
			return true;
	return false;
}
#pragma endregion
