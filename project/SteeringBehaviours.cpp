#include "stdafx.h"
#include "SteeringBehaviours.h"

Elite::Vector2 SteeringBehaviour::Seek(const AgentInfo& agentInfo, const Elite::Vector2& targetPos)
{
    Elite::Vector2 toTarget = targetPos - agentInfo.Position;
    float distSquared = toTarget.MagnitudeSquared();
    toTarget.Normalize();
    return toTarget;
}

Elite::Vector2 SteeringBehaviour::Pursuit(const AgentInfo& agentInfo, const Elite::Vector2& targetPos, const Elite::Vector2& targetVelocity)
{
    Elite::Vector2 toTarget = targetPos - agentInfo.Position;
    float dist = toTarget.Magnitude();

    float timeAhead = dist / agentInfo.MaxLinearSpeed; 
    Elite::Vector2 futurePos = targetPos + targetVelocity * timeAhead;
    Elite::Vector2 desired = futurePos - agentInfo.Position;
    desired.Normalize();
    return desired;
}

Elite::Vector2 SteeringBehaviour::Evade(const AgentInfo& agentInfo, const Elite::Vector2& threatPos, const Elite::Vector2& threatVelocity)
{
    Elite::Vector2 toThreat = threatPos - agentInfo.Position;
    float dist = toThreat.Magnitude();
    float timeAhead = dist / agentInfo.MaxLinearSpeed;
    Elite::Vector2 futureThreat = threatPos + threatVelocity * timeAhead;
    Elite::Vector2 away = agentInfo.Position - futureThreat;
    away.Normalize();
    return away;
}




