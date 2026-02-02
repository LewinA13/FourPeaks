// ---------------------------------------------------------------------------
// includes
// ---------------------------------------------------------------------------

#include <crtdbg.h>        // To check for memory leaks
#include "AEEngine.h"
#include "graphics.hpp"    // Graphics helper for shapes and initialization
#include "player.hpp"
#include "gamestate.hpp"
#include "sprite.hpp"
#include "mainmenu.hpp"
#include "Summer_s1.hpp"
#include "Summer_s2.hpp"
#include "camera.hpp"
#include "collision.hpp"


// Global font handle used by all states
s8 gFontId = -1;

enum class SceneState
{
    MainMenu,
    HowToPlay,
    SummerS1,
    SummerS2,
    Exit
};


SceneState pendingScene = SceneState::SummerS1;
SceneState lastState = SceneState::Exit;


// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Flag to determine if the game should continue running.
    int gGameRunning = 1;


    // Initialize the Alpha Engine.
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);

    // Window title.
    AESysSetWindowTitle("Four Peaks Alpha");

    // Reset all system modules once before starting.
    AESysReset();

    // Initialise the graphics helper.
    gfx::init();
    camera::init();
    camera::setY(0.0f);


	// Initialize sprite system.    
    sprite::init();

    // Load font once and share it.
    // Make sure this path points to a valid .ttf in your Assets folder.
    gFontId = AEGfxCreateFont("Assets/Super Mellow.ttf", 24);

    // Game state objects.
    game::MainMenu mainMenu;
    game::SummerS1 summerStage;
    game::SummerS2 summerStage2;


    // Start on the main menu.
    SceneState currentState = SceneState::MainMenu;


 
    // Game Loop
    PlayerInit(gGame.player);
    while (gGameRunning)
    {

        if (currentState == SceneState::SummerS1) {
            g_currentMap = summerStage.getTileMap();
        }
        else if (currentState == SceneState::SummerS2) {
            g_currentMap = summerStage2.getTileMap();
        }

        // Begin frame.
        AESysFrameStart();
        f32 dt = (f32)AEFrameRateControllerGetFrameTime();

        camera::update(dt);

        // 1) If transition just finished, SWITCH STATE and SPAWN PLAYER
        if (camera::consumeJustFinished())
        {
            currentState = pendingScene;
            lastState = currentState;

            // Spawn player at the correct position for the new stage
            if (currentState == SceneState::SummerS2)
            {
                // Spawn at grid (3, 3) in Stage 2 - BOTTOM LEFT
                float minX = AEGfxGetWinMinX();
                float maxX = AEGfxGetWinMaxX();
                float minY = AEGfxGetWinMinY();
                float maxY = AEGfxGetWinMaxY();

                float cellW = (maxX - minX) / 32.0f;
                float cellH = (maxY - minY) / 20.0f;

                // Grid (3,3) in Stage 2's world space
                float worldX = minX + 3 * cellW + cellW * 0.5f;
                float worldY = camera::screenHeight() + minY + 3 * cellH + cellH * 0.5f;

                gGame.player.pos.x = worldX;
                gGame.player.pos.y = worldY;
            }

            // Reset player physics
            gGame.player.velY = 0.0f;
            gGame.player.grounded = false;
        }

        // 2) Handle direct state entry (pressing keys / menu / etc.) - NO transition
        else if (currentState != lastState)
        {
            if (currentState == SceneState::SummerS2)
            {
                camera::setY(camera::screenHeight());
                if (gGame.player.pos.y < camera::screenHeight() * 0.5f)
                    gGame.player.pos.y += camera::screenHeight();
            }
            else if (currentState == SceneState::SummerS1)
            {
                camera::setY(0.0f);
                if (gGame.player.pos.y > camera::screenHeight() * 0.5f)
                    gGame.player.pos.y -= camera::screenHeight();
            }
            lastState = currentState;
        }

        // 3) APPLY CAMERA
        camera::apply();


        // ------------------------------------------------------------
        // DEBUG STAGE SWITCH (no animation)
        // Press 1 for Stage 1, Press 2 for Stage 2
        // ------------------------------------------------------------
        if (AEInputCheckTriggered(AEVK_2))
        {
            // Go to Stage 2
            if (currentState != SceneState::SummerS2)
            {
                currentState = SceneState::SummerS2;

                // Snap camera to Stage 2 (one screen above Stage 1)
                float h = camera::screenHeight();
                camera::setY(h);

                // Keep player in same screen-relative position
                // If you were in Stage 1, move player up by one screen.
                gGame.player.pos.y += h;

              

                // Force entry logic to run cleanly
                lastState = SceneState::Exit;
            }
        }

        if (AEInputCheckTriggered(AEVK_1))
        {
            // Go to Stage 1
            if (currentState != SceneState::SummerS1)
            {
                currentState = SceneState::SummerS1;

                float h = camera::screenHeight();
                camera::setY(0.0f);

                // Move player down by one screen when returning
                gGame.player.pos.y -= h;

                lastState = SceneState::Exit;
            }
        }


        // Optionally let the window close terminate the game.
        if (AESysDoesWindowExist() == 0)
        {
            gGameRunning = 0;
        }

        // Run current state.
        int action = 0;

        switch (currentState)
        {
        case SceneState::MainMenu:
        {
            action = mainMenu.update();
            mainMenu.draw();

            if (action == 1)
            {
                // Go to Summer stage.
                // AESysReset();
                currentState = SceneState::SummerS1;
            }
            else if (action == 2)
            {
                // Exit selected.
                gGameRunning = 0;
            }
            // action == 3 is reserved for "How To Play".
        }
        break;

        case SceneState::SummerS1:
        {
            action = summerStage.update(dt);
            summerStage.draw();

            // If transitioning, ALSO draw Stage 2 on top
            if (camera::isTransitioning())
            {
                summerStage2.draw();
            }

            // Transition request (you haven't wired this yet, see Fix 2)
            if (action == 20)
            {
                pendingScene = SceneState::SummerS2;
            }

            if (action == 2)
            {
                currentState = SceneState::MainMenu;
                camera::setY(0.0f);
            }

            else if (action == 3)
            {
                gGameRunning = 0;
            }
        }
        break;


        case SceneState::SummerS2:
        {
            action = summerStage2.update(dt);
            summerStage2.draw();

            // Debug back to stage 1 (if you add return 5)
            if (action == 5)
            {
                currentState = SceneState::SummerS1;
                camera::setY(0.0f);
                gGame.player.pos.y -= camera::screenHeight();
            }

            if (action == 2)
            {
                currentState = SceneState::MainMenu;
                camera::setY(0.0f);
            }

            else if (action == 3)
            {
                gGameRunning = 0;
            }
        }
        break;
        } 

        // End frame.
        AESysFrameEnd();
    }

    // Clean up font.
    if (gFontId >= 0)
    {
        AEGfxDestroyFont(gFontId);
        gFontId = -1;
    }

	// Shut down sprite helper.
    sprite::shutdown();

    // Shut down graphics helper.
    gfx::shutdown();

    // Free all engine resources.
    AESysExit();

    return 0;
}
