#include "stdafx.h"
#include "FiniteStateMachine.h"




FiniteStateMachine::FiniteStateMachine(IExamInterface* pInterface) : 
	m_pInterface{pInterface}, 
	m_pGrid{ std::make_unique<Grid>(pInterface) }, 
	m_pSteeringBehaviour{std::make_unique<SteeringBehaviour>()}
{
}

void FiniteStateMachine::Update(float dt)
{
	m_BB.agent = m_pInterface->Agent_GetInfo(); 
	m_BB.enemies = m_pInterface->GetEnemiesInFOV(); 
	m_BB.items = m_pInterface->GetItemsInFOV(); 
	m_BB.houses = m_pInterface->GetHousesInFOV(); 

	AgentState next = m_StateDecider.Decide(m_CurrentState, m_BB); 
	if (next != m_CurrentState)
		OnExit(), m_CurrentState = next, OnEnter(); 

	switch (m_CurrentState)
	{
	case AgentState::Explore: UpdateExplore(dt); break; 
	}

}
void FiniteStateMachine::DebugRender() const
{
	m_pGrid->DebugDraw(); 
}
void FiniteStateMachine::OnEnter()
{
}

void FiniteStateMachine::OnExit()
{
}

#pragma region EXPLORE
void FiniteStateMachine::UpdateExplore(float dt)
{




}
#pragma endregion