#pragma once
#include "States.h"
#include "Blackboard.h"

class StateDecider
{

	bool SelectNextHouse(Blackboard* bb) const; 
public: 
	StateDecider() = default; 
	~StateDecider() = default; 

	AgentState Decide(AgentState current, Blackboard* bb); 

	


};
