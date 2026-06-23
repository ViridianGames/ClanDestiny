#include "MainState.h"

#include "GameGlobals.h"
#include "Render.h"

#include "../Geist/Source/Engine.h"
#include "../Geist/Source/Globals.h"
#include "../Geist/Source/InputSystem.h"

void MainState::Init(const std::string&)
{
    g_GameFont = LoadFontEx("Data/Fonts/softsquare.ttf", 9, nullptr, 0);
    if (g_GameFont.texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load gameFont: Data/Fonts/softsquare.ttf");
        if (g_Engine)
            g_Engine->m_Done = true;
        return;
    }

    g_LargeFont = LoadFontEx("Data/Fonts/softsquare.ttf", 18, nullptr, 0);
    if (g_LargeFont.texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load largeFont: Data/Fonts/softsquare.ttf");
        if (g_Engine)
            g_Engine->m_Done = true;
        return;
    }

    g_Tileset = LoadTexture("Images/tiles.png");
    generateMap(g_Map, g_Villages, g_Clans);

    for (size_t i = 0; i < g_Villages.size(); ++i)
    {
        if (g_Villages[i].clanIdx >= 0 && g_Villages[i].clanIdx < static_cast<int>(g_Clans.size()))
        {
            VillageProduction production = calculateVillageProduction(g_Villages[i], g_Clans[g_Villages[i].clanIdx]);
            g_Villages[i].foodProduction = production.food;
            g_Villages[i].productionOutput = production.production;
            g_Villages[i].goldOutput = production.gold;
            g_Villages[i].knowledgeOutput = production.knowledge;
            g_Villages[i].worshipOutput = production.worship;
        }
    }

    g_ViewX = GRID_WIDTH / 2;
    g_ViewY = GRID_HEIGHT / 2;
    g_WaterAnimTime = 0.0f;
    g_WaterFrame = 0;
    g_CurrentTurn = 1;
    g_SelectedVillageIdx = -1;
}

void MainState::Shutdown()
{
    if (g_LargeFont.texture.id != 0)
        UnloadFont(g_LargeFont);
    if (g_GameFont.texture.id != 0)
        UnloadFont(g_GameFont);
    if (g_Tileset.id != 0)
        UnloadTexture(g_Tileset);
}

void MainState::OnEnter() {}

void MainState::OnExit() {}

void MainState::Update()
{
    if (!g_InputSystem)
        return;

    if (g_InputSystem->IsLButtonDown())
    {
        const float renderMouseX = GetRenderMouseX();
        const float renderMouseY = GetRenderMouseY();
        const int mx = static_cast<int>((renderMouseX - MINIMAP_OFFSET_X) / MINIMAP_CELL_SIZE);
        const int my = static_cast<int>((renderMouseY - MINIMAP_OFFSET_Y) / MINIMAP_CELL_SIZE);
        if (mx >= 0 && mx < GRID_WIDTH && my >= 0 && my < GRID_HEIGHT)
        {
            g_ViewX = mx;
            g_ViewY = my;
            if (g_ViewX < VIEW_TILES_X / 2) g_ViewX = VIEW_TILES_X / 2;
            if (g_ViewX > GRID_WIDTH - VIEW_TILES_X / 2 - 1) g_ViewX = GRID_WIDTH - VIEW_TILES_X / 2 - 1;
            if (g_ViewY < VIEW_TILES_Y / 2) g_ViewY = VIEW_TILES_Y / 2;
            if (g_ViewY > GRID_HEIGHT - VIEW_TILES_Y / 2 - 1) g_ViewY = GRID_HEIGHT - VIEW_TILES_Y / 2 - 1;
        }
    }

    const int buttonX = 4;
    const int buttonY = 330;
    const int buttonW = 148;
    const int buttonH = 20;
    const float renderMouseX = GetRenderMouseX();
    const float renderMouseY = GetRenderMouseY();

    if (g_InputSystem->IsLButtonJustDown() &&
        renderMouseX >= buttonX && renderMouseX < buttonX + buttonW &&
        renderMouseY >= buttonY && renderMouseY < buttonY + buttonH)
    {
        processEndOfTurn(g_Clans, g_Villages);
        ++g_CurrentTurn;
    }

    const int mainViewX = VIEW_OFFSET_X;
    const int mainViewY = VIEW_OFFSET_Y;
    const int mainViewW = VIEW_TILES_X * TILE_SIZE;
    const int mainViewH = VIEW_TILES_Y * TILE_SIZE;

    if (g_InputSystem->IsLButtonJustDown() &&
        renderMouseX >= mainViewX && renderMouseX < mainViewX + mainViewW &&
        renderMouseY >= mainViewY && renderMouseY < mainViewY + mainViewH)
    {
        const int tileX = (static_cast<int>(renderMouseX) - mainViewX) / TILE_SIZE;
        const int tileY = (static_cast<int>(renderMouseY) - mainViewY) / TILE_SIZE;

        const int mapX = g_ViewX - VIEW_TILES_X / 2 + tileX;
        const int mapY = g_ViewY - VIEW_TILES_Y / 2 + tileY;

        if (mapX >= 0 && mapX < GRID_WIDTH && mapY >= 0 && mapY < GRID_HEIGHT)
        {
            const int idx = mapY * GRID_WIDTH + mapX;
            if (g_Map[idx].hasVillage)
                g_SelectedVillageIdx = g_Map[idx].villageIdx;
        }
    }

    if (g_InputSystem->WasKeyPressed(KEY_ESCAPE) && g_Engine)
        g_Engine->m_Done = true;
}

void MainState::Draw()
{
    drawView(g_Map, g_Tileset, g_ViewX, g_ViewY, g_WaterAnimTime, g_WaterFrame,
        g_Clans, g_Villages, g_GameFont, g_LargeFont, g_SelectedVillageIdx, g_CurrentTurn);
}