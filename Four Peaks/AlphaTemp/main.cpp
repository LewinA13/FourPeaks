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
#include "summer.hpp"
#include "camera.hpp"
#include "collision.hpp"
#include "dialogue.hpp"
#include "audio.hpp"


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
    SummerS1,
    SummerS2,
    SummerS3,
    SummerS4,
    Exit
};


// Default pending scene (used by some older transition code paths).
// We start the game flow at Tutorial 1, not Winter.
SceneState pendingScene = SceneState::Tutorial1;
SceneState lastState = SceneState::Exit;


enum StateID {
    MENU = 0,
    WINTER_S1 = 1,
    WINTER_S2 = 2,
    WINTER_S3 = 3,
    WINTER_S4 = 4
};

int getStateID(SceneState scene) {
    switch (scene) {
    case SceneState::WinterS1: return WINTER_S1;
    case SceneState::WinterS2: return WINTER_S2;
    case SceneState::WinterS3: return WINTER_S3;
    case SceneState::WinterS4: return WINTER_S4;
    default: return MENU;
    }
}


//checkpoint saving
static std::string SceneToString(SceneState s)
{
    switch (s)
    {
    case SceneState::Tutorial1: return "Tutorial1";
    case SceneState::Tutorial2: return "Tutorial2";
    case SceneState::Tutorial3: return "Tutorial3";
    case SceneState::WinterS1:  return "WinterS1";
    case SceneState::WinterS2:  return "WinterS2";
    case SceneState::WinterS3:  return "WinterS3";
    case SceneState::WinterS4:  return "WinterS4";
    case SceneState::SummerS1:  return "SummerS1";
    case SceneState::SummerS2:  return "SummerS2";
    case SceneState::SummerS3:  return "SummerS3";
    case SceneState::SummerS4:  return "SummerS4";
    default:                    return "";
    }
}

static SceneState StringToScene(const std::string& s)
{
    if (s == "Tutorial1") return SceneState::Tutorial1;
    if (s == "Tutorial2") return SceneState::Tutorial2;
    if (s == "Tutorial3") return SceneState::Tutorial3;
    if (s == "WinterS1")  return SceneState::WinterS1;
    if (s == "WinterS2")  return SceneState::WinterS2;
    if (s == "WinterS3")  return SceneState::WinterS3;
    if (s == "WinterS4")  return SceneState::WinterS4;
    if (s == "SummerS1")  return SceneState::SummerS1;
    if (s == "SummerS2")  return SceneState::SummerS2;
    if (s == "SummerS3")  return SceneState::SummerS3;
    if (s == "SummerS4")  return SceneState::SummerS4;
    return SceneState::Tutorial1; // default if unrecognised
}


static BgmType Audio_GetDesiredBgmType(SceneState state)
{
    if (state == SceneState::WinterS1 ||
        state == SceneState::WinterS2 ||
        state == SceneState::WinterS3 ||
        state == SceneState::WinterS4)
    {
        return BgmType::Winter;
    }

    // Future:
    // if (state == SceneState::SummerS1 || ... ) return BgmType::Summer;

    return BgmType::None;
}

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

    // Audio system + loading (done in audio.cpp)
    audio::init();

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

    game::SummerS1 summerStage;
    game::SummerS2 summerStage2;
    game::SummerS3 summerStage3;
    game::SummerS4 summerStage4;


    // Start on the main menu.
    SceneState currentState = SceneState::MainMenu;



    // Game Loop
    PlayerInit(gGame.player);
    UI::Dialog dialog;

    // Dialog
    dialog.initialize();



    while (gGameRunning)
    {
        g_currentScene = SceneToString(currentState);

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
            dialog.showForLevel(1);
        }

        else if (currentState == SceneState::WinterS2) {
            g_currentMap = winterStage2.getTileMap();
            dialog.showForLevel(2);
        }

        else if (currentState == SceneState::WinterS3) {
            g_currentMap = winterStage3.getTileMap();
            dialog.showForLevel(3);
        }

        else if (currentState == SceneState::WinterS4) {
            g_currentMap = winterStage4.getTileMap();
            dialog.showForLevel(4);
        }

        // Summer stages (so collision + pickups use the correct current map)
        else if (currentState == SceneState::SummerS1) {
            g_currentMap = summerStage.getTileMap();
        }
        else if (currentState == SceneState::SummerS2) {
            g_currentMap = summerStage2.getTileMap();
        }
        else if (currentState == SceneState::SummerS3) {
            g_currentMap = summerStage3.getTileMap();
        }
        else if (currentState == SceneState::SummerS4) {
            g_currentMap = summerStage4.getTileMap();
        }

        // Begin frame.
        AESysFrameStart();
        f32 dt = (f32)AEFrameRateControllerGetFrameTime();


        // frame audio maintenance + switching
        audio::update(Audio_GetDesiredBgmType(currentState));

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


        if (AEInputCheckTriggered(AEVK_5))
        {
            // Go to Summer Stage 1
            if (currentState != SceneState::SummerS1)
            {
                currentState = SceneState::SummerS1;
                float h = camera::screenHeight();
                camera::setY(0.0f);

                if (lastState == SceneState::SummerS2 || lastState == SceneState::SummerS3 || lastState == SceneState::SummerS4)
                    gGame.player.pos.y -= h;

                lastState = SceneState::Exit;
            }
        }

        if (AEInputCheckTriggered(AEVK_6))
        {
            // Go to Summer Stage 2
            if (currentState != SceneState::SummerS2)
            {
                currentState = SceneState::SummerS2;
                float h = camera::screenHeight();
                camera::setY(h);

                if (lastState == SceneState::SummerS1)
                    gGame.player.pos.y += h;

                lastState = SceneState::Exit;
            }
        }

        if (AEInputCheckTriggered(AEVK_7))
        {
            // Go to Summer Stage 3
            if (currentState != SceneState::SummerS3)
            {
                currentState = SceneState::SummerS3;
                float h = camera::screenHeight();
                camera::setY(h * 2.0f);
                gGame.player.pos.y = h * 2.0f + 100.0f;

                lastState = SceneState::Exit;
            }
        }

        if (AEInputCheckTriggered(AEVK_8))
        {
            // Go to Summer Stage 4
            if (currentState != SceneState::SummerS4)
            {
                currentState = SceneState::SummerS4;
                float h = camera::screenHeight();
                camera::setY(h * 3.0f);
                gGame.player.pos.y = h * 3.0f + 100.0f;

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
                if (!gGame.player.checkpointScene.empty())
                {
                    currentState = StringToScene(gGame.player.checkpointScene);
                    lastState = SceneState::Exit;

                    float h = camera::screenHeight();
                    if (currentState == SceneState::WinterS2 || currentState == SceneState::SummerS2) camera::setY(h);
                    else if (currentState == SceneState::WinterS3 || currentState == SceneState::SummerS3) camera::setY(h * 2.0f);
                    else if (currentState == SceneState::WinterS4 || currentState == SceneState::SummerS4) camera::setY(h * 3.0f);
                    else camera::setY(0.0f);
                }
                else
                {
                    currentState = SceneState::WinterS1;
                    camera::setY(0.0f);
                }
            }
            else if (action == 2)
            {
                // Exit selected.
                gGameRunning = 0;
            }
            // action == 3 is reserved for "How To Play".

            if (action == 4) {
                currentState = SceneState::Tutorial1;
				camera::setY(0.0f);
            }
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

        // --------------------------------------------------------
        // SUMMER (Stages 1..4)
        // action codes from summer.cpp:
        // 20 -> go SummerS2
        // 21 -> go SummerS3
        // 22 -> go SummerS4
        // 2  -> back to MainMenu
        // --------------------------------------------------------
        case SceneState::SummerS1:
        {
            action = summerStage.update(dt);
            summerStage.draw();

            if (action == 20) { currentState = SceneState::SummerS2; camera::setY(camera::screenHeight()); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::SummerS2:
        {
            action = summerStage2.update(dt);
            summerStage2.draw();

            if (action == 21) { currentState = SceneState::SummerS3; camera::setY(camera::screenHeight() * 2.0f); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::SummerS3:
        {
            action = summerStage3.update(dt);
            summerStage3.draw();

            if (action == 22) { currentState = SceneState::SummerS4; camera::setY(camera::screenHeight() * 3.0f); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::SummerS4:
        {
            action = summerStage4.update(dt);
            summerStage4.draw();

            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
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

        dialog.update(dt);
        dialog.render();


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


    // -----------------------
    // AUDIO CLEANUP
    // -----------------------
    audio::shutdown();


    // Free all engine resources.
    AESysExit();

    return 0;
}
