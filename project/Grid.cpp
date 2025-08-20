#include "stdafx.h"
#include "Grid.h"

void Grid::UpdateFOVGrid()
{
	float halfAngle = m_BB->agent.FOV_Angle * 0.5f;
	Elite::Vector2 facing{ cosf(m_BB->agent.Orientation), sinf(m_BB->agent.Orientation) };
	for (int r = 0; r < m_Rows; ++r)
	{
		for (int c = 0; c < m_Cols; ++c)
		{
			Elite::Vector2 cellCenter = m_Origin + Elite::Vector2{ c + 0.5f, r + 0.5f } * m_CellSize;
			Elite::Vector2 toCell = cellCenter - m_BB->agent.Position;
			float dist = toCell.Magnitude();
			if (dist > m_BB->agent.FOV_Range) continue;
			float ang = acosf(facing.Dot(toCell.GetNormalized()));
			if (ang <= halfAngle)
				m_Grid[r][c] = 1;  // seen
		}
	}
    DetectFrontiers(); 
}


void Grid::InitGrid(const Blackboard* blackboard)
{
    m_BB = blackboard; 
	m_Origin = m_BB->worldInfo.Center - m_BB->worldInfo.Dimensions * 0.5f;
	m_CellSize = m_BB->agent.FOV_Range * 0.5;
	m_Cols = static_cast<int>(m_BB->worldInfo.Dimensions.x / m_CellSize) + 1; //+1 is to round up
	m_Rows = static_cast<int>(m_BB->worldInfo.Dimensions.y / m_CellSize) + 1;
	m_Grid.assign(m_Rows, std::vector<int>(m_Cols, 0)); 
	
    m_IsInitialized = true; 
}

void Grid::DetectFrontiers()
{
	m_Frontiers.clear();
	for (int r = 1; r < m_Rows - 1; ++r)
		for (int c = 1; c < m_Cols - 1; ++c)
		{
			if (m_Grid[r][c] != 0) continue;
			bool neighSeen = false;
			for (int dy = -1; dy <= 1 && !neighSeen; ++dy)
				for (int dx = -1; dx <= 1; ++dx)
					if (m_Grid[r + dy][c + dx] == 1)
						neighSeen = true;
			if (neighSeen)
			{
				Elite::Vector2 fc = m_Origin + Elite::Vector2{ c + 0.5f, r + 0.5f } * m_CellSize;
				m_Frontiers.push_back(fc);
			}
		}
}

Elite::Vector2 Grid::GetNextFrontierTarget() const
{
    if (m_Frontiers.empty())
        return m_BB->agent.Position;

    Elite::Vector2 facing{ cosf(m_BB->agent.Orientation), sinf(m_BB->agent.Orientation) };

    // Weight that controls how much the angle with the current direction
    // influences the selection. Higher values bias the selection to targets
    // that deviate from the current heading.
    constexpr float angleWeight = 2.f;

    auto it = std::min_element(m_Frontiers.begin(), m_Frontiers.end(),
        [&](const Elite::Vector2& a, const Elite::Vector2& b)
        {
            auto score = [&](const Elite::Vector2& target)
                {
                    float distance = Elite::Distance(target, m_BB->agent.Position);

                    Elite::Vector2 dir = (target - m_BB->agent.Position).GetNormalized();
                    float dot = facing.Dot(dir);
                    dot = max(-1.f, min(1.f, dot)); 
                    float angle = acosf(dot);

                    return distance + angleWeight * angle;
                };

            return score(a) < score(b);
        });

    return *it;
}

void Grid::DebugDraw(IExamInterface* m_pInterface) const
{
    // Depths
    const float zDepthFill = 0.9f; 
    const float zDepthLine = 0.89f; 
    const float zDepthFrontier = 0.88f; 

    // Colors
    const Elite::Vector3 seenCol{ 0.f, 1.f,   0.f };
    const Elite::Vector3 unseenCol{ 0.3f,0.3f, 0.3f };
    const Elite::Vector3 lineCol{ 0.1f,0.1f, 0.1f };
    const Elite::Vector3 frontierCol{ 0.4f,0.f,   0.8f }; 


    for (int r = 0; r < m_Rows; ++r)
    {
        int runStart = 0;
        int currentVal = m_Grid[r][0];

        auto flushRun = [&](int runEnd)
            {
                float x0 = m_Origin.x + runStart * m_CellSize;
                float x1 = m_Origin.x + runEnd * m_CellSize;
                float y0 = m_Origin.y + r * m_CellSize;
                float y1 = y0 + m_CellSize;

                Elite::Vector2 verts[4] = {
                    { x0, y0 }, { x1, y0 },
                    { x1, y1 }, { x0, y1 }
                };
                m_pInterface->Draw_SolidPolygon(
                    verts, 4,
                    (currentVal == 1 ? seenCol : unseenCol),
                    zDepthFill
                );
            };

        for (int c = 1; c < m_Cols; ++c)
        {
            if (m_Grid[r][c] != currentVal)
            {
                flushRun(c);
                runStart = c;
                currentVal = m_Grid[r][c];
            }
        }
        flushRun(m_Cols);
    }

    for (int r = 0; r <= m_Rows; ++r)
    {
        Elite::Vector2 p1{ m_Origin.x, m_Origin.y + r * m_CellSize };
        Elite::Vector2 p2{ m_Origin.x + m_Cols * m_CellSize, m_Origin.y + r * m_CellSize };
        m_pInterface->Draw_Segment(p1, p2, lineCol, zDepthLine);
    }

    for (int c = 0; c <= m_Cols; ++c)
    {
        Elite::Vector2 p1{ m_Origin.x + c * m_CellSize, m_Origin.y };
        Elite::Vector2 p2{ m_Origin.x + c * m_CellSize, m_Origin.y + m_Rows * m_CellSize };
        m_pInterface->Draw_Segment(p1, p2, lineCol, zDepthLine);
    }

    const float half = m_CellSize * 0.5f;
    for (const auto& fc : m_Frontiers)
    {
        Elite::Vector2 verts[4] = {
            { fc.x - half, fc.y - half },
            { fc.x + half, fc.y - half },
            { fc.x + half, fc.y + half },
            { fc.x - half, fc.y + half }
        };
        m_pInterface->Draw_SolidPolygon(verts, 4, frontierCol, zDepthFrontier);
    }
}
