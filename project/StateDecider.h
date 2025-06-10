#pragma once
#include "States.h"
#include "Blackboard.h"

class StateDecider
{

	static bool IsVisited(const Elite::Vector2& center, const std::vector<Elite::Vector2>& visited);
	bool SelectNextHouse(Blackboard* bb) const; 
public: 
	StateDecider() = default; 
	~StateDecider() = default; 

	AgentState Decide(AgentState current, Blackboard* bb); 

	


};
