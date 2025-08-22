#pragma once
#include <Exam_HelperStructs.h>
#include <vector>

struct Blackboard
{
    AgentInfo agent;
    std::vector<EnemyInfo> enemies;

    bool  hasHouseTarget = false;

    bool  hasWeapon = false;
    int   weaponAmmo = 0;
    int   freeSlot = -1; 

    bool  attackLatched = false;
    bool  hasNonGarbage = false;
    bool  inPurgeZone = false;
    bool  needsMedkit = false;
    bool  needsFood = false;
    bool  canReplaceItem = false;
};