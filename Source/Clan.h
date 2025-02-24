#ifndef CLAN_H
#define CLAN_H

#include "Game.h"
#include <vector>
#include <string>

// Clan struct
struct Clan
{
   std::string name;
   Color color;
   int gold = 0;
   int knowledge = 0;
   int worship = 0;
   std::vector<int> villages;
   Rectangle villageTile;
};

// Building struct (for villages)
struct Building
{
   std::string name;
   int productionCost;
   int upkeepCost;
   int foodBonus = 0;
   int productionBonus = 0;
   int goldBonus = 0;
   int knowledgeBonus = 0;
   int worshipBonus = 0;
};

// Village struct
struct Village
{
   int x, y;
   std::string name;
   int clanIdx;
   int population = 1;
   int foodStorehouse = 0;
   int productionStorehouse = 0;
   int foodProduction = 2;
   int productionOutput = 1;
   int goldOutput = 1;
   int knowledgeOutput = 0;
   int worshipOutput = 0;
   std::vector<Building> buildings;
};

// Unit struct
struct Unit
{
   std::string name;
   int clanIdx;
   int x, y;
   int attackStrength = 1;
   int defenseStrength = 1;
   int movementPoints = 2;
   std::vector<SpecialAbility> specialAbilities;
};

#endif