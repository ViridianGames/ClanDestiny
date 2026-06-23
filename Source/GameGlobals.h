#ifndef _GAMEGLOBALS_H_
#define _GAMEGLOBALS_H_

#include "Clan.h"
#include "Map.h"

#include <vector>

enum GameStates
{
    STATE_MAINSTATE = 0,
    STATE_LASTSTATE
};

extern std::vector<Clan> g_Clans;
extern std::vector<SquareTile> g_Map;
extern std::vector<Village> g_Villages;
extern Texture2D g_Tileset;
extern Font g_GameFont;
extern Font g_LargeFont;
extern int g_ViewX;
extern int g_ViewY;
extern float g_WaterAnimTime;
extern int g_WaterFrame;
extern int g_CurrentTurn;
extern int g_SelectedVillageIdx;

float GetRenderMouseX();
float GetRenderMouseY();

#endif