#include <raylib.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>

// Terrain types
enum class Terrain
{
   WATER, DESERT, GRASSLAND, FOREST, SWAMP, HILLS, MOUNTAIN
};

// Special Abilities for Units
enum class SpecialAbility
{
   BUILD_VILLAGE, // Can establish new villages
   FLY,          // Ignores terrain movement costs
   CAST_SPELL,   // Can cast magic spells
   BUFF_STACK    // Boosts other units in the same tile
};

// Clan struct
struct Clan
{
   std::string name;
   Color color;           // For visual distinction
   int gold = 0;          // Clan-wide gold storehouse
   int knowledge = 0;     // Clan-wide knowledge for tech tree (renamed from science)
   int worship = 0;       // Clan-wide worship for magic tree
   std::vector<int> villages; // Indices of villages owned by this clan
   Rectangle villageTile;     // Tile coords in tiles.png for villages
};

// Building struct (for villages)
struct Building
{
   std::string name;
   int productionCost; // Production required to build
   int upkeepCost;     // Production per turn to maintain
   int foodBonus = 0;     // Extra food production
   int productionBonus = 0; // Extra production
   int goldBonus = 0;     // Extra gold
   int knowledgeBonus = 0;  // Extra knowledge (renamed from science)
   int worshipBonus = 0;  // Extra worship
};

// Village struct
struct Village
{
   int x, y;                    // Map coordinates
   std::string name;
   int clanIdx;                 // Index of owning clan (-1 if none)
   int population = 1;          // Starts with 1 person
   int foodStorehouse = 0;      // Local food storage
   int productionStorehouse = 0;// Local production storage
   int foodProduction = 2;      // Base food output (e.g., 2 per turn)
   int productionOutput = 1;    // Base production output (e.g., 1 per turn)
   int goldOutput = 1;          // Gold sent to clan
   int knowledgeOutput = 0;     // Knowledge sent to clan (renamed from science)
   int worshipOutput = 0;       // Worship sent to clan
   std::vector<Building> buildings; // Buildings in this village
};

// Unit struct
struct Unit
{
   std::string name;
   int clanIdx;                 // Index of owning clan
   int x, y;                    // Current map position
   int attackStrength = 1;      // Attack power
   int defenseStrength = 1;     // Defense power
   int movementPoints = 2;      // Tiles per turn
   std::vector<SpecialAbility> specialAbilities; // List of special abilities
};

// Tile struct
struct SquareTile
{
   int x, y;
   Vector2 pos;
   Terrain terrain;
   bool hasVillage = false;
   int villageIdx = -1;     // Index in villages vector (-1 if no village)
};

// Constants
const int GRID_WIDTH = 74;
const int GRID_HEIGHT = 46;
const int BASE_WIDTH = 640;
const int BASE_HEIGHT = 360;
const int FINAL_WIDTH = 1280;
const int FINAL_HEIGHT = 720;
const float SCALE_FACTOR = static_cast<float>(FINAL_WIDTH) / BASE_WIDTH; // 2.0
const int TOTAL_CELLS = GRID_WIDTH * GRID_HEIGHT; // 3404
const int TILE_SIZE = 32; // Rendered size (16x16 scaled to 32x32)
const int VIEW_TILES_X = 15; // 15 tiles wide = 480px
const int VIEW_TILES_Y = 11; // 11 tiles tall = 352px
const int VIEW_OFFSET_X = 156; // Main view at 156x4
const int VIEW_OFFSET_Y = 4;
const int MINIMAP_OFFSET_X = 4; // Minimap at 4x4
const int MINIMAP_OFFSET_Y = 4;
const int MINIMAP_CELL_SIZE = 2; // 2x2px per cell
const float WATER_ANIM_SPEED = 0.25f; // 4 FPS (0.25s per frame)
const int CLAN_PANEL_X = MINIMAP_OFFSET_X; // Align with minimap
const int CLAN_PANEL_Y = MINIMAP_OFFSET_Y + GRID_HEIGHT * MINIMAP_CELL_SIZE + 4; // Below minimap with 4px gap

// Count land neighbors (8 surrounding cells)
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

// Generate map with cellular automata and villages
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
   clans.push_back({ "Red Claw", {255, 128, 128, 255}, 0, 0, 0, {}, {0 * 16.0f, 44 * 16.0f, 16, 16} });    // Red: 0x44
   clans.push_back({ "Glendwellers", {128, 255, 128, 255}, 0, 0, 0, {}, {1 * 16.0f, 6 * 16.0f, 16, 16} }); // Green: 1x6
   clans.push_back({ "Gilded", {255, 255, 128, 255}, 0, 0, 0, {}, {0 * 16.0f, 6 * 16.0f, 16, 16} });      // Yellow: 0x6
   clans.push_back({ "Xenth", {0, 243, 192, 255}, 0, 0, 0, {}, {1 * 16.0f, 44 * 16.0f, 16, 16} });       // Blue: 1x44

   // Place 3 villages per clan
   for (size_t c = 0; c < clans.size(); ++c)
   {
      // Place central village
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
               if (dx + dy < 15) // Min distance from other clan centers
               {
                  tooClose = true;
                  break;
               }
            }
            if (!tooClose)
            {
               map[idx].hasVillage = true;
               map[idx].villageIdx = villages.size();
               map[idx].terrain = Terrain::GRASSLAND; // Force grassland
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

      // Place 2 more villages within 10-square radius, min 3 apart
      int placedCount = 1;
      while (placedCount < 3)
      {
         float angle = static_cast<float>(std::rand()) / RAND_MAX * 2 * PI;
         float radius = static_cast<float>(std::rand()) / RAND_MAX * 10; // Up to 10 tiles
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
                  if (vdx + vdy < 3) // Min 3 squares apart
                  {
                     tooClose = true;
                     break;
                  }
               }
               if (!tooClose)
               {
                  map[idx].hasVillage = true;
                  map[idx].villageIdx = villages.size();
                  map[idx].terrain = Terrain::GRASSLAND; // Force grassland
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

// Draw the view window and minimap
void drawView(RenderTexture2D& target, const std::vector<SquareTile>& map, Texture2D& tileset,
   int& viewX, int& viewY, float& waterAnimTime, int& waterFrame,
   const std::vector<Clan>& clans, const std::vector<Village>& villages, Font& gameFont, Font& largeFont)
{
   BeginTextureMode(target);
   ClearBackground(BLACK);

   // Update water animation
   waterAnimTime += GetFrameTime();
   if (waterAnimTime >= WATER_ANIM_SPEED)
   {
      waterFrame = (waterFrame + 1) % 4; // Cycle 0-3
      waterAnimTime -= WATER_ANIM_SPEED;
   }

   // Draw minimap (74x46 -> 148x92px at 4x4)
   for (int y = 0; y < GRID_HEIGHT; ++y)
   {
      for (int x = 0; x < GRID_WIDTH; ++x)
      {
         int idx = y * GRID_WIDTH + x;
         Rectangle miniDest = { MINIMAP_OFFSET_X + x * MINIMAP_CELL_SIZE,
                              MINIMAP_OFFSET_Y + y * MINIMAP_CELL_SIZE,
                              MINIMAP_CELL_SIZE, MINIMAP_CELL_SIZE };
         if (map[idx].hasVillage)
            DrawRectangleRec(miniDest, clans[villages[map[idx].villageIdx].clanIdx].color); // Clan color
         else if (map[idx].terrain == Terrain::WATER)
            DrawRectangleRec(miniDest, { 37, 70, 184, 255 }); // Water color
         else
            DrawRectangleRec(miniDest, { 33, 122, 0, 255 }); // Land color
      }
   }

   // Draw white outline for main view on minimap
   Rectangle viewRect = { MINIMAP_OFFSET_X + (viewX - VIEW_TILES_X / 2) * MINIMAP_CELL_SIZE,
                        MINIMAP_OFFSET_Y + (viewY - VIEW_TILES_Y / 2) * MINIMAP_CELL_SIZE,
                        VIEW_TILES_X * MINIMAP_CELL_SIZE, VIEW_TILES_Y * MINIMAP_CELL_SIZE };
   DrawRectangleLinesEx(viewRect, 1.0f, WHITE);

   // Draw main view (15x11 at 156x4)
   int startX = viewX - VIEW_TILES_X / 2;
   int startY = viewY - VIEW_TILES_Y / 2;
   for (int y = 0; y < VIEW_TILES_Y; ++y)
   {
      for (int x = 0; x < VIEW_TILES_X; ++x)
      {
         int mapX = startX + x;
         int mapY = startY + y;
         if (mapX < 0 || mapX >= GRID_WIDTH || mapY < 0 || mapY >= GRID_HEIGHT) continue;

         int idx = mapY * GRID_WIDTH + mapX;
         const SquareTile& tile = map[idx];
         Rectangle dest = { VIEW_OFFSET_X + x * TILE_SIZE, VIEW_OFFSET_Y + y * TILE_SIZE, TILE_SIZE, TILE_SIZE };

         // Base terrain
         Rectangle src;
         switch (tile.terrain)
         {
         case Terrain::WATER:
            src = { waterFrame * 16.0f, 24 * 16.0f, 16, 16 }; // Animate 0x24 to 3x24
            break;
         case Terrain::GRASSLAND: src = { 0, 0, 16, 16 }; break;
         case Terrain::DESERT:    src = { 3 * 16.0f, 0, 16, 16 }; break;
         default:                 src = { 0, 0, 16, 16 }; break; // Grassland base for overlays
         }
         DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE);

         // Overlays
         switch (tile.terrain)
         {
         case Terrain::MOUNTAIN: src = { 3 * 16.0f, 1 * 16.0f, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         case Terrain::HILLS:    src = { 6 * 16.0f, 3 * 16.0f, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         case Terrain::SWAMP:    src = { 4 * 16.0f, 2 * 16.0f, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         case Terrain::FOREST:   src = { 5 * 16.0f, 0, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         default: break;
         }

         // Villages (use clan-specific tiles)
         if (tile.hasVillage)
         {
            src = clans[villages[tile.villageIdx].clanIdx].villageTile;
            DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE);
         }
      }
   }

   // Clan stockpile panel (under minimap)
   int yPos = CLAN_PANEL_Y;
   const Clan& redFang = clans[0]; // Assuming Red Fang is clan 0 for now
   DrawTextEx(largeFont, redFang.name.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 18, 1, WHITE);
   yPos += 20; // Space for title

   // Gold
   std::string goldText = "Gold: " + std::to_string(redFang.gold);
   int goldPerTurn = 0; // Placeholder until turn logic is added
   for (const int vIdx : redFang.villages) goldPerTurn += villages[vIdx].goldOutput;
   std::string goldPerTurnText = (goldPerTurn >= 0 ? "+" : "") + std::to_string(goldPerTurn);
   DrawTextEx(gameFont, goldText.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 9, 1, WHITE);
   DrawTextEx(gameFont, goldPerTurnText.c_str(), { float(CLAN_PANEL_X + 80), float(yPos) }, 9, 1, goldPerTurn >= 0 ? WHITE : RED);
   yPos += 12;

   // Knowledge
   std::string knowledgeText = "Knowledge: " + std::to_string(redFang.knowledge);
   int knowledgePerTurn = 0; // Placeholder
   for (const int vIdx : redFang.villages) knowledgePerTurn += villages[vIdx].knowledgeOutput;
   std::string knowledgePerTurnText = (knowledgePerTurn >= 0 ? "+" : "") + std::to_string(knowledgePerTurn);
   DrawTextEx(gameFont, knowledgeText.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 9, 1, WHITE);
   DrawTextEx(gameFont, knowledgePerTurnText.c_str(), { float(CLAN_PANEL_X + 80), float(yPos) }, 9, 1, knowledgePerTurn >= 0 ? WHITE : RED);
   yPos += 12;

   // Worship
   std::string worshipText = "Worship: " + std::to_string(redFang.worship);
   int worshipPerTurn = 0; // Placeholder
   for (const int vIdx : redFang.villages) worshipPerTurn += villages[vIdx].worshipOutput;
   std::string worshipPerTurnText = (worshipPerTurn >= 0 ? "+" : "") + std::to_string(worshipPerTurn);
   DrawTextEx(gameFont, worshipText.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 9, 1, WHITE);
   DrawTextEx(gameFont, worshipPerTurnText.c_str(), { float(CLAN_PANEL_X + 80), float(yPos) }, 9, 1, worshipPerTurn >= 0 ? WHITE : RED);

   // Space below is reserved for village/unit info (to be added later)

   EndTextureMode();
}

int main()
{
   InitWindow(FINAL_WIDTH, FINAL_HEIGHT, "ClanDestiny");
   SetTargetFPS(60);

   // Load the custom fonts
   Font gameFont = LoadFontEx("Data/Fonts/softsquare.ttf", 9, nullptr, 0);
   if (gameFont.texture.id == 0)
   {
      TraceLog(LOG_ERROR, "Failed to load gameFont: Data/Fonts/softsquare.ttf");
      CloseWindow();
      return 1;
   }

   Font largeFont = LoadFontEx("Data/Fonts/softsquare.ttf", 18, nullptr, 0);
   if (largeFont.texture.id == 0)
   {
      TraceLog(LOG_ERROR, "Failed to load largeFont: Data/Fonts/softsquare.ttf");
      UnloadFont(gameFont);
      CloseWindow();
      return 1;
   }

   RenderTexture2D target = LoadRenderTexture(BASE_WIDTH, BASE_HEIGHT);
   Texture2D tileset = LoadTexture("Images/tiles.png");
   std::vector<Clan> clans;
   std::vector<SquareTile> map;
   std::vector<Village> villages;
   std::vector<Unit> units; // Global list of units
   generateMap(map, villages, clans);

   int viewX = GRID_WIDTH / 2;  // Center: 37
   int viewY = GRID_HEIGHT / 2; // Center: 23
   float waterAnimTime = 0.0f;  // Animation timer
   int waterFrame = 0;          // Current water frame (0-3)

   while (!WindowShouldClose())
   {
      // Mouse handling
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
      {
         Vector2 mousePos = GetMousePosition();
         float renderMouseX = mousePos.x / SCALE_FACTOR; // Scale to 640x360
         float renderMouseY = mousePos.y / SCALE_FACTOR;
         int mx = static_cast<int>((renderMouseX - MINIMAP_OFFSET_X) / MINIMAP_CELL_SIZE);
         int my = static_cast<int>((renderMouseY - MINIMAP_OFFSET_Y) / MINIMAP_CELL_SIZE);
         if (mx >= 0 && mx < GRID_WIDTH && my >= 0 && my < GRID_HEIGHT)
         {
            viewX = mx;
            viewY = my;
            // Clamp view to keep it in bounds
            if (viewX < VIEW_TILES_X / 2) viewX = VIEW_TILES_X / 2;
            if (viewX > GRID_WIDTH - VIEW_TILES_X / 2 - 1) viewX = GRID_WIDTH - VIEW_TILES_X / 2 - 1;
            if (viewY < VIEW_TILES_Y / 2) viewY = VIEW_TILES_Y / 2;
            if (viewY > GRID_HEIGHT - VIEW_TILES_Y / 2 - 1) viewY = GRID_HEIGHT - VIEW_TILES_Y / 2 - 1;
         }
      }

      drawView(target, map, tileset, viewX, viewY, waterAnimTime, waterFrame, clans, villages, gameFont, largeFont);

      BeginDrawing();
      ClearBackground(BLACK);
      DrawTexturePro(target.texture,
         { 0, 0, (float)BASE_WIDTH, (float)-BASE_HEIGHT },
         { 0, 0, (float)FINAL_WIDTH, (float)FINAL_HEIGHT },
         { 0, 0 }, 0.0f, WHITE);
      EndDrawing();
   }

   UnloadFont(largeFont);
   UnloadFont(gameFont);
   UnloadTexture(tileset);
   UnloadRenderTexture(target);
   CloseWindow();
   return 0;
}