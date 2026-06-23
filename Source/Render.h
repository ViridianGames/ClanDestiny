#ifndef RENDER_H
#define RENDER_H

#include "Game.h"
#include "Map.h"
#include "Clan.h"

void drawView(const std::vector<SquareTile>& map, Texture2D& tileset,
   int viewX, int viewY, float& waterAnimTime, int& waterFrame,
   const std::vector<Clan>& clans, const std::vector<Village>& villages, Font& gameFont, Font& largeFont,
   int selectedVillageIdx, int currentTurn);

#endif