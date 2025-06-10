#include "stdafx.h"
#include "StateDecider.h"

AgentState StateDecider::Decide(AgentState current, Blackboard* bb)
{

	if (bb->hasHouseTarget)
		return AgentState::GoToHouse;
    if (SelectNextHouse(bb))
        return AgentState::GoToHouse; 

	return AgentState::Explore;

}

bool StateDecider::IsVisited(const Elite::Vector2& center, const std::vector<Elite::Vector2>& visited)
{
    constexpr float eps = 0.1f;
    for (auto const& v : visited)
        if ((v - center).MagnitudeSquared() < eps * eps)
            return true;
    return false;
}

bool StateDecider::SelectNextHouse(Blackboard* bb) const
{
    // Find the closest house center that hasn't been visited yet
    float bestDistSqr = FLT_MAX;
    Elite::Vector2 bestCenter{};
    for (auto const& h : bb->houses)
    {
        // skip visited
        bool visited = false;
        for (auto const& v : bb->visitedHouseCenters)
        {
            if ((v - h.Center).MagnitudeSquared() < 0.1f * 0.1f)
            {
                visited = true;
                break;
            }
        }
        if (visited) continue;

        float d = (h.Center - bb->agent.Position).MagnitudeSquared();
        if (d < bestDistSqr)
        {
            bestDistSqr = d;
            bestCenter = h.Center;
        }
    }

    if (bestDistSqr < FLT_MAX)
    {
        // latch it on the blackboard
        bb->currentHouseTarget = bestCenter;
        bb->hasHouseTarget = true;
        return true;
    }
    return false;
}