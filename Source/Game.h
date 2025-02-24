#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <string>

// Terrain types
enum class Terrain
{
   WATER, DESERT, GRASSLAND, FOREST, SWAMP, HILLS, MOUNTAIN
};

// Special Abilities for Units
enum class SpecialAbility
{
   BUILD_VILLAGE, FLY, CAST_SPELL, BUFF_STACK
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
const int CLAN_PANEL_X = MINIMAP_OFFSET_X;
const int CLAN_PANEL_Y = MINIMAP_OFFSET_Y + GRID_HEIGHT * MINIMAP_CELL_SIZE + 4;

#endif