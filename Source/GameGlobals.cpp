#include "GameGlobals.h"

#include "../Geist/Source/Engine.h"
#include "../Geist/Source/Globals.h"
#include "../Geist/Source/InputSystem.h"

std::vector<Clan> g_Clans;
std::vector<SquareTile> g_Map;
std::vector<Village> g_Villages;
Texture2D g_Tileset{};
Font g_GameFont{};
Font g_LargeFont{};
int g_ViewX = GRID_WIDTH / 2;
int g_ViewY = GRID_HEIGHT / 2;
float g_WaterAnimTime = 0.0f;
int g_WaterFrame = 0;
int g_CurrentTurn = 1;
int g_SelectedVillageIdx = -1;

float GetRenderMouseX()
{
    if (!g_Engine || !g_InputSystem)
        return 0.0f;
    return g_InputSystem->m_MouseX / g_Engine->GetInputScale();
}

float GetRenderMouseY()
{
    if (!g_Engine || !g_InputSystem)
        return 0.0f;
    return g_InputSystem->m_MouseY / g_Engine->GetInputScale();
}