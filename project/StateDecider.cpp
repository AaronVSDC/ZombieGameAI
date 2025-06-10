#include "stdafx.h"
#include "StateDecider.h"

AgentState StateDecider::Decide(AgentState current, Blackboard* bb)
{
    
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