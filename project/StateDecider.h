#pragma once
#include "States.h"
#include "Blackboard.h"

class StateDecider
{

public: 
	StateDecider() = default; 
	~StateDecider() = default; 

	AgentState Decide(AgentState current, Blackboard* bb, float dt); 

	float m_EvadeDuration = 4.f;
	float m_TotalTime = 0.f; 
	


};
