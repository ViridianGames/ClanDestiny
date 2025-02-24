#ifndef MAP_H
#define MAP_H

#include "Game.h"
#include <vector>

// Tile struct
struct SquareTile
{
   int x, y;
   Vector2 pos;
   Terrain terrain;
   bool hasVillage = false;
   int villageIdx = -1; // Index in villages vector (-1 if no village)
};

void generateMap(std::vector<SquareTile>& map, std::vector<struct Village>& villages, std::vector<struct Clan>& clans);

#endif