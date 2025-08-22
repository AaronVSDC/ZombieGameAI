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
	Elite::Vector2 GetNextFrontierTarget() const;
	 
	bool IsCellVisited(const Elite::Vector2& pos) const;

	//----------------
	//GETTERS/SETTERS
	//----------------
	bool IsInitialized() const { return m_IsInitialized; }
	float GetCellSize() const { return m_CellSize;  }




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
	mutable int m_Mode = 0; 






};
