#include "stdafx.h"
#include "StateDecider.h"

AgentState StateDecider::Decide(AgentState current, Blackboard* bb, float dt)
{
    m_TotalTime += dt;

    if (bb->inPurgeZone)
        return AgentState::EvadePurgeZone;

    if (!bb->enemies.empty() || bb->agent.Bitten)
    {
        m_TotalTime = 0.f;
        if (bb->hasWeapon && bb->weaponAmmo > 0)
        {
            bb->attackLatched = true;
            return AgentState::Attack;
        }
        return AgentState::EvadeEnemy;
    }

    if (current == AgentState::Attack && bb->hasWeapon && bb->weaponAmmo > 0 && bb->attackLatched)
        return AgentState::Attack;

    if (current == AgentState::EvadeEnemy && m_TotalTime < m_EvadeDuration)
        return AgentState::EvadeEnemy;

    if (bb->needsMedkit || bb->needsFood)
        return AgentState::UseItem;

    if (bb->hasNonGarbage && bb->freeSlot >= 0)
        return AgentState::PickupLoot;

    if (current == AgentState::ExploreHouse && bb->agent.IsInHouse)
        return AgentState::ExploreHouse;
    if (bb->agent.IsInHouse && bb->inUnvisitedHouse)
        return AgentState::ExploreHouse;

    if (current == AgentState::GoToHouse && bb->hasHouseTarget && !bb->agent.IsInHouse)
        return AgentState::GoToHouse;

    if (bb->hasHouseTarget)
        return AgentState::GoToHouse;

    return AgentState::Explore; 



}
