#include "Map.h"
#include "Clan.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

int countLandNeighbors(const std::vector<int>& grid, int x, int y)
{
   int count = 0;
   for (int dy = -1; dy <= 1; ++dy)
   {
      for (int dx = -1; dx <= 1; ++dx)
      {
         if (dx == 0 && dy == 0) continue;
         int nx = x + dx;
         int ny = y + dy;
         if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
         {
            count += grid[ny * GRID_WIDTH + nx];
         }
      }
   }
   return count;
}

void generateMap(std::vector<SquareTile>& map, std::vector<Village>& villages, std::vector<Clan>& clans)
{
   std::srand(static_cast<unsigned>(std::time(nullptr)));
   std::vector<int> grid(GRID_WIDTH * GRID_HEIGHT, 0);

   // Seed ~50% land (~1702 cells)
   for (int i = 0; i < TOTAL_CELLS / 2; ++i)
   {
      int idx = std::rand() % TOTAL_CELLS;
      grid[idx] = 1;
   }

   // Smooth with CA (5 iterations)
   for (int iter = 0; iter < 5; ++iter)
   {
      std::vector<int> newGrid = grid;
      for (int y = 0; y < GRID_HEIGHT; ++y)
      {
         for (int x = 0; x < GRID_WIDTH; ++x)
         {
            int idx = y * GRID_WIDTH + x;
            int landNeighbors = countLandNeighbors(grid, x, y);
            if (landNeighbors >= 4) newGrid[idx] = 1;
            else if (landNeighbors <= 3) newGrid[idx] = 0;
         }
      }
      grid = newGrid;
   }

   // Assign terrain types
   map.resize(GRID_WIDTH * GRID_HEIGHT);
   for (int y = 0; y < GRID_HEIGHT; ++y)
   {
      for (int x = 0; x < GRID_WIDTH; ++x)
      {
         int idx = y * GRID_WIDTH + x;
         SquareTile& tile = map[idx];
         tile.x = x;
         tile.y = y;
         tile.pos = { x * (float)TILE_SIZE, y * (float)TILE_SIZE };
         tile.hasVillage = false;
         tile.villageIdx = -1;

         if (grid[idx] == 0) // Water
         {
            tile.terrain = Terrain::WATER;
         }
         else // Land
         {
            int neighbors = countLandNeighbors(grid, x, y);
            float rand = static_cast<float>(std::rand()) / RAND_MAX;
            bool nearWater = false;
            for (int dy = -1; dy <= 1 && !nearWater; ++dy)
            {
               for (int dx = -1; dx <= 1 && !nearWater; ++dx)
               {
                  int nx = x + dx;
                  int ny = y + dy;
                  if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT &&
                     grid[ny * GRID_WIDTH + nx] == 0)
                  {
                     nearWater = true;
                  }
               }
            }

            if (neighbors >= 7)
            {
               if (rand < 0.2f) tile.terrain = Terrain::MOUNTAIN;
               else if (rand < 0.5f) tile.terrain = Terrain::HILLS;
               else if (rand < 0.75f) tile.terrain = Terrain::FOREST;
               else tile.terrain = Terrain::GRASSLAND;
            }
            else if (neighbors >= 5)
            {
               if (rand < 0.8f) tile.terrain = Terrain::GRASSLAND;
               else tile.terrain = Terrain::FOREST;
            }
            else
            {
               if (nearWater && rand < 0.7f) tile.terrain = Terrain::SWAMP;
               else if (rand < 0.6f) tile.terrain = Terrain::DESERT;
               else tile.terrain = Terrain::GRASSLAND;
            }
         }
      }
   }

   // Define clans with village tiles
   clans.clear();
   clans.push_back({ "Red Claw", {255, 128, 128, 255}, 0, 0, 0, {}, {0 * 16.0f, 44 * 16.0f, 16, 16} });
   clans.push_back({ "Glendwellers", {128, 255, 128, 255}, 0, 0, 0, {}, {1 * 16.0f, 6 * 16.0f, 16, 16} });
   clans.push_back({ "Gilded", {255, 255, 128, 255}, 0, 0, 0, {}, {0 * 16.0f, 6 * 16.0f, 16, 16} });
   clans.push_back({ "Xenth", {0, 243, 192, 255}, 0, 0, 0, {}, {1 * 16.0f, 44 * 16.0f, 16, 16} });

   // Place 3 villages per clan
   for (size_t c = 0; c < clans.size(); ++c)
   {
      bool centerPlaced = false;
      int centerX, centerY;
      while (!centerPlaced)
      {
         centerX = std::rand() % GRID_WIDTH;
         centerY = std::rand() % GRID_HEIGHT;
         int idx = centerY * GRID_WIDTH + centerX;

         if (grid[idx] == 1 && !map[idx].hasVillage)
         {
            bool tooClose = false;
            for (const auto& v : villages)
            {
               int dx = abs(centerX - v.x);
               int dy = abs(centerY - v.y);
               if (dx + dy < 15)
               {
                  tooClose = true;
                  break;
               }
            }
            if (!tooClose)
            {
               map[idx].hasVillage = true;
               map[idx].villageIdx = villages.size();
               map[idx].terrain = Terrain::GRASSLAND;
               Village centerVillage;
               centerVillage.x = centerX;
               centerVillage.y = centerY;
               centerVillage.name = clans[c].name + " Village " + std::to_string(villages.size() + 1);
               centerVillage.clanIdx = c;
               villages.push_back(centerVillage);
               clans[c].villages.push_back(villages.size() - 1);
               centerPlaced = true;
            }
         }
      }

      int placedCount = 1;
      while (placedCount < 3)
      {
         float angle = static_cast<float>(std::rand()) / RAND_MAX * 2 * PI;
         float radius = static_cast<float>(std::rand()) / RAND_MAX * 10;
         int dx = static_cast<int>(radius * cos(angle));
         int dy = static_cast<int>(radius * sin(angle));
         int x = centerX + dx;
         int y = centerY + dy;

         if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
         {
            int idx = y * GRID_WIDTH + x;
            if (grid[idx] == 1 && !map[idx].hasVillage)
            {
               bool tooClose = false;
               for (const auto& v : villages)
               {
                  int vdx = abs(x - v.x);
                  int vdy = abs(y - v.y);
                  if (vdx + vdy < 3)
                  {
                     tooClose = true;
                     break;
                  }
               }
               if (!tooClose)
               {
                  map[idx].hasVillage = true;
                  map[idx].villageIdx = villages.size();
                  map[idx].terrain = Terrain::GRASSLAND;
                  Village newVillage;
                  newVillage.x = x;
                  newVillage.y = y;
                  newVillage.name = clans[c].name + " Village " + std::to_string(villages.size() + 1);
                  newVillage.clanIdx = c;
                  villages.push_back(newVillage);
                  clans[c].villages.push_back(villages.size() - 1);
                  placedCount++;
               }
            }
         }
      }
   }
}