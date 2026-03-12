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
#include "thankyou.hpp"
#include "transition.hpp"
#include "camera.hpp"
#include "collision.hpp"
#include "dialogue.hpp"
#include "audio.hpp"
#include "hud.hpp"


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
    ThankYou,
    Exit
};


// Default pending scene (used by some older transition code paths).
// We start the game flow at Tutorial 1, not Winter.
SceneState pendingScene = SceneState::Tutorial1;
SceneState lastState = SceneState::Exit;
Transition      gTransition;
SceneState      transitionTarget = SceneState::MainMenu;


enum StateID {
    MENU = 0,
    WINTER_S1 = 1,
    WINTER_S2 = 2,
    WINTER_S3 = 3,
    WINTER_S4 = 4
};

int static getStateID(SceneState scene) {
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
    case SceneState::ThankYou:  return "ThankYou";

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
    if (s == "ThankYou")   return SceneState::ThankYou;

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
    if (state == SceneState::WinterS1 || state == SceneState::WinterS2 || state == SceneState::WinterS3 || state == SceneState::WinterS4)
    {
        return BgmType::Winter;
    }

    // Summer
    if (state == SceneState::SummerS1 || state == SceneState::SummerS2 || state == SceneState::SummerS3 || state == SceneState::SummerS4)
    {
        return BgmType::Summer;
    }

    // Spring
    if (state == SceneState::SpringS1 || state == SceneState::SpringS2 || state == SceneState::SpringS3 || state == SceneState::SpringS4)
    {
        return BgmType::Spring; // currently silent unless you load a summer track in audio.cpp
    }

    // Autumn
    if (state == SceneState::AutumnS1 || state == SceneState::AutumnS2 || state == SceneState::AutumnS3 || state == SceneState::AutumnS4)
    {
        return BgmType::Autumn; // currently silent unless you load a summer track in audio.cpp
    }

    return BgmType::None;
}

//helper function for hud
static bool IsGameplayScene(SceneState s)
{
    switch (s)
    {
    case SceneState::Tutorial1:
    case SceneState::Tutorial2:
    case SceneState::Tutorial3:
    case SceneState::WinterS1:
    case SceneState::WinterS2:
    case SceneState::WinterS3:
    case SceneState::WinterS4:
    case SceneState::SummerS1:
    case SceneState::SummerS2:
    case SceneState::SummerS3:
    case SceneState::SummerS4:
    case SceneState::SpringS1:
    case SceneState::SpringS2:
    case SceneState::SpringS3:
    case SceneState::SpringS4:
    case SceneState::AutumnS1:
    case SceneState::AutumnS2:
    case SceneState::AutumnS3:
    case SceneState::AutumnS4:
        return true;

    default:
        return false;
    }
}

// new one specifically for game run time, because we dont want to count tutorial
static bool IsSeasonScene(SceneState s)
{
    switch (s)
    {
    case SceneState::WinterS1:
    case SceneState::WinterS2:
    case SceneState::WinterS3:
    case SceneState::WinterS4:
    case SceneState::SummerS1:
    case SceneState::SummerS2:
    case SceneState::SummerS3:
    case SceneState::SummerS4:
    case SceneState::SpringS1:
    case SceneState::SpringS2:
    case SceneState::SpringS3:
    case SceneState::SpringS4:
    case SceneState::AutumnS1:
    case SceneState::AutumnS2:
    case SceneState::AutumnS3:
    case SceneState::AutumnS4:
        return true;

    default:
        return false;
    }
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
    game::ThankYouScreen thankYouScreen;

    game::SpringS1 springStage;
    game::SpringS2 springStage2;
    game::SpringS3 springStage3;
    game::SpringS4 springStage4;


    // Start on the splash screen.
    SceneState currentState = SceneState::Splash;



    // Game Loop
    PlayerInit(gGame.player);

    // Apply saved melon removals to all stage maps after checkpoint is loaded.

    ApplyCollectedMelonsToTileMap("WinterS1", game::WinterS1::gridRows, winterStage.getTileMap());
    ApplyCollectedMelonsToTileMap("WinterS2", game::WinterS2::gridRows, winterStage2.getTileMap());
    ApplyCollectedMelonsToTileMap("WinterS3", game::WinterS3::gridRows, winterStage3.getTileMap());
    ApplyCollectedMelonsToTileMap("WinterS4", game::WinterS4::gridRows, winterStage4.getTileMap());

    ApplyCollectedMelonsToTileMap("SummerS1", game::SummerS1::gridRows, summerStage.getTileMap());
    ApplyCollectedMelonsToTileMap("SummerS2", game::SummerS2::gridRows, summerStage2.getTileMap());
    ApplyCollectedMelonsToTileMap("SummerS3", game::SummerS3::gridRows, summerStage3.getTileMap());
    ApplyCollectedMelonsToTileMap("SummerS4", game::SummerS4::gridRows, summerStage4.getTileMap());

    ApplyCollectedMelonsToTileMap("SpringS1", game::SpringS1::gridRows, springStage.getTileMap());
    ApplyCollectedMelonsToTileMap("SpringS2", game::SpringS2::gridRows, springStage2.getTileMap());
    ApplyCollectedMelonsToTileMap("SpringS3", game::SpringS3::gridRows, springStage3.getTileMap());
    ApplyCollectedMelonsToTileMap("SpringS4", game::SpringS4::gridRows, springStage4.getTileMap());

    ApplyCollectedMelonsToTileMap("AutumnS1", game::AutumnS1::gridRows, autumnStage.getTileMap());
    ApplyCollectedMelonsToTileMap("AutumnS2", game::AutumnS2::gridRows, autumnStage2.getTileMap());
    ApplyCollectedMelonsToTileMap("AutumnS3", game::AutumnS3::gridRows, autumnStage3.getTileMap());
    ApplyCollectedMelonsToTileMap("AutumnS4", game::AutumnS4::gridRows, autumnStage4.getTileMap());

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

        // Spring stages
        else if (currentState == SceneState::SpringS1) {
            g_currentMap = springStage.getTileMap();
            g_currentSignID = 30;
        }
        else if (currentState == SceneState::SpringS2) {
            g_currentMap = springStage2.getTileMap();
            g_currentSignID = 31;
        }
        else if (currentState == SceneState::SpringS3) {
            g_currentMap = springStage3.getTileMap();
            g_currentSignID = 32;
        }
        else if (currentState == SceneState::SpringS4) {
            g_currentMap = springStage4.getTileMap();
            g_currentSignID = 33;
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

        // only count time while the player is inside actual season stages.
        // excludes splash screen, main menu, tutorial, and any future pause/settings states.
        if (IsSeasonScene(currentState))
        {
            gGame.runTimeSeconds += dt;
        }


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
            g_currentY = h * idx;

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

        // Spring levels: V=S1, B=S2, N=S3, M=S4
        if (AEInputCheckTriggered(AEVK_V))
        {
            currentState = SceneState::SpringS1;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_B))
        {
            currentState = SceneState::SpringS2;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_N))
        {
            currentState = SceneState::SpringS3;
            lastState = SceneState::Exit;
        }
        if (AEInputCheckTriggered(AEVK_M))
        {
            currentState = SceneState::SpringS4;
            lastState = SceneState::Exit;
        }
        // T key: test the transition effect (PokemonWipe then goes to MainMenu)
        if (AEInputCheckTriggered(AEVK_T))
        {
            gTransition.style = TransitionStyle::PokemonWipe;
            gTransition.start();
            transitionTarget = SceneState::MainMenu;
        }

        // Optionally let the window close terminate the game.
        if (AESysDoesWindowExist() == 0)
        {
            gGameRunning = 0;
        }

        // Run current state.
        int action = 0;

        UI::gDialog.PLAYERNEARSIGN(false);

        // ?? Transition: fire scene switch at mid-point, draw overlay on top ??
        gTransition.update(dt);
        if (gTransition.isReadyToSwitch())
        {
            currentState = transitionTarget;
            camera::setY(0.0f);
            lastState = SceneState::Exit;
            gTransition.notifySwitch();
        }

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
            if (action == 24)
            {
                // Back-teleport to Winter Stage 4
                currentState = SceneState::WinterS4;
                float h = camera::screenHeight();
                camera::setY(h * 3.0f);
                float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
                float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
                float cellW = (maxX - minX) / 32.0f;
                float cellH = (maxY - minY) / 20.0f;
                gGame.player.pos.x = minX + 2 * cellW + cellW * 0.5f;
                gGame.player.pos.y = h * 3.0f + minY + 2 * cellH + cellH * 0.5f;
                lastState = SceneState::Exit;
            }
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

            if (action == 25) { currentState = SceneState::SpringS1; camera::setY(0.0f); lastState = SceneState::Exit; } // teleport -> SpringS1
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

            if (action == 63) { currentState = SceneState::ThankYou; camera::setY(0.0f); }  // End game
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::ThankYou:
        {
            float dt_ty = AEFrameRateControllerGetFrameTime();
            int ty_action = thankYouScreen.update(dt_ty);
            thankYouScreen.draw();
            if (ty_action == 1) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        // --------------------------------------------------------
        // SPRING (Stages 1..4)
        // action codes from spring.cpp:
        // 40 -> go SpringS2
        // 41 -> go SpringS3
        // 42 -> go SpringS4
        // 43 -> go AutumnS1 (last spring level teleports to Autumn)
        // 2  -> back to MainMenu
        // --------------------------------------------------------
        case SceneState::SpringS1:
        {
            action = springStage.update(dt);
            springStage.draw();

            if (action == 40) { currentState = SceneState::SpringS2; camera::setY(camera::screenHeight()); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::SpringS2:
        {
            action = springStage2.update(dt);
            springStage2.draw();

            if (action == 41) { currentState = SceneState::SpringS3; camera::setY(camera::screenHeight() * 2.0f); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::SpringS3:
        {
            action = springStage3.update(dt);
            springStage3.draw();

            if (action == 42) { currentState = SceneState::SpringS4; camera::setY(camera::screenHeight() * 3.0f); }
            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }
            break;
        }

        case SceneState::SpringS4:
        {
            action = springStage4.update(dt);
            springStage4.draw();

            if (action == 43)
            {
                // Teleport from Spring Stage 4 to Autumn Stage 1
                currentState = SceneState::AutumnS1;
                camera::setY(0.0f);
                lastState = SceneState::Exit;
            }
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

            if (action == 23)
            {
                // Teleport from Winter Stage 4 to Summer Stage 1
                currentState = SceneState::SummerS1;
                camera::setY(0.0f);
                float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
                float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
                float cellW = (maxX - minX) / 32.0f;
                float cellH = (maxY - minY) / 20.0f;
                gGame.player.pos.x = minX + 29 * cellW + cellW * 0.5f;
                gGame.player.pos.y = minY + 2 * cellH + cellH * 0.5f;
                lastState = SceneState::Exit;
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


        }  // End of switch statement


        UI::gDialog.update(dt);
        UI::gDialog.render();

        // draw hud last. putting it here allows it to be present in all season
        if (IsGameplayScene(currentState))
        {
            hud::drawMelonCounter(gGame.player.melonsCollected);
            hud::drawDeathCounter(gGame.player.deathCount);
        }

        // only draw timer during the actual season stages
        if (IsSeasonScene(currentState))
        {
            hud::drawRunTimer(gGame.runTimeSeconds);
        }


        // End frame.
        gTransition.draw();  // overlay drawn last, on top of everything
        AESysFrameEnd();
    }

    // Save latest checkpoint data before shutting down.
    // This also saves the latest run timer.
    if (!gGame.player.checkpointScene.empty())
    {
        PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
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