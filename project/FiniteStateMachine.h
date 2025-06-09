#pragma once
#include "StateDecider.h"
#include "IExamInterface.h"
#include "Grid.h"
#include "SteeringBehaviours.h"

class FiniteStateMachine
{

	AgentState m_CurrentState = AgentState::Explore; 
	IExamInterface* m_pInterface; 
	std::unique_ptr<StateDecider> m_pStateDecider;
	std::unique_ptr<Blackboard> m_pBB; 
	std::unique_ptr<Grid> m_pGrid;
	std::unique_ptr<SteeringBehaviour> m_pSteeringBehaviour;


	void OnEnter(); 
	void OnExit(); 
	SteeringPlugin_Output UpdateExplore(float dt);



public: 
	FiniteStateMachine(IExamInterface* pInterface);
	~FiniteStateMachine() = default; 

	SteeringPlugin_Output Update(float dt);

	void DebugRender()const;





};