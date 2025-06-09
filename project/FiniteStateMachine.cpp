#include "stdafx.h"
#include "FiniteStateMachine.h"

FiniteStateMachine::FiniteStateMachine(IExamInterface* pInterface) : 
	m_pInterface{pInterface}, 
	m_pBB{std::make_unique<Blackboard>()},
	m_pStateDecider{std::make_unique<StateDecider>()},
	m_pGrid{ std::make_unique<Grid>() }, 
	m_pSteeringBehaviour{std::make_unique<SteeringBehaviour>()}
{
}

SteeringPlugin_Output FiniteStateMachine::Update(float dt)
{
	//--------------------
	//POPULATE BLACKBOARD
	//--------------------
	m_pBB->agent = m_pInterface->Agent_GetInfo(); 
	m_pBB->enemies = m_pInterface->GetEnemiesInFOV(); 
	m_pBB->items = m_pInterface->GetItemsInFOV(); 
	m_pBB->houses = m_pInterface->GetHousesInFOV(); 
	m_pBB->worldInfo = m_pInterface->World_GetInfo();

	if (!m_pGrid->IsInitialized()) m_pGrid->InitGrid(m_pBB.get()); 

	AgentState next = m_pStateDecider->Decide(m_CurrentState, m_pBB.get()); 
	if (next != m_CurrentState)
		OnExit(), m_CurrentState = next, OnEnter(); 

	switch (m_CurrentState)
	{
	case AgentState::Explore: return UpdateExplore(dt); break; 
	}

}
void FiniteStateMachine::DebugRender() const
{
	m_pGrid->DebugDraw(m_pInterface); 
}
void FiniteStateMachine::OnEnter()
{
}

void FiniteStateMachine::OnExit()
{
}

#pragma region EXPLORE
SteeringPlugin_Output FiniteStateMachine::UpdateExplore(float dt)
{


	return SteeringPlugin_Output(); 

}
#pragma endregion