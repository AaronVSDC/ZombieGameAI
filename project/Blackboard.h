#pragma once
#include <Exam_HelperStructs.h>
#include <vector>


struct Blackboard
{
	AgentInfo agent; 
    std::vector<EnemyInfo> enemies;
    std::vector<ItemInfo> items;
    std::vector<HouseInfo>   houses;
};