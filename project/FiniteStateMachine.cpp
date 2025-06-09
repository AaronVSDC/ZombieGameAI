#include "stdafx.h"
#include "FiniteStateMachine.h"

FiniteStateMachine::FiniteStateMachine(IExamInterface* pInterface) : 
	m_pInterface{pInterface}, 
	m_pBB{std::make_unique<Blackboard>()},
	m_pStateDecider{std::make_unique<StateDecider>()},
	m_pGrid{ std::make_unique<Grid>() }, 
	m_pSteeringBehaviour{std::make_unique<SteeringBehaviour>()}, 
	m_CurrentState{AgentState::Explore}
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
	m_pBB->worldInfo = m_pInterface->World_GetInfo(); //Todo: test if can be moved to constructor

	if (!m_pGrid->IsInitialized()) m_pGrid->InitGrid(m_pBB.get()); 
	if (!m_HasEnteredFirstState) OnEnter(), m_HasEnteredFirstState = true;
	

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
	switch (m_CurrentState)
	{
	case AgentState::Explore:
	{
		m_pGrid->UpdateFOVGrid();
		m_Target = m_pGrid->GetNextFrontierTarget(); 
		break;
	}
	}
}

void FiniteStateMachine::OnExit()
{
}

#pragma region EXPLORE
SteeringPlugin_Output FiniteStateMachine::UpdateExplore(float dt)
{
	SteeringPlugin_Output steering{}; 

	m_pGrid->UpdateFOVGrid();

	const float arriveDist = m_pGrid->GetCellSize() * 0.5f;
	if ((m_pBB->agent.Position - m_Target).Magnitude() < arriveDist)
	{
		m_Target = m_pGrid->GetNextFrontierTarget();
	}

	////if no frontiers -> wander
	//if ((m_Target - m_pBB->agent.Position).MagnitudeSquared() < 1e-4f)
	//{
	//	steering.LinearVelocity = m_pSteeringBehaviour->Wander(m_pBB->agent);
	//	return steering;
	//}

	Elite::Vector2  navPt = m_pInterface->NavMesh_GetClosestPathPoint(m_Target);
	Elite::Vector2  desired = m_pSteeringBehaviour->Seek(m_pBB->agent, navPt);
	steering.AutoOrient = true;
	steering.LinearVelocity = desired * m_pBB->agent.MaxLinearSpeed;
	
	return steering;
}
#pragma endregion