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
    std::vector<eItemType> inventory;
    bool                 hasWeapon = false;
    int                  weaponSlot = -1;
    int                  freeSlot = -1;
    EnemyInfo             lastEnemy{};
    bool                  lastEnemyValid = false;
    float                 lastHealth = 0.f;
    bool                  tookDamage = false;
};