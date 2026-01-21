// ---------------------------------------------------------------------------
// includes
// ---------------------------------------------------------------------------

#include <crtdbg.h>        // To check for memory leaks
#include "AEEngine.h"
#include "graphics.hpp"    // Graphics helper for shapes and initialization

#include "mainmenu.hpp"
#include "summer_s1.hpp"

// Global font handle used by all states
s8 gFontId = -1;

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

    // Load font once and share it.
    // Make sure this path points to a valid .ttf in your Assets folder.
    gFontId = AEGfxCreateFont("Assets/Super Mellow.ttf", 24);

    // Game state objects.
    game::MainMenu mainMenu;
    game::SummerS1 summerStage;

    // Enumeration for states.
    enum class GameState
    {
        MainMenu,
        HowToPlay,
        SummerS1,
        Exit
    };

    // Start on the main menu.
    GameState currentState = GameState::MainMenu;

    // Game Loop
    while (gGameRunning)
    {
        // Begin frame.
        AESysFrameStart();

        // Optionally let the window close terminate the game.
        if (AESysDoesWindowExist() == 0)
        {
            gGameRunning = 0;
        }

        // Run current state.
        int action = 0;

        switch (currentState)
        {
        case GameState::MainMenu:
        {
            action = mainMenu.update();
            mainMenu.draw();

            if (action == 1)
            {
                // Go to Summer stage.
                AESysReset();
                currentState = GameState::SummerS1;
            }
            else if (action == 2)
            {
                // Exit selected.
                gGameRunning = 0;
            }
            // action == 3 is reserved for "How To Play".
        }
        break;

        case GameState::SummerS1:
        {
            // Update and draw the first summer stage.
            action = summerStage.update();
            summerStage.draw();

            if (action == 2)
            {
                // Return to main menu from level.
                AESysReset();
                currentState = GameState::MainMenu;
            }
            else if (action == 3)
            {
                // Future: quit from within a level.
                gGameRunning = 0;
            }
        }
        break;

        default:
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

    // Shut down graphics helper.
    gfx::shutdown();

    // Free all engine resources.
    AESysExit();

    return 0;
}
