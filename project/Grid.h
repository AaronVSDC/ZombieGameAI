#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include <vector>
#include <algorithm>
#include <EliteMath\EVector2.h>
#include "Blackboard.h"

class Grid
{

public: 
	Grid() = default;
	void InitGrid(const Blackboard* blackboard);


	void UpdateFOVGrid(); 
	void DebugDraw(IExamInterface* _interface) const;
	void Refresh();

	//----------------
	//GETTERS/SETTERS
	//----------------
	bool IsInitialized() const { return m_IsInitialized; }




private: 
	//---------------------------------
	// BLACKBOARD 
	//---------------------------------
	const Blackboard*		m_BB; 

	//---------------------------------
	// EXPLORATION GRID PARAMETERS
	//---------------------------------
	float              m_CellSize = 0.f;
	int                m_Rows = 0, m_Cols = 0;
	Elite::Vector2     m_Origin;
	std::vector<std::vector<int>> m_Grid; // 0=unknown,1=seen

	std::vector<Elite::Vector2> m_Frontiers;
	Elite::Vector2     m_CurrentTarget = { 0,0 };

	//---------------------------------
	//HELPER MEMBERS/METHODS
	//---------------------------------
	bool m_IsInitialized = false; 

	void DetectFrontiers(); 






};
