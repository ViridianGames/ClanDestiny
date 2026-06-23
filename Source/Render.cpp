#include "Render.h"

void drawView(const std::vector<SquareTile>& map, Texture2D& tileset,
   int viewX, int viewY, float& waterAnimTime, int& waterFrame,
   const std::vector<Clan>& clans, const std::vector<Village>& villages, Font& gameFont, Font& largeFont,
   int selectedVillageIdx, int currentTurn)
{
   waterAnimTime += GetFrameTime();
   if (waterAnimTime >= WATER_ANIM_SPEED)
   {
      waterFrame = (waterFrame + 1) % 4;
      waterAnimTime -= WATER_ANIM_SPEED;
   }

   // Minimap
   for (int y = 0; y < GRID_HEIGHT; ++y)
   {
      for (int x = 0; x < GRID_WIDTH; ++x)
      {
         int idx = y * GRID_WIDTH + x;
         Rectangle miniDest = { MINIMAP_OFFSET_X + x * MINIMAP_CELL_SIZE,
                              MINIMAP_OFFSET_Y + y * MINIMAP_CELL_SIZE,
                              MINIMAP_CELL_SIZE, MINIMAP_CELL_SIZE };
         if (map[idx].hasVillage)
            DrawRectangleRec(miniDest, clans[villages[map[idx].villageIdx].clanIdx].color);
         else if (map[idx].terrain == Terrain::WATER)
            DrawRectangleRec(miniDest, { 37, 70, 184, 255 });
         else
            DrawRectangleRec(miniDest, { 33, 122, 0, 255 });
      }
   }

   Rectangle viewRect = { MINIMAP_OFFSET_X + (viewX - VIEW_TILES_X / 2) * MINIMAP_CELL_SIZE,
                        MINIMAP_OFFSET_Y + (viewY - VIEW_TILES_Y / 2) * MINIMAP_CELL_SIZE,
                        VIEW_TILES_X * MINIMAP_CELL_SIZE, VIEW_TILES_Y * MINIMAP_CELL_SIZE };
   DrawRectangleLinesEx(viewRect, 1.0f, WHITE);

   // Main view
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

         Rectangle src;
         switch (tile.terrain)
         {
         case Terrain::WATER:
            src = { waterFrame * 16.0f, 24 * 16.0f, 16, 16 };
            break;
         case Terrain::GRASSLAND: src = { 0, 0, 16, 16 }; break;
         case Terrain::DESERT:    src = { 3 * 16.0f, 0, 16, 16 }; break;
         default:                 src = { 0, 0, 16, 16 }; break;
         }
         DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE);

         switch (tile.terrain)
         {
         case Terrain::MOUNTAIN: src = { 3 * 16.0f, 1 * 16.0f, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         case Terrain::HILLS:    src = { 6 * 16.0f, 3 * 16.0f, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         case Terrain::SWAMP:    src = { 4 * 16.0f, 2 * 16.0f, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         case Terrain::FOREST:   src = { 5 * 16.0f, 0, 16, 16 }; DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE); break;
         default: break;
         }

         if (tile.hasVillage)
         {
            src = clans[villages[tile.villageIdx].clanIdx].villageTile;
            DrawTexturePro(tileset, src, dest, { 0, 0 }, 0.0f, WHITE);
         }
      }
   }

   // Clan stockpile panel
   int yPos = CLAN_PANEL_Y;
   const Clan& redFang = clans[0];
   DrawTextEx(largeFont, redFang.name.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 18, 1, WHITE);
   yPos += 20;

   std::string goldText = "Gold: " + std::to_string(redFang.gold);
   int goldPerTurn = 0;
   for (const int vIdx : redFang.villages) goldPerTurn += villages[vIdx].goldOutput;
   std::string goldPerTurnText = (goldPerTurn >= 0 ? "+" : "") + std::to_string(goldPerTurn);
   DrawTextEx(gameFont, goldText.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 9, 1, WHITE);
   DrawTextEx(gameFont, goldPerTurnText.c_str(), { float(CLAN_PANEL_X + 80), float(yPos) }, 9, 1, goldPerTurn >= 0 ? WHITE : RED);
   yPos += 12;

   std::string knowledgeText = "Knowledge: " + std::to_string(redFang.knowledge);
   int knowledgePerTurn = 0;
   for (const int vIdx : redFang.villages) knowledgePerTurn += villages[vIdx].knowledgeOutput;
   std::string knowledgePerTurnText = (knowledgePerTurn >= 0 ? "+" : "") + std::to_string(knowledgePerTurn);
   DrawTextEx(gameFont, knowledgeText.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 9, 1, WHITE);
   DrawTextEx(gameFont, knowledgePerTurnText.c_str(), { float(CLAN_PANEL_X + 80), float(yPos) }, 9, 1, knowledgePerTurn >= 0 ? WHITE : RED);
   yPos += 12;

   std::string worshipText = "Worship: " + std::to_string(redFang.worship);
   int worshipPerTurn = 0;
   for (const int vIdx : redFang.villages) worshipPerTurn += villages[vIdx].worshipOutput;
   std::string worshipPerTurnText = (worshipPerTurn >= 0 ? "+" : "") + std::to_string(worshipPerTurn);
   DrawTextEx(gameFont, worshipText.c_str(), { float(CLAN_PANEL_X), float(yPos) }, 9, 1, WHITE);
   DrawTextEx(gameFont, worshipPerTurnText.c_str(), { float(CLAN_PANEL_X + 80), float(yPos) }, 9, 1, worshipPerTurn >= 0 ? WHITE : RED);

   // === Village Info Panel (below clan panel when a village is selected) ===
   if (selectedVillageIdx >= 0 && selectedVillageIdx < (int)villages.size())
   {
      const Village& v = villages[selectedVillageIdx];
      int vy = yPos + 20; // start a bit below the clan worship line

      // Village header
      std::string header = v.name + " (Pop: " + std::to_string(v.population) + ")";
      DrawTextEx(largeFont, header.c_str(), { float(CLAN_PANEL_X), float(vy) }, 14, 1, WHITE);
      vy += 18;

      // Per-turn production (we'll show base + buildings later; for now use stored outputs if present)
      // For immediate feedback, show the village's current output fields
      DrawTextEx(gameFont, ("Food: " + std::to_string(v.foodProduction) + "/turn").c_str(), { float(CLAN_PANEL_X), float(vy) }, 9, 1, WHITE);
      vy += 11;
      DrawTextEx(gameFont, ("Prod: " + std::to_string(v.productionOutput) + "/turn").c_str(), { float(CLAN_PANEL_X), float(vy) }, 9, 1, WHITE);
      vy += 11;
      DrawTextEx(gameFont, ("Gold: " + std::to_string(v.goldOutput) + "/turn").c_str(), { float(CLAN_PANEL_X), float(vy) }, 9, 1, WHITE);
      vy += 11;
      DrawTextEx(gameFont, ("Know: " + std::to_string(v.knowledgeOutput) + "/turn").c_str(), { float(CLAN_PANEL_X), float(vy) }, 9, 1, WHITE);
      vy += 11;
      DrawTextEx(gameFont, ("Worship: " + std::to_string(v.worshipOutput) + "/turn").c_str(), { float(CLAN_PANEL_X), float(vy) }, 9, 1, WHITE);
      vy += 14;

      DrawTextEx(gameFont, ("Food Store: " + std::to_string(v.foodStorehouse)).c_str(), { float(CLAN_PANEL_X), float(vy) }, 9, 1, WHITE);
      vy += 11;
      DrawTextEx(gameFont, ("Prod Store: " + std::to_string(v.productionStorehouse)).c_str(), { float(CLAN_PANEL_X), float(vy) }, 9, 1, WHITE);
   }

   // === End Turn Button (bottom of left panel) ===
   const int BTN_X = 4;
   const int BTN_Y = 330;
   const int BTN_W = 148;
   const int BTN_H = 20;

   DrawRectangle(BTN_X, BTN_Y, BTN_W, BTN_H, DARKGRAY);
   DrawRectangleLines(BTN_X, BTN_Y, BTN_W, BTN_H, WHITE);

   std::string btnText = "End Turn (Turn " + std::to_string(currentTurn) + ")";
   int textWidth = MeasureTextEx(gameFont, btnText.c_str(), 9, 1).x;
   DrawTextEx(gameFont, btnText.c_str(),
              { float(BTN_X + (BTN_W - textWidth) / 2), float(BTN_Y + 5) },
              9, 1, WHITE);
}