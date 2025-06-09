#pragma once
#include "States.h"
#include "Blackboard.h"

class StateDecider
{


public: 
	StateDecider() = default; 
	~StateDecider() = default; 

	AgentState Decide(AgentState current, const Blackboard* bb); 




};
