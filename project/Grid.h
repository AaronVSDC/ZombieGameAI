#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include <vector>
#include <algorithm>
#include <EliteMath\EVector2.h>

class Grid
{

public: 
	Grid(IExamInterface* _interface);


	void UpdateFOVGrid(); 
	void DebugDraw() const;
	void Refresh();



private: 
	//---------------------------------
	// INTERFACE AND AGENT/WORLD INFO
	//---------------------------------
	IExamInterface* m_pInterface; 
	AgentInfo		m_AgentInfo; 
	WorldInfo		m_WorldInfo; 


	//---------------------------------
	// EXPLORATION GRID PARAMETERS
	//---------------------------------
	float              m_CellSize = 0.f;
	int                m_Rows = 0, m_Cols = 0;
	Elite::Vector2     m_Origin;
	std::vector<std::vector<int>> m_Grid; // 0=unknown,1=seen

	std::vector<Elite::Vector2> m_Frontiers;
	Elite::Vector2     m_CurrentTarget = { 0,0 };

	void InitGrid(); 
	void DetectFrontiers(); 






};
