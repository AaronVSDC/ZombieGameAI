#pragma once
#include "StateDecider.h"
#include "IExamInterface.h"
#include "Grid.h"
#include "SteeringBehaviours.h"

class FiniteStateMachine
{

	AgentState m_CurrentState; 
	IExamInterface* m_pInterface; 
	std::unique_ptr<StateDecider> m_pStateDecider;
	std::unique_ptr<Blackboard> m_pBB; 
	std::unique_ptr<Grid> m_pGrid;
	std::unique_ptr<SteeringBehaviour> m_pSteeringBehaviour;
	

	Elite::Vector2 m_Target;
	bool m_HasEnteredFirstState = false; 


	void OnEnter(); 
	void OnExit(); 


	//----------------
	//STATE UPDATES
	//----------------
	SteeringPlugin_Output UpdateExplore(float dt);
	SteeringPlugin_Output UpdateGoToHouse(float dt); 
	SteeringPlugin_Output UpdateAttack(float dt); 
	SteeringPlugin_Output UpdateEvadeEnemy(float dt); 
	SteeringPlugin_Output PickupLoot(float dt); 


public: 
	FiniteStateMachine(IExamInterface* pInterface);
	~FiniteStateMachine() = default; 

	SteeringPlugin_Output Update(float dt);

	void DebugRender()const;





};