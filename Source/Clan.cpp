#include "Clan.h"
#include "Map.h"

bool canBuild(const Village& village, const std::vector<SquareTile>& map, BuildingType type, int& tileX, int& tileY)
{
   // Check if there’s an available worker
   bool hasFreeWorker = false;
   int freeWorkerIdx = -1;
   for (size_t i = 0; i < village.workers.size(); ++i)
   {
      if (!village.workers[i])
      {
         hasFreeWorker = true;
         freeWorkerIdx = i;
         break;
      }
   }
   if (!hasFreeWorker || village.buildings.size() >= 8) return false; // Max 8 buildings (surrounding tiles)

   // Check production points
   int cost = 0;
   Terrain requiredTerrain = Terrain::GRASSLAND; // Default
   switch (type)
   {
   case BuildingType::FARM:
      cost = 5; // Example cost
      requiredTerrain = Terrain::GRASSLAND;
      break;
   case BuildingType::LOGGING_CAMP:
      cost = 5;
      requiredTerrain = Terrain::FOREST;
      break;
   case BuildingType::MINE:
      cost = 7;
      requiredTerrain = Terrain::HILLS;
      break;
   case BuildingType::WORSHIP_SITE:
      cost = 7;
      requiredTerrain = Terrain::HILLS;
      break;
   case BuildingType::LIBRARY:
      cost = 6;
      requiredTerrain = Terrain::GRASSLAND;
      break;
   }
   if (village.productionStorehouse < cost) return false;

   // Check adjacent tiles
   for (int dy = -1; dy <= 1; ++dy)
   {
      for (int dx = -1; dx <= 1; ++dx)
      {
         if (dx == 0 && dy == 0) continue; // Skip village tile itself
         int nx = village.x + dx;
         int ny = village.y + dy;
         if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
         {
            int idx = ny * GRID_WIDTH + nx;
            if (map[idx].terrain == requiredTerrain && !map[idx].hasVillage) // Check if tile is free
            {
               bool tileOccupied = false;
               for (const auto& b : village.buildings)
               {
                  if (b.tileX == nx && b.tileY == ny)
                  {
                     tileOccupied = true;
                     break;
                  }
               }
               if (!tileOccupied)
               {
                  tileX = nx;
                  tileY = ny;
                  return true;
               }
            }
         }
      }
   }
   return false; // No suitable tile found
}

void buildBuilding(Village& village, BuildingType type, int tileX, int tileY)
{
   Building building;
   building.tileX = tileX;
   building.tileY = tileY;

   switch (type)
   {
   case BuildingType::FARM:
      building.name = "Farm";
      building.type = BuildingType::FARM;
      building.productionCost = 5;
      building.upkeepCost = 0;
      building.productionBonus = 1;
      break;
   case BuildingType::LOGGING_CAMP:
      building.name = "Logging Camp";
      building.type = BuildingType::LOGGING_CAMP;
      building.productionCost = 5;
      building.upkeepCost = 0;
      building.productionBonus = 1;
      break;
   case BuildingType::MINE:
      building.name = "Mine";
      building.type = BuildingType::MINE;
      building.productionCost = 7;
      building.upkeepCost = 0;
      building.goldBonus = 1;
      break;
   case BuildingType::WORSHIP_SITE:
      building.name = "Worship Site";
      building.type = BuildingType::WORSHIP_SITE;
      building.productionCost = 7;
      building.upkeepCost = 0;
      building.worshipBonus = 1;
      break;
   case BuildingType::LIBRARY:
      building.name = "Library";
      building.type = BuildingType::LIBRARY;
      building.productionCost = 6;
      building.upkeepCost = 0;
      building.knowledgeBonus = 1;
      break;
   }

   // Assign worker
   for (size_t i = 0; i < village.workers.size(); ++i)
   {
      if (!village.workers[i])
      {
         village.workers[i] = true;
         building.workerIdx = i;
         break;
      }
   }

   // Deduct production cost
   village.productionStorehouse -= building.productionCost;

   // Add building
   village.buildings.push_back(building);
}