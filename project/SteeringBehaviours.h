#pragma once
#include <Exam_HelperStructs.h>


class SteeringBehaviour final
{
public: 
	Elite::Vector2 Seek(const AgentInfo& agentInfo, const Elite::Vector2& targetPos);
	Elite::Vector2 Pursuit(const AgentInfo& agentInfo, const Elite::Vector2& targetPos, const Elite::Vector2& targetVelocity); 
	Elite::Vector2 Evade(const AgentInfo& agentInfo, const Elite::Vector2& threatPos, const Elite::Vector2& threatVelocity); 
};