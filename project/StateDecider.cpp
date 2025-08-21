#include "stdafx.h"
#include "StateDecider.h"

AgentState StateDecider::Decide(AgentState current, Blackboard* bb, float dt)
{
    m_TotalTime += dt;

    SelectState(current, bb, dt);

    if (current == AgentState::Attack && bb->attackLatched)
        return AgentState::Attack;

    if (current == AgentState::PickupLoot)
        return AgentState::PickupLoot;

    if (current == AgentState::GoToHouse)
        return AgentState::GoToHouse;

    if (current == AgentState::EvadeEnemy && m_TotalTime < m_EvadeDuration)
        return AgentState::EvadeEnemy;

    return AgentState::Explore;


}
void StateDecider::SelectState(AgentState current, Blackboard* bb, float dt) 
{
    if (!bb->enemies.empty() || bb->agent.Bitten)
    {
        if (bb->hasWeapon && bb->weaponAmmo > 0)
        {
            bb->attackLatched = true;
            current = AgentState::Attack;
        }
        else
        {
            current = AgentState::EvadeEnemy;
        }
    }
    else if (bb->hasNonGarbage && bb->freeSlot >= 0)
        current = AgentState::PickupLoot;
     
    else if (bb->hasHouseTarget && !bb->agent.IsInHouse || SelectNextHouse(bb))
        current = AgentState::GoToHouse;
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