#include "stdafx.h"
#include "StateDecider.h"

AgentState StateDecider::Decide(AgentState current, Blackboard* bb, float dt)
{
    // Keep track of time since last direct enemy interaction
    m_TotalTime += dt;

    //ENEMY HANDLING HAS PRIORITY
    if (!bb->enemies.empty() || bb->tookDamage)
    {
        // reset timer whenever a threat is detected
        m_TotalTime = 0.f;

        if (bb->hasWeapon && bb->weaponAmmo > 0)
        {
            bb->attackLatched = true; 
            return AgentState::Attack;
        }
        return AgentState::EvadeEnemy;
    } 

    // Keep attacking as long as we still have a valid target and ammo
    if (current == AgentState::Attack && bb->hasWeapon && bb->weaponAmmo > 0 && (bb->attackLatched || bb->lastEnemyValid))
        return AgentState::Attack;

    // Continue evading for a short period after losing sight of the enemy
    if (current == AgentState::EvadeEnemy && m_TotalTime < m_EvadeDuration)
        return AgentState::EvadeEnemy;

    //ITEM HANDLING WHEN THERE IS FREE SPACE
    bool hasNonGarbage = std::any_of(bb->items.begin(), bb->items.end(),
        [](const ItemInfo& item) { return item.Type != eItemType::GARBAGE; });
    if (hasNonGarbage && bb->freeSlot >= 0)
        return AgentState::PickupLoot;

    //CONTINUE GOING TO HOUSE TARGET IF IS SET
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