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
#include "splash.hpp"
#include "tutorial.hpp"
#include "winter.hpp"
#include "summer.hpp"
#include "spring.hpp"
#include "autumn.hpp"
#include "camera.hpp"
#include "collision.hpp"
#include "dialogue.hpp"
#include "audio.hpp"


// Global font handle used by all states
s8 gFontId = -1;

UI::Dialog UI::gDialog;

enum class SceneState
{
    Splash,
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
    SpringS1,
    SpringS2,
    SpringS3,
    SpringS4,
    AutumnS1,
    AutumnS2,
    AutumnS3,
    AutumnS4,
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
    case SceneState::SpringS1:  return "SpringS1";
    case SceneState::SpringS2:  return "SpringS2";
    case SceneState::SpringS3:  return "SpringS3";
    case SceneState::SpringS4:  return "SpringS4";
    case SceneState::AutumnS1:  return "AutumnS1";
    case SceneState::AutumnS2:  return "AutumnS2";
    case SceneState::AutumnS3:  return "AutumnS3";
    case SceneState::AutumnS4:  return "AutumnS4";

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

    if (s == "SpringS1")  return SceneState::SpringS1;
    if (s == "SpringS2")  return SceneState::SpringS2;
    if (s == "SpringS3")  return SceneState::SpringS3;
    if (s == "SpringS4")  return SceneState::SpringS4;

    if (s == "AutumnS1")  return SceneState::AutumnS1;
    if (s == "AutumnS2")  return SceneState::AutumnS2;
    if (s == "AutumnS3")  return SceneState::AutumnS3;
    if (s == "AutumnS4")  return SceneState::AutumnS4;

    return SceneState::Tutorial1; // fallback
}

// save what stage number we are in, regardless of season
static int StageIndex(SceneState s)
{
    switch (s)
    {
    case SceneState::WinterS1:  case SceneState::SummerS1:  case SceneState::SpringS1:  case SceneState::AutumnS1:  return 0;
    case SceneState::WinterS2:  case SceneState::SummerS2:  case SceneState::SpringS2:  case SceneState::AutumnS2:  return 1;
    case SceneState::WinterS3:  case SceneState::SummerS3:  case SceneState::SpringS3:  case SceneState::AutumnS3:  return 2;
    case SceneState::WinterS4:  case SceneState::SummerS4:  case SceneState::SpringS4:  case SceneState::AutumnS4:  return 3;
    default: return 0;
    }
}

static gfx::Vec2 GridToWorld(int gridX, int gridY, float screenYOffset)
{
    // Use constant screen half-dimensions instead of AEGfxGetWinMin/Max,
    // because those return stale values until camera::apply() is called.
    float halfW = camera::screenWidth() * 0.5f;
    float halfH = camera::screenHeight() * 0.5f;

    float minX = -halfW;
    float minY = -halfH;

    float cellW = camera::screenWidth() / 32.0f;
    float cellH = camera::screenHeight() / 20.0f;

    float worldX = minX + gridX * cellW + cellW * 0.5f;
    float worldY = screenYOffset + minY + gridY * cellH + cellH * 0.5f;

    return { worldX, worldY };
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

    // Summer / Spring / Autumn
    if (state == SceneState::SummerS1 || state == SceneState::SummerS2 || state == SceneState::SummerS3 || state == SceneState::SummerS4 ||
        state == SceneState::SpringS1 || state == SceneState::SpringS2 || state == SceneState::SpringS3 || state == SceneState::SpringS4 ||
        state == SceneState::AutumnS1 || state == SceneState::AutumnS2 || state == SceneState::AutumnS3 || state == SceneState::AutumnS4)
    {
        return BgmType::Summer; // currently silent unless you load a summer track in audio.cpp
    }

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
    gFontId = AEGfxCreateFont("Assets/Font/L.ttf", 24);

    // Audio system + loading (done in audio.cpp)
    audio::init();

    // Game state objects.
    game::SplashScreen splashScreen;
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

    game::AutumnS1 autumnStage;
    game::AutumnS2 autumnStage2;
    game::AutumnS3 autumnStage3;
    game::AutumnS4 autumnStage4;


    // Start on the splash screen.
    SceneState currentState = SceneState::Splash;



    // Game Loop
    PlayerInit(gGame.player);

    // gDialog
    UI::gDialog.initialize();



    while (gGameRunning)
    {
        g_currentScene = SceneToString(currentState);
        // =========================================================
        // Tutorial stage
        // =========================================================
        if (currentState == SceneState::Tutorial1) {
            g_currentMap = tutorial1.getTileMap();
            g_currentSignID = 0;
        }
        else if (currentState == SceneState::Tutorial2) {
            g_currentMap = tutorial2.getTileMap();
            g_currentSignID = 1;
        }
        else if (currentState == SceneState::Tutorial3) {
            g_currentMap = tutorial3.getTileMap();
            g_currentSignID = 2;
        }

        // =========================================================
        // Winter stage
        // =========================================================
        else if (currentState == SceneState::WinterS1) {
            g_currentMap = winterStage.getTileMap();
            g_currentSignID = 10;
        }

        else if (currentState == SceneState::WinterS2) {
            g_currentMap = winterStage2.getTileMap();
            g_currentSignID = 11;
        }

        else if (currentState == SceneState::WinterS3) {
            g_currentMap = winterStage3.getTileMap();
            g_currentSignID = 12;
        }

        else if (currentState == SceneState::WinterS4) {
            g_currentMap = winterStage4.getTileMap();
            g_currentSignID = 13;
        }

        else if (currentState == SceneState::AutumnS1) {
            g_currentMap = autumnStage.getTileMap();
        }
        else if (currentState == SceneState::AutumnS2) {
            g_currentMap = autumnStage2.getTileMap();
        }
        else if (currentState == SceneState::AutumnS3) {
            g_currentMap = autumnStage3.getTileMap();
        }
        else if (currentState == SceneState::AutumnS4) {
            g_currentMap = autumnStage4.getTileMap();
        }

        // Summer stages (so collision + pickups use the correct current map)
        // ==================================================================
        else if (currentState == SceneState::SummerS1) {
            g_currentMap = summerStage.getTileMap();
            g_currentSignID = 20;
        }
        else if (currentState == SceneState::SummerS2) {
            g_currentMap = summerStage2.getTileMap();
            g_currentSignID = 21;
        }
        else if (currentState == SceneState::SummerS3) {
            g_currentMap = summerStage3.getTileMap();
            g_currentSignID = 22;
        }
        else if (currentState == SceneState::SummerS4) {
            g_currentMap = summerStage4.getTileMap();
            g_currentSignID = 23;
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
            const float h = camera::screenHeight();
            const int idx = StageIndex(currentState);

            // 1) Snap camera to the correct vertical band
            camera::setY(h * idx);

            // 2) Give EVERY playable scene a default spawn
            gfx::Vec2 spawn;

            switch (currentState)
            {

            /* ---------- TUTORIAL ---------- */
            case SceneState::Tutorial1: spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::Tutorial2: spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::Tutorial3: spawn = GridToWorld(2, 2, h * idx); break;

            /* ---------- WINTER ---------- */
            case SceneState::WinterS1:  spawn = GridToWorld(3, 3, h * idx); break;
            case SceneState::WinterS2:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::WinterS3:  spawn = GridToWorld(1, 6, h * idx); break;
            case SceneState::WinterS4:  spawn = GridToWorld(3, 3, h * idx); break;

            /* ---------- SUMMER ---------- */
            case SceneState::SummerS1:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::SummerS2:  spawn = GridToWorld(2, 17, h * idx); break;
            case SceneState::SummerS3:  spawn = GridToWorld(1, 16, h * idx); break;
            case SceneState::SummerS4:  spawn = GridToWorld(2, 2, h * idx); break;

            /* ---------- SPRING ---------- */
            case SceneState::SpringS1:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::SpringS2:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::SpringS3:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::SpringS4:  spawn = GridToWorld(2, 2, h * idx); break;

            /* ---------- AUTUMN ---------- */
            case SceneState::AutumnS1:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::AutumnS2:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::AutumnS3:  spawn = GridToWorld(2, 2, h * idx); break;
            case SceneState::AutumnS4:  spawn = GridToWorld(2, 2, h * idx); break;

            default:
                spawn = GridToWorld(2, 2, h * idx);
                break;
            }

            // Prefer using your helper so feet align nicely with tiles
            PlayerSetFeetWorld(gGame.player, spawn);

            // Also update respawn so dying in this scene returns here
            PlayerSetRespawn(gGame.player, spawn);

            // 3) Reset player physics
            gGame.player.velY = 0.0f;
            gGame.player.grounded = false;

            lastState = currentState;
        }
        

        // Apply camera
        camera::apply();

        // --------------------------------------------------------
        // CHEATS: F11 toggles ALL cheats
        // --------------------------------------------------------
        if (AEInputCheckTriggered(AEVK_F11))
        {
            gGame.cheatsOn = !gGame.cheatsOn;

            // Optional: clean up physics instantly when enabling cheats
            if (gGame.cheatsOn)
            {
                gGame.player.grounded = false;
                gGame.player.dashCount = gGame.player.maxDashCount;
            }
        }

        if (gGame.cheatsOn && AEInputCheckTriggered(AEVK_F10))
        {
            gGame.noClip = !gGame.noClip;
        }

        // --------------------------------------------------------
        // DEBUG: STAGE SWITCH (no animation)
        // Tutorial: F1/F2/F3
        // Winter:   1/2/3/4
        // --------------------------------------------------------

        if (AEInputCheckTriggered(AEVK_F1))
        {
            currentState = SceneState::Tutorial1;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F2))
        {
            currentState = SceneState::Tutorial2;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F3))
        {
            currentState = SceneState::Tutorial3;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F4))
        {
            currentState = SceneState::AutumnS1;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F5))
        {
            currentState = SceneState::AutumnS2;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F6))
        {
            currentState = SceneState::AutumnS3;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_F7))
        {
            currentState = SceneState::AutumnS4;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_1))
        {
            currentState = SceneState::WinterS1;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_2))
        {
            currentState = SceneState::WinterS2;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_3))
        {
            currentState = SceneState::WinterS3;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_4))
        {
            currentState = SceneState::WinterS4;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_5))
        {
            currentState = SceneState::SummerS1;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_6))
        {
            currentState = SceneState::SummerS2;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_7))
        {
            currentState = SceneState::SummerS3;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_8))
        {
            currentState = SceneState::SummerS4;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_9))
        {
            currentState = SceneState::SpringS1;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_0))
        {
            currentState = SceneState::AutumnS1;
            lastState = SceneState::Exit;
        }

        // Optionally let the window close terminate the game.
        if (AESysDoesWindowExist() == 0)
        {
            gGameRunning = 0;
        }

        // Run current state.
        int action = 0;

        UI::gDialog.PLAYERNEARSIGN(false);

        switch (currentState)
        {
        case SceneState::Splash:
        {
            action = splashScreen.update();
            splashScreen.draw();

            if (action == 1)
                currentState = SceneState::MainMenu;
        }
        break;

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

        // --------------------------------------------------------
        // AUTUMN (Stages 1..4)
        // action codes from autumn.cpp:
        // 60 -> go AutumnS2
        // 61 -> go AutumnS3
        // 62 -> go AutumnS4
        // 2  -> back to MainMenu
        // --------------------------------------------------------
        case SceneState::AutumnS1:
        {
            action = autumnStage.update(dt);
            autumnStage.draw();

            if (action == 60) { currentState = SceneState::AutumnS2; camera::setY(camera::screenHeight()); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::AutumnS2:
        {
            action = autumnStage2.update(dt);
            autumnStage2.draw();

            if (action == 61) { currentState = SceneState::AutumnS3; camera::setY(camera::screenHeight() * 2.0f); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::AutumnS3:
        {
            action = autumnStage3.update(dt);
            autumnStage3.draw();

            if (action == 62) { currentState = SceneState::AutumnS4; camera::setY(camera::screenHeight() * 3.0f); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::AutumnS4:
        {
            action = autumnStage4.update(dt);
            autumnStage4.draw();

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


        UI::gDialog.update(dt);
        UI::gDialog.render();


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