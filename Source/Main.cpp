#include "../Geist/Source/Engine.h"
#include "../Geist/Source/Globals.h"
#include "../Geist/Source/StateMachine.h"

#include "GameGlobals.h"
#include "MainState.h"

#include "raylib.h"

#include <memory>

int main()
{
    g_Engine = std::make_unique<Engine>();
    g_Engine->Init("engine.cfg");
    ShowCursor();

    MainState* mainState = new MainState();
    mainState->Init("");
    g_StateMachine->RegisterState(STATE_MAINSTATE, mainState, "MainState");
    g_StateMachine->MakeStateTransition(STATE_MAINSTATE);

    while (!g_Engine->m_Done && !WindowShouldClose())
    {
        g_Engine->Update();
        g_Engine->Draw();
    }

    g_Engine->Shutdown();
    return 0;
}