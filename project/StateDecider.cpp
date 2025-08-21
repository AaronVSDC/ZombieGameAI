#include "stdafx.h"
#include "StateDecider.h"

AgentState StateDecider::Decide(AgentState current, Blackboard* bb, float dt)
{
    m_TotalTime += dt;

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

    for (auto const& item : bb->inventory)
    {
        if (item.Type == eItemType::MEDKIT && bb->agent.Health <= 10.f - static_cast<float>(item.Value))
            return AgentState::UseItem;
        if (item.Type == eItemType::FOOD && bb->agent.Energy <= 10.f - static_cast<float>(item.Value))
            return AgentState::UseItem;
    }

    if (bb->hasNonGarbage && bb->freeSlot >= 0)
        return AgentState::PickupLoot;

    if (bb->agent.IsInHouse)
        return AgentState::ExploreHouse;

    if (current == AgentState::GoToHouse && bb->hasHouseTarget && !bb->agent.IsInHouse)
        return AgentState::GoToHouse;

    if (SelectNextHouse(bb))
        return AgentState::GoToHouse;

    return AgentState::Explore;



}
bool StateDecider::SelectNextHouse(Blackboard* bb) const
{
    // Find the closest house center that hasn't been visited yet
    float bestDistSqr = FLT_MAX;
    Elite::Vector2 bestCenter{};
    for (auto const& center : bb->knownHouseCenters)
    {
        bool visited = false;
        for (auto const& v : bb->visitedHouseCenters)
        {
            if ((v - center).MagnitudeSquared() < 0.1f * 0.1f)
            {
                visited = true;
                break;
            }
        }
        if (visited) continue;

        float d = (center - bb->agent.Position).MagnitudeSquared();
        if (d < bestDistSqr)
        {
            bestDistSqr = d;
            bestCenter = center;
        }
    }

    if (bestDistSqr < FLT_MAX)
    {
        bb->currentHouseTarget = bestCenter;
        bb->hasHouseTarget = true;
        return true;
    }
    return false;
}