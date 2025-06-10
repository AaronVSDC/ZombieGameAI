#pragma once
#include <Exam_HelperStructs.h>
#include <vector>


struct Blackboard
{
	AgentInfo agent; 
    WorldInfo worldInfo;
    std::vector<EnemyInfo> enemies;
    std::vector<ItemInfo> items;
    std::vector<HouseInfo>   houses;
    std::vector<Elite::Vector2> knownHouseCenters;   
    std::vector<Elite::Vector2> visitedHouseCenters; 
    bool                 hasHouseTarget = false;
    Elite::Vector2       currentHouseTarget{};
};