// ---------------------------------------------------------------------------
// includes
// ---------------------------------------------------------------------------

#include <crtdbg.h>        // To check for memory leaks
#include <memory>           // std::make_unique
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
#include "stageselection.hpp"
#include "credit.hpp"
#include "pause.hpp"


// Global font handle used by all states
s8 gFontId = -1;
s8 gFontTitle = -1;

UI::Dialog UI::gDialog;

enum class SceneState
{
    Splash,
    MainMenu,
    HowToPlay,
    Tutorial1,
    Tutorial2,
    Tutorial3,
    StageSelect,
    Credit,
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

// Convert a seasonal gameplay scene into unlock array index
// Returns -1 for non season scenes like tutorials/menu
static int UnlockIndex(SceneState s)
{
    switch (s)
    {
    case SceneState::WinterS1: return 0;
    case SceneState::WinterS2: return 1;
    case SceneState::WinterS3: return 2;
    case SceneState::WinterS4: return 3;

    case SceneState::SummerS1: return 4;
    case SceneState::SummerS2: return 5;
    case SceneState::SummerS3: return 6;
    case SceneState::SummerS4: return 7;

    case SceneState::SpringS1: return 8;
    case SceneState::SpringS2: return 9;
    case SceneState::SpringS3: return 10;
    case SceneState::SpringS4: return 11;

    case SceneState::AutumnS1: return 12;
    case SceneState::AutumnS2: return 13;
    case SceneState::AutumnS3: return 14;
    case SceneState::AutumnS4: return 15;

    default: return -1;
    }
}

// returns true if stage has already been unlocked
static bool IsStageUnlocked(SceneState s)
{
    int idx = UnlockIndex(s);
    if (idx < 0) return false;
    return gGame.unlockedStages[idx];
}

// marks one seasonal stage as unlocked
static void UnlockStage(SceneState s)
{
    int idx = UnlockIndex(s);
    if (idx >= 0)
        gGame.unlockedStages[idx] = true;
}

//unlock the next stage in the fixed campaign order
static void UnlockNextStage(SceneState clearedScene)
{
    switch (clearedScene)
    {
    case SceneState::Tutorial3: UnlockStage(SceneState::WinterS1); break;

    case SceneState::WinterS1: UnlockStage(SceneState::WinterS2); break;
    case SceneState::WinterS2: UnlockStage(SceneState::WinterS3); break;
    case SceneState::WinterS3: UnlockStage(SceneState::WinterS4); break;
    case SceneState::WinterS4: UnlockStage(SceneState::SummerS1); break;

    case SceneState::SummerS1: UnlockStage(SceneState::SummerS2); break;
    case SceneState::SummerS2: UnlockStage(SceneState::SummerS3); break;
    case SceneState::SummerS3: UnlockStage(SceneState::SummerS4); break;
    case SceneState::SummerS4: UnlockStage(SceneState::SpringS1); break;

    case SceneState::SpringS1: UnlockStage(SceneState::SpringS2); break;
    case SceneState::SpringS2: UnlockStage(SceneState::SpringS3); break;
    case SceneState::SpringS3: UnlockStage(SceneState::SpringS4); break;
    case SceneState::SpringS4: UnlockStage(SceneState::AutumnS1); break;

    case SceneState::AutumnS1: UnlockStage(SceneState::AutumnS2); break;
    case SceneState::AutumnS2: UnlockStage(SceneState::AutumnS3); break;
    case SceneState::AutumnS3: UnlockStage(SceneState::AutumnS4); break;

    default: break;
    }
}

// season is unlocked when its first stage is unlocked
static bool IsSeasonUnlocked(int seasonIndex)
{
    switch (seasonIndex)
    {
    case 0: return gGame.unlockedStages[0];   // WinterS1
    case 1: return gGame.unlockedStages[4];   // SummerS1
    case 2: return gGame.unlockedStages[8];   // SpringS1
    case 3: return gGame.unlockedStages[12];  // AutumnS1
    default: return false;
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

static void ResetPauseState()
{
    gGame.pauseActive = false;
    gGame.pauseShowSettings = false;
    gGame.pauseSelectedIndex = 0;
    gGame.pauseSettingsRow = 0;
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
	gFontTitle = AEGfxCreateFont("Assets/Font/L.ttf" , 80);

    // Audio system + loading (done in audio.cpp)
    audio::init();

    // Game state objects ? allocated on the heap to avoid C6262 stack-size warning.
    auto splashScreenPtr = std::make_unique<game::SplashScreen>();
    auto mainMenuPtr = std::make_unique<game::MainMenu>();
    auto tutorial1Ptr = std::make_unique<game::Tutorial1>();
    auto tutorial2Ptr = std::make_unique<game::Tutorial2>();
    auto tutorial3Ptr = std::make_unique<game::Tutorial3>();
    auto winterStagePtr = std::make_unique<game::WinterS1>();
    auto winterStage2Ptr = std::make_unique<game::WinterS2>();
    auto winterStage3Ptr = std::make_unique<game::WinterS3>();
    auto winterStage4Ptr = std::make_unique<game::WinterS4>();
    auto summerStagePtr = std::make_unique<game::SummerS1>();
    auto summerStage2Ptr = std::make_unique<game::SummerS2>();
    auto summerStage3Ptr = std::make_unique<game::SummerS3>();
    auto summerStage4Ptr = std::make_unique<game::SummerS4>();
    auto autumnStagePtr = std::make_unique<game::AutumnS1>();
    auto autumnStage2Ptr = std::make_unique<game::AutumnS2>();
    auto autumnStage3Ptr = std::make_unique<game::AutumnS3>();
    auto autumnStage4Ptr = std::make_unique<game::AutumnS4>();
    auto thankYouScreenPtr = std::make_unique<game::ThankYouScreen>();
    auto springStagePtr = std::make_unique<game::SpringS1>();
    auto springStage2Ptr = std::make_unique<game::SpringS2>();
    auto springStage3Ptr = std::make_unique<game::SpringS3>();
    auto springStage4Ptr = std::make_unique<game::SpringS4>();
    auto stageSelectPtr = std::make_unique<game::StageSelect>();
    auto creditPtr = std::make_unique<game::Credits>();

    // Convenience references so the rest of the code is unchanged.
    game::SplashScreen& splashScreen = *splashScreenPtr;
    game::MainMenu& mainMenu = *mainMenuPtr;
    game::Tutorial1& tutorial1 = *tutorial1Ptr;
    game::Tutorial2& tutorial2 = *tutorial2Ptr;
    game::Tutorial3& tutorial3 = *tutorial3Ptr;
    game::WinterS1& winterStage = *winterStagePtr;
    game::WinterS2& winterStage2 = *winterStage2Ptr;
    game::WinterS3& winterStage3 = *winterStage3Ptr;
    game::WinterS4& winterStage4 = *winterStage4Ptr;
    game::SummerS1& summerStage = *summerStagePtr;
    game::SummerS2& summerStage2 = *summerStage2Ptr;
    game::SummerS3& summerStage3 = *summerStage3Ptr;
    game::SummerS4& summerStage4 = *summerStage4Ptr;
    game::AutumnS1& autumnStage = *autumnStagePtr;
    game::AutumnS2& autumnStage2 = *autumnStage2Ptr;
    game::AutumnS3& autumnStage3 = *autumnStage3Ptr;
    game::AutumnS4& autumnStage4 = *autumnStage4Ptr;
    game::ThankYouScreen& thankYouScreen = *thankYouScreenPtr;
    game::SpringS1& springStage = *springStagePtr;
    game::SpringS2& springStage2 = *springStage2Ptr;
    game::SpringS3& springStage3 = *springStage3Ptr;
    game::SpringS4& springStage4 = *springStage4Ptr;
    game::StageSelect& stageSelect = *stageSelectPtr;
    game::Credits& credit = *creditPtr;


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

        // Autumn stages
        else if (currentState == SceneState::AutumnS1) {
            g_currentMap = autumnStage.getTileMap();
            g_currentSignID = 40;
        }
        else if (currentState == SceneState::AutumnS2) {
            g_currentMap = autumnStage2.getTileMap();
            g_currentSignID = 41;
        }
        else if (currentState == SceneState::AutumnS3) {
            g_currentMap = autumnStage3.getTileMap();
            g_currentSignID = 42;
        }
        else if (currentState == SceneState::AutumnS4) {
            g_currentMap = autumnStage4.getTileMap();
            g_currentSignID = 43;
        }

        // Begin frame.
        AESysFrameStart();
        f32 dt = (f32)AEFrameRateControllerGetFrameTime();

        // only count time while the player is inside actual season stages.
        // excludes splash screen, main menu, tutorial, and any future pause/settings states.
        if (IsSeasonScene(currentState) && !gGame.pauseActive)
        {
            // timer only runs when not paused
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

            // clear artifacts when changing dialog
            UI::gDialog.reset();

            // 1) Snap camera to the correct vertical band
            camera::setY(h * idx);
            g_currentY = h * static_cast<float>(idx);

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

        // esc controls pause only during gameplay
        if (IsGameplayScene(currentState) && !gTransition.isActive())
        {
            if (AEInputCheckTriggered(AEVK_ESCAPE))
            {
                if (!gGame.pauseActive)
                {
                    // open pause
                    ResetPauseState();
                    gGame.pauseActive = true;
                }
                else
                {
                    // if inside pause settings, esc goes back to pause menu
                    if (gGame.pauseShowSettings)
                    {
                        gGame.pauseShowSettings = false;
                        gGame.pauseSettingsRow = 0;
                    }
                    else
                    {
                        // otherwise close pause
                        ResetPauseState();
                    }
                }
            }
        }

        // --------------------------------------------------------
        // CHEATS: F11 toggles ALL cheats
        // --------------------------------------------------------
        if (!gGame.pauseActive && AEInputCheckTriggered(AEVK_F11))
        {
            gGame.cheatsOn = !gGame.cheatsOn;

            // reset some player state when cheats turn on
            if (gGame.cheatsOn)
            {
                gGame.player.grounded = false;
                gGame.player.dashCount = gGame.player.maxDashCount;
            }
            else
            {
                gGame.noClip = false;
            }
        }

        if (!gGame.pauseActive && gGame.cheatsOn && AEInputCheckTriggered(AEVK_F10))
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

        // Helper: start a PokemonWipe transition to a target scene.
        // Call this instead of setting currentState directly for level switches.
        // Always reset and start a new transition. This avoids situations where
        // back-to-back teleports happen before the previous transition has
        // finished (for example, quickly chaining Winter S2->S3->S4). In such
        // cases, ignoring the second trigger would cause the scene switch to
        // occur without the wipe effect. By resetting and starting a new
        // transition whenever a teleport is requested we ensure the wipe
        // animation always plays.
        auto triggerTransition = [&](SceneState target)
            {
                // Ignore re-trigger if transition already running.
                // Prevents the fade restarting every frame when the player
                // stands still inside the teleport zone.
                if (gTransition.isActive()) return;
                gTransition.start();
                transitionTarget = target;
            };

        UI::gDialog.playerNearSignBoard(false);

        if (gGame.pauseActive)
        {
            pause::Action pauseAction = pause::update();

            if (pauseAction == pause::Resume)
            {
                ResetPauseState();
            }
            else if (pauseAction == pause::MainMenu)
            {
                ResetPauseState();
                triggerTransition(SceneState::MainMenu);
            }
            else if (pauseAction == pause::ExitGame)
            {
                ResetPauseState();
                gGameRunning = 0;
            }
        }

        // ?? Transition: fire scene switch at mid-point, draw overlay on top ??
        gTransition.update(dt);
        if (gTransition.isReadyToSwitch())
        {
            currentState = transitionTarget;
            lastState = SceneState::Exit;  // triggers camera + spawn reset in the block below
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
                stageSelect.reset();
                currentState = SceneState::StageSelect;
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

            if (action == 5) {
                currentState = SceneState::Credit;
            }
        }
        break;

        case SceneState::Tutorial1:
        {
            if (!gGame.pauseActive)
                action = tutorial1.update(dt);

            tutorial1.draw();

            if (!gGame.pauseActive)
            {
                if (action == 30) triggerTransition(SceneState::Tutorial2);
                if (action == 31) triggerTransition(SceneState::Tutorial3);
                if (action == 32) triggerTransition(SceneState::WinterS1);
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::Tutorial2:
        {
            if (!gGame.pauseActive)
                action = tutorial2.update(dt);

            tutorial2.draw();

            if (!gGame.pauseActive)
            {
                if (action == 30) triggerTransition(SceneState::Tutorial2);
                if (action == 31) triggerTransition(SceneState::Tutorial3);
                if (action == 32) triggerTransition(SceneState::WinterS1);
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }

            break;
        }

        case SceneState::Tutorial3:
        {
            if (!gGame.pauseActive)
                action = tutorial3.update(dt);

            tutorial3.draw();

            if (!gGame.pauseActive)
            {
                if (action == 30) triggerTransition(SceneState::Tutorial2);
                if (action == 31) triggerTransition(SceneState::Tutorial3);
                if (action == 32 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::Tutorial3);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::WinterS1);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }


        case SceneState::StageSelect:
        {
            action = stageSelect.update(dt);
            stageSelect.draw();

            if (action == 2) { currentState = SceneState::MainMenu; camera::setY(0.0f); }

            if (action == 10) { currentState = SceneState::WinterS1; lastState = SceneState::Exit; }
            if (action == 11) { currentState = SceneState::WinterS2; lastState = SceneState::Exit; }
            if (action == 12) { currentState = SceneState::WinterS3; lastState = SceneState::Exit; }
            if (action == 13) { currentState = SceneState::WinterS4; lastState = SceneState::Exit; }

            if (action == 20) { currentState = SceneState::SummerS1; lastState = SceneState::Exit; }
            if (action == 21) { currentState = SceneState::SummerS2; lastState = SceneState::Exit; }
            if (action == 22) { currentState = SceneState::SummerS3; lastState = SceneState::Exit; }
            if (action == 23) { currentState = SceneState::SummerS4; lastState = SceneState::Exit; }

            if (action == 30) { currentState = SceneState::SpringS1; lastState = SceneState::Exit; }
            if (action == 31) { currentState = SceneState::SpringS2; lastState = SceneState::Exit; }
            if (action == 32) { currentState = SceneState::SpringS3; lastState = SceneState::Exit; }
            if (action == 33) { currentState = SceneState::SpringS4; lastState = SceneState::Exit; }

            if (action == 40) { currentState = SceneState::AutumnS1; lastState = SceneState::Exit; }
            if (action == 41) { currentState = SceneState::AutumnS2; lastState = SceneState::Exit; }
            if (action == 42) { currentState = SceneState::AutumnS3; lastState = SceneState::Exit; }
            if (action == 43) { currentState = SceneState::AutumnS4; lastState = SceneState::Exit; }



            break;
        }

        case SceneState::Credit:
        {
            action = credit.update(dt);
            credit.draw();
            if (action == 2) { currentState = SceneState::MainMenu; }
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
            if (!gGame.pauseActive)
                action = summerStage.update(dt);

            summerStage.draw();

            if (!gGame.pauseActive)
            {
                if (action == 20 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SummerS1);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SummerS2);
                }

                if (action == 24 && !gTransition.isActive()) triggerTransition(SceneState::WinterS4);
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::SummerS2:
        {
            if (!gGame.pauseActive)
                action = summerStage2.update(dt);

            summerStage2.draw();

            if (!gGame.pauseActive)
            {
                if (action == 21 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SummerS2);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SummerS3);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::SummerS3:
        {
            if (!gGame.pauseActive)
                action = summerStage3.update(dt);

            summerStage3.draw();

            if (!gGame.pauseActive)
            {
                if (action == 22 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SummerS3);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SummerS4);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::SummerS4:
        {
            if (!gGame.pauseActive)
                action = summerStage4.update(dt);

            summerStage4.draw();

            if (!gGame.pauseActive)
            {
                if (action == 25 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SummerS4);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SpringS1);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
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
            if (!gGame.pauseActive)
                action = autumnStage.update(dt);

            autumnStage.draw();

            if (!gGame.pauseActive)
            {
                if (action == 60 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::AutumnS1);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::AutumnS2);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::AutumnS2:
        {
            if (!gGame.pauseActive)
                action = autumnStage2.update(dt);

            autumnStage2.draw();

            if (!gGame.pauseActive)
            {
                if (action == 61 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::AutumnS2);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::AutumnS3);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::AutumnS3:
        {
            if (!gGame.pauseActive)
                action = autumnStage3.update(dt);

            autumnStage3.draw();

            if (!gGame.pauseActive)
            {
                if (action == 62 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::AutumnS3);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::AutumnS4);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::AutumnS4:
        {
            if (!gGame.pauseActive)
                action = autumnStage4.update(dt);

            autumnStage4.draw();

            if (!gGame.pauseActive)
            {
                if (action == 63 && !gTransition.isActive()) triggerTransition(SceneState::ThankYou);
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::ThankYou:
        {
            float dt_ty = static_cast<float>(AEFrameRateControllerGetFrameTime());
            int ty_action = thankYouScreen.update(dt_ty);
            thankYouScreen.draw();
            if (ty_action == 1) triggerTransition(SceneState::MainMenu);
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
            if (!gGame.pauseActive)
                action = springStage.update(dt);

            springStage.draw();

            if (!gGame.pauseActive)
            {
                if (action == 40 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SpringS1);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SpringS2);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::SpringS2:
        {
            if (!gGame.pauseActive)
                action = springStage2.update(dt);

            springStage2.draw();

            if (!gGame.pauseActive)
            {
                if (action == 41 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SpringS2);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SpringS3);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::SpringS3:
        {
            if (!gGame.pauseActive)
                action = springStage3.update(dt);

            springStage3.draw();

            if (!gGame.pauseActive)
            {
                if (action == 42 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SpringS3);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SpringS4);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::SpringS4:
        {
            if (!gGame.pauseActive)
                action = springStage4.update(dt);

            springStage4.draw();

            if (!gGame.pauseActive)
            {
                if (action == 43 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::SpringS4);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::AutumnS1);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
            }
            break;
        }

        case SceneState::WinterS1:
        {
            if (!gGame.pauseActive)
                action = winterStage.update(dt);

            winterStage.draw();

            if (!gGame.pauseActive)
            {
                if (action == 20 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::WinterS1);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::WinterS2);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
                else if (action == 3) gGameRunning = 0;
            }
        }
        break;

        case SceneState::WinterS2:
        {
            if (!gGame.pauseActive)
                action = winterStage2.update(dt);

            winterStage2.draw();

            if (!gGame.pauseActive)
            {
                if (action == 21 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::WinterS2);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::WinterS3);
                }
                if (action == 5 && !gTransition.isActive())  triggerTransition(SceneState::WinterS1);
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
                else if (action == 3) gGameRunning = 0;
            }
            break;
        }

        case SceneState::WinterS3:
        {
            if (!gGame.pauseActive)
                action = winterStage3.update(dt);

            winterStage3.draw();

            if (!gGame.pauseActive)
            {
                if (action == 22 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::WinterS3);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::WinterS4);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
                else if (action == 3) gGameRunning = 0;
            }
            break;
        }

        case SceneState::WinterS4:
        {
            if (!gGame.pauseActive)
                action = winterStage4.update(dt);

            winterStage4.draw();

            if (!gGame.pauseActive)
            {
                if (action == 23 && !gTransition.isActive()) {
                    UnlockNextStage(SceneState::WinterS4);
                    PlayerSaveCheckpoint(gGame.player, "checkpoint.txt");
                    triggerTransition(SceneState::SummerS1);
                }
                if (action == 2 && !gTransition.isActive())  triggerTransition(SceneState::MainMenu);
                else if (action == 3) gGameRunning = 0;
            }
            break;
        }


        }  // End of switch statement


        if (!gGame.pauseActive)
        {
            // stop dialogue animation while paused
            UI::gDialog.update(dt);
        }
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

        if (gGame.pauseActive)
        {
            // draw pause overlay after gameplay and hud
            pause::draw();
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
