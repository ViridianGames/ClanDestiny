#include "Clan.h"
#include "Map.h"
#include "Game.h"

bool canBuild(const Village& village, const std::vector<SquareTile>& map, BuildingType type, int& tileX, int& tileY)
{
   // Check if there's an available worker
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
   if (!hasFreeWorker || (int)village.buildings.size() >= village.population) return false; // Max buildings = current population

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

VillageProduction calculateVillageProduction(const Village& village, const Clan& owner)
{
    VillageProduction prod;
    prod.food         = BASE_FOOD;
    prod.production   = BASE_PRODUCTION;
    prod.gold         = BASE_GOLD;
    prod.knowledge    = BASE_KNOWLEDGE;
    prod.worship      = BASE_WORSHIP;

    bool isGlendwellers = (owner.name == "Glendwellers");
    bool isGilded       = (owner.name == "Gilded");

    for (const auto& building : village.buildings)
    {
        switch (building.type)
        {
        case BuildingType::FARM:
            prod.food += (isGlendwellers ? 2 : 1);
            break;
        case BuildingType::LOGGING_CAMP:
            prod.production += 1;
            break;
        case BuildingType::MINE:
            prod.gold += (isGilded ? 2 : 1);
            break;
        case BuildingType::WORSHIP_SITE:
            prod.worship += 1;
            break;
        case BuildingType::LIBRARY:
            prod.knowledge += 1;
            break;
        }
    }

    return prod;
}

void processEndOfTurn(std::vector<Clan>& clans, std::vector<Village>& villages)
{
    for (size_t vIdx = 0; vIdx < villages.size(); ++vIdx)
    {
        Village& village = villages[vIdx];
        if (village.clanIdx < 0 || village.clanIdx >= (int)clans.size()) continue;

        Clan& owner = clans[village.clanIdx];
        VillageProduction thisTurn = calculateVillageProduction(village, owner);

        // Accumulate into village stores
        village.foodStorehouse       += thisTurn.food;
        village.productionStorehouse += thisTurn.production;

        // Update the village's "per turn" fields for UI display
        village.foodProduction    = thisTurn.food;
        village.productionOutput  = thisTurn.production;
        village.goldOutput        = thisTurn.gold;
        village.knowledgeOutput   = thisTurn.knowledge;
        village.worshipOutput     = thisTurn.worship;

        // Accumulate global resources into the clan
        owner.gold       += thisTurn.gold;
        owner.knowledge  += thisTurn.knowledge;
        owner.worship    += thisTurn.worship;

        // Food growth / population increase
        int growthThreshold = FOOD_PER_POP_GROWTH * village.population;
        while (village.foodStorehouse >= growthThreshold && village.population < MAX_VILLAGE_POPULATION)
        {
            village.foodStorehouse -= growthThreshold;
            village.population++;
            // Note: workers vector stays size 12 for now (or we can resize if desired)
        }
    }
}