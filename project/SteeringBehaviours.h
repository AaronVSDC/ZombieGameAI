#pragma once
#include <Exam_HelperStructs.h>


class SteeringBehaviour
{
	//WANDER DATA
	float m_WanderOffset = 5.f; 
	float m_WanderRadius = 3.f; 
	float m_WanderJitter = 1.f; 
	float m_Theta = Elite::randomFloat() * 2.f * static_cast<float>(E_PI); 
	Elite::Vector2 m_WanderTarget = { cosf(m_Theta) * m_WanderRadius, sinf(m_Theta) * m_WanderRadius };


public: 
	Elite::Vector2 Seek(const AgentInfo& agentInfo, const Elite::Vector2& targetPos);
	Elite::Vector2 Flee(const AgentInfo& agentInfo, const Elite::Vector2& threatPos);
	Elite::Vector2 Pursuit(const AgentInfo& agentInfo, const Elite::Vector2& targetPos, const Elite::Vector2& targetVelocity); 
	Elite::Vector2 Evade(const AgentInfo& agentInfo, const Elite::Vector2& threatPos, const Elite::Vector2& threatVelocity); 
	Elite::Vector2 Wander(const AgentInfo& agentInfo); 
};