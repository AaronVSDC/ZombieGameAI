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

	float m_FrontierWanderTimer = 0.f;
	const float m_FrontierWanderDuration = 9.f;

	float m_SearchRotationAccumulation = 0.f;

	bool m_WantsToRun = false;

	//---------------------------------
	//EXPLORE_HOUSE STATE
	//---------------------------------
	std::vector<Elite::Vector2> m_HouseExploreTargets;
	size_t m_CurrentHouseExploreIndex = 0;
	std::vector<ItemInfo> m_HouseItems;
	Elite::Vector2 m_HouseExitTarget{};
	bool m_HouseExplorationComplete = false;
	bool m_IsExploringHouse = false;


	//---------------------------------
	// EXPLORE STATE
	//---------------------------------
	Elite::Vector2 m_ExploreOrigin{};
	std::vector<Elite::Vector2> m_RadialTargets{};
	size_t m_RadialTargetIndex = 0;
	float m_InitialRadius = 10.f;
	float m_CurrentRadius = m_InitialRadius;
	float m_RadiusIncrement = 75.f;
	float m_MinRadius = 1.f; 
	size_t m_NumRadialTargets = 16;
	std::deque<Elite::Vector2> m_ExtraExploreTargets{};
	void GenerateRadialTargets();
	void SetNextExploreTarget();

	void OnEnter(); 
	void OnExit(); 


	//----------------
	//STATE UPDATES
	//----------------
	SteeringPlugin_Output UpdateExplore(float dt);
	SteeringPlugin_Output UpdateGoToHouse(float dt);
	SteeringPlugin_Output UpdateAttack(float dt);
	SteeringPlugin_Output UpdateEvadeEnemy(float dt);
	SteeringPlugin_Output UpdateEvadePurgeZone(float dt);
	SteeringPlugin_Output PickupLoot(float dt);
	SteeringPlugin_Output UseItem(float dt);
	SteeringPlugin_Output UpdateExploreHouse(float dt);



public: 
	FiniteStateMachine(IExamInterface* pInterface);
	~FiniteStateMachine() = default;

	SteeringPlugin_Output Update(float dt);

	void DebugRender()const;
	void PopulateBlackboard();
	void UpdatePurgeZoneMemory(float dt);
	void InitAndUpdateGrid();
	SteeringPlugin_Output UpdateStates(float dt);

	void UpdateInventoryInfo();

	void UpdateHouseMemory();

	void EnableSprint(SteeringPlugin_Output& steering);
	bool IsPointInPurgeZone(const Elite::Vector2& pos) const;




};