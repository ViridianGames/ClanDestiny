#ifndef CLAN_H
#define CLAN_H

#include "Game.h"
#include "Map.h" // Added for SquareTile
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

// Building types
enum class BuildingType
{
   FARM, LOGGING_CAMP, MINE, WORSHIP_SITE, LIBRARY
};

// Building struct
struct Building
{
   std::string name;
   BuildingType type;
   int productionCost;
   int upkeepCost;
   int foodBonus = 0;
   int productionBonus = 0;
   int goldBonus = 0;
   int knowledgeBonus = 0;
   int worshipBonus = 0;
   int workerIdx = -1; // Index of villager assigned (-1 if none)
   int tileX, tileY;   // Adjacent tile coords this building occupies
};

// Village struct
struct Village
{
   int x, y;
   std::string name;
   int clanIdx;
   int population = 4;          // Starts at 4, max 12
   int foodStorehouse = 0;
   int productionStorehouse = 0;
   int foodProduction = 2;      // Base 2 food/turn
   int productionOutput = 1;    // Base 1 production/turn
   int goldOutput = 1;          // Base 1 gold/turn
   int knowledgeOutput = 0;
   int worshipOutput = 0;
   std::vector<Building> buildings;
   std::vector<bool> workers;   // True if villager is assigned to a building
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

// Functions
bool canBuild(const Village& village, const std::vector<SquareTile>& map, BuildingType type, int& tileX, int& tileY);
void buildBuilding(Village& village, BuildingType type, int tileX, int tileY);

#endif