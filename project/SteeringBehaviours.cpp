#include "stdafx.h"
#include "SteeringBehaviours.h"

Elite::Vector2 SteeringBehaviour::Seek(const AgentInfo& agentInfo, const Elite::Vector2& targetPos)
{
    Elite::Vector2 toTarget = targetPos - agentInfo.Position;
    float distSquared = toTarget.MagnitudeSquared();
    toTarget.Normalize();
    return toTarget;
}

Elite::Vector2 SteeringBehaviour::Flee(const AgentInfo& agentInfo, const Elite::Vector2& threatPos)
{
    
    Elite::Vector2 away = agentInfo.Position - threatPos;
    away.Normalize();
    return away;
}

Elite::Vector2 SteeringBehaviour::Pursuit(const AgentInfo& agentInfo, const Elite::Vector2& targetPos, const Elite::Vector2& targetVelocity)
{
    Elite::Vector2 toTarget = targetPos - agentInfo.Position;
    float dist = toTarget.Magnitude();

    float timeAhead = dist / agentInfo.MaxLinearSpeed; // how long until we get there
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

Elite::Vector2 SteeringBehaviour::Wander(const AgentInfo& agentInfo, float wanderRadius, float jitterMultiplier)
{
    float jitter = m_WanderJitter * jitterMultiplier;
    Elite::Vector2 randomDisp = { Elite::randomFloat() * jitter - jitter * 0.5f,
                                 Elite::randomFloat() * jitter - jitter * 0.5f };
    m_WanderTarget += randomDisp;
    if (jitterMultiplier <= 0.f)
        m_WanderTarget = { 0.f, 0.f };
    else
    {
        m_WanderTarget.Normalize(); 
        m_WanderTarget *= wanderRadius;
    }

    Elite::Vector2 forward = { cosf(agentInfo.Orientation), sinf(agentInfo.Orientation) };
    Elite::Vector2 circleCenter = agentInfo.Position + forward * m_WanderOffset;
    Elite::Vector2 worldTarget = circleCenter + m_WanderTarget;

    Elite::Vector2 desired = worldTarget - agentInfo.Position;
    desired.Normalize();
    return desired;
}




