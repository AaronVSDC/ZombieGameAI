#pragma once
#include "StateDecider.h"
#include "IExamInterface.h"
#include "Grid.h"
#include "SteeringBehaviours.h"

class FiniteStateMachine
{

	AgentState m_CurrentState = AgentState::Explore; 
	StateDecider m_StateDecider; 
	IExamInterface* m_pInterface; 
	Blackboard m_BB; 
	std::unique_ptr<Grid> m_pGrid;
	std::unique_ptr<SteeringBehaviour> m_pSteeringBehaviour;


	void OnEnter(); 
	void OnExit(); 
	void UpdateExplore(float dt); 



public: 
	FiniteStateMachine(IExamInterface* pInterface);
	~FiniteStateMachine() = default; 

	void Update(float dt); 

	void DebugRender()const;





};