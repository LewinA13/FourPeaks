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
#include "tutorial.hpp"
#include "winter.hpp"
#include "camera.hpp"
#include "collision.hpp"


// Global font handle used by all states
s8 gFontId = -1;

enum class SceneState
{
    MainMenu,
    HowToPlay,
    Tutorial1,
    Tutorial2,
    Tutorial3,
    WinterS1,
    WinterS2,
    WinterS3,
    WinterS4,
    Exit
};


// Default pending scene (used by some older transition code paths).
// We start the game flow at Tutorial 1, not Winter.
SceneState pendingScene = SceneState::Tutorial1;
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

    // Tutorial stages (3 levels before Winter)
    game::Tutorial1 tutorial1;
    game::Tutorial2 tutorial2;
    game::Tutorial3 tutorial3;

    game::WinterS1 winterStage;
    game::WinterS2 winterStage2;
    game::WinterS3 winterStage3;
    game::WinterS4 winterStage4;


    // Start on the main menu.
    SceneState currentState = SceneState::MainMenu;



    // Game Loop
    PlayerInit(gGame.player);
    while (gGameRunning)
    {

        if (currentState == SceneState::Tutorial1) {
            g_currentMap = tutorial1.getTileMap();
        }
        else if (currentState == SceneState::Tutorial2) {
            g_currentMap = tutorial2.getTileMap();
        }
        else if (currentState == SceneState::Tutorial3) {
            g_currentMap = tutorial3.getTileMap();
        }
        else if (currentState == SceneState::WinterS1) {
            g_currentMap = winterStage.getTileMap();
        }
        else if (currentState == SceneState::WinterS2) {
            g_currentMap = winterStage2.getTileMap();
        }
        else if (currentState == SceneState::WinterS3) {
            g_currentMap = winterStage3.getTileMap();
        }
        else if (currentState == SceneState::WinterS4) {
            g_currentMap = winterStage4.getTileMap();
        }

        // Begin frame.
        AESysFrameStart();
        f32 dt = (f32)AEFrameRateControllerGetFrameTime();

        camera::update(dt);

        // Handle state changes (both transition-based and direct)
        if (currentState != lastState)
        {
            if (currentState == SceneState::WinterS2)
            {
                // Set camera to Stage 2
                camera::setY(camera::screenHeight());

                // Spawn player at grid stage 2 - change here for spawn location - A
                float minX = AEGfxGetWinMinX();
                float maxX = AEGfxGetWinMaxX();
                float minY = AEGfxGetWinMinY();
                float maxY = AEGfxGetWinMaxY();

                float cellW = (maxX - minX) / 32.0f;
                float cellH = (maxY - minY) / 20.0f;

                // Grid (3,3) in Stage 2's world space
                float worldX = minX + 2 * cellW + cellW * 0.5f;
                float worldY = camera::screenHeight() + minY + 1 * cellH + cellH * 0.5f;

                gGame.player.pos.x = worldX;
                gGame.player.pos.y = worldY;
            }
            else if (currentState == SceneState::WinterS1)
            {
                camera::setY(0.0f);
                // Optionally set spawn position for Stage 1 too
            }

            // Reset player physics
            gGame.player.velY = 0.0f;
            gGame.player.grounded = false;

            lastState = currentState;
        }

        // Apply camera
        camera::apply();


        // --------------------------------------------------------
        // DEBUG: STAGE SWITCH (no animation)
        // Tutorial: F1/F2/F3
        // Winter:   1/2/3/4
        // --------------------------------------------------------

        if (AEInputCheckTriggered(AEVK_F1))
        {
            currentState = SceneState::Tutorial1;
            camera::setY(0.0f);
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F2))
        {
            currentState = SceneState::Tutorial2;
            camera::setY(0.0f);
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F3))
        {
            currentState = SceneState::Tutorial3;
            camera::setY(0.0f);
            lastState = SceneState::Exit;
        }

        if (AEInputCheckTriggered(AEVK_1))
        {
            // Go to Stage 1
            if (currentState != SceneState::WinterS1)
            {
                currentState = SceneState::WinterS1;
                float h = camera::screenHeight();
                camera::setY(0.0f);
                // If you came from another Winter stage, keep screen-relative position.
                // If you came from Tutorial/Menu, don't apply the offset.
                if (lastState == SceneState::WinterS2 || lastState == SceneState::WinterS3 || lastState == SceneState::WinterS4)
                    gGame.player.pos.y -= h;
                lastState = SceneState::Exit;
            }
        }

        if (AEInputCheckTriggered(AEVK_2))
        {
            // Go to Stage 2
            if (currentState != SceneState::WinterS2)
            {
                currentState = SceneState::WinterS2;
                // Snap camera to Stage 2 (one screen above Stage 1)
                float h = camera::screenHeight();
                camera::setY(h);
                // Keep player in same screen-relative position
                // If you were in Stage 1, move player up by one screen.
                if (lastState == SceneState::WinterS1)
                    gGame.player.pos.y += h;
                // Force entry logic to run cleanly
                lastState = SceneState::Exit;
            }
        }

        if (AEInputCheckTriggered(AEVK_3))
        {
            // Go to Stage 3
            if (currentState != SceneState::WinterS3)
            {
                currentState = SceneState::WinterS3;
                float h = camera::screenHeight();
                camera::setY(h * 2.0f);  // Two screens above Stage 1
                gGame.player.pos.y = h * 2.0f + 100.0f;  // Spawn at Stage 3
                lastState = SceneState::Exit;
            }
        }

        if (AEInputCheckTriggered(AEVK_4))
        {
            // Go to Stage 4
            if (currentState != SceneState::WinterS4)
            {
                currentState = SceneState::WinterS4;
                float h = camera::screenHeight();
                camera::setY(h * 3.0f);  // Three screens above Stage 1
                gGame.player.pos.y = h * 3.0f + 100.0f;  // Spawn at Stage 4
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
                // Go to Tutorial 1 first.
                currentState = SceneState::Tutorial1;
                camera::setY(0.0f);
            }
            else if (action == 2)
            {
                // Exit selected.
                gGameRunning = 0;
            }
            // action == 3 is reserved for "How To Play".
        }
        break;

        case SceneState::Tutorial1:
        {
            action = tutorial1.update(dt);
            tutorial1.draw();

            if (action == 30) { currentState = SceneState::Tutorial2; camera::setY(0.0f); }
            if (action == 31) { currentState = SceneState::Tutorial3; camera::setY(0.0f); }
            if (action == 32) { currentState = SceneState::WinterS1;  camera::setY(0.0f); }
            if (action == 2)
            {
                currentState = SceneState::MainMenu;
                camera::setY(0.0f);
            }
            break;
        }

        case SceneState::Tutorial2:
        {
            action = tutorial2.update(dt);
            tutorial2.draw();

            if (action == 30) { currentState = SceneState::Tutorial2; camera::setY(0.0f); }
            if (action == 31) { currentState = SceneState::Tutorial3; camera::setY(0.0f); }
            if (action == 32) { currentState = SceneState::WinterS1;  camera::setY(0.0f); }
            if (action == 2)
            {
                currentState = SceneState::MainMenu;
                camera::setY(0.0f);
            }
            break;
        }

        case SceneState::Tutorial3:
        {
            action = tutorial3.update(dt);
            tutorial3.draw();

            if (action == 30) { currentState = SceneState::Tutorial2; camera::setY(0.0f); }
            if (action == 31) { currentState = SceneState::Tutorial3; camera::setY(0.0f); }
            if (action == 32) { currentState = SceneState::WinterS1;  camera::setY(0.0f); }
            if (action == 2)
            {
                currentState = SceneState::MainMenu;
                camera::setY(0.0f);
            }
            break;
        }

        case SceneState::WinterS1:
        {
            action = winterStage.update(dt);
            winterStage.draw();

            // Handle action == 20 (instant teleport to Stage 2)
            if (action == 20)
            {
                currentState = SceneState::WinterS2;
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

        case SceneState::WinterS2:
        {
            action = winterStage2.update(dt);
            winterStage2.draw();

            // ADD THIS: Handle teleport to Stage 3
            if (action == 21)
            {
                currentState = SceneState::WinterS3;
            }

            // Debug: back to stage 1 if you add return 5
            if (action == 5)
            {
                currentState = SceneState::WinterS1;
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
            break;
        }

        case SceneState::WinterS3:
        {
            action = winterStage3.update(dt);
            winterStage3.draw();

            // ADD THIS: Handle teleport to Stage 4
            if (action == 22)
            {
                currentState = SceneState::WinterS4;
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
            break;
        }

        case SceneState::WinterS4:
        {
            action = winterStage4.update(dt);
            winterStage4.draw();

            if (action == 2)
            {
                currentState = SceneState::MainMenu;
                camera::setY(0.0f);
            }
            else if (action == 3)
            {
                gGameRunning = 0;
            }
            break;
        }


        }  // End of switch statement


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
