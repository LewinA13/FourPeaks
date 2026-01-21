// ---------------------------------------------------------------------------
// mainmenu.cpp
// ---------------------------------------------------------------------------
//
// Very simple text-based menu with three options.
// Navigation: Up / Down arrows, confirm with Enter or Space.
// Uses normalized coordinates (-1..1) for AEGfxPrint.
// ---------------------------------------------------------------------------

#include "mainmenu.hpp"
#include "AEEngine.h"
#include <cstdint>

extern s8 gFontId;      // Font handle created in main.cpp

namespace game
{
    // Helper for drawing ARGB-colour text using the shared font.
    // x,y are in normalized coordinates (-1..1).
    static void printText(f32 x, f32 y, u32 argbColor,
        const char* text, f32 scale = 1.0f)
    {
        f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
        f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
        f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
        f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;

        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    // Menu entries
    static const char* const kMenuItems[] =
    {
        "Play",
        "How To Play",
        "Exit"
    };

    static const int kMenuItemCount =
        static_cast<int>(sizeof(kMenuItems) / sizeof(kMenuItems[0]));

    static const u32 kColorNormal = 0xFFFFFFFF; // white
    static const u32 kColorSelected = 0xFF00FF00; // green

    MainMenu::MainMenu()
        : selectedIndex(0)
        , showHowTo(false)
    {
    }

    // -------------------------------------------------------------------
    // MainMenu::update
    // -------------------------------------------------------------------
    int MainMenu::update()
    {
        // If How To Play is visible, wait for any confirm key to close it.
        if (showHowTo)
        {
            if (AEInputCheckTriggered(AEVK_RETURN) ||
                AEInputCheckTriggered(AEVK_SPACE) ||
                AEInputCheckTriggered(AEVK_ESCAPE))
            {
                showHowTo = false;
            }
            return 0; // stay on menu
        }

        // Move selection down.
        if (AEInputCheckTriggered(AEVK_DOWN))
        {
            selectedIndex = (selectedIndex + 1) % kMenuItemCount;
        }

        // Move selection up.
        if (AEInputCheckTriggered(AEVK_UP))
        {
            selectedIndex = (selectedIndex + kMenuItemCount - 1) % kMenuItemCount;
        }

        // Confirm selection.
        if (AEInputCheckTriggered(AEVK_RETURN) ||
            AEInputCheckTriggered(AEVK_SPACE))
        {
            switch (selectedIndex)
            {
            case 0: // Play
                return 1;
            case 1: // How To Play (placeholder)
                showHowTo = true;
                return 0;
            case 2: // Exit
                return 2;
            default:
                break;
            }
        }

        return 0; // no state change
    }

    // -------------------------------------------------------------------
    // MainMenu::draw
    // -------------------------------------------------------------------
    void MainMenu::draw() const
    {
        // Background colour.
        AEGfxSetBackgroundColor(0.05f, 0.05f, 0.2f);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        if (showHowTo)
        {
            drawHowToPlay();
            return;
        }

        // Title near top-center (normalized coordinates).
        printText(-0.6f, 0.8f, 0xFFFFFF00u, "Four Seasons Platformer");

        // Menu entries vertically centered.
        f32 baseY = 0.2f;   // first item
        f32 spacing = -0.25f; // distance between items

        for (int i = 0; i < kMenuItemCount; ++i)
        {
            u32 color = (i == selectedIndex) ? kColorSelected : kColorNormal;
            f32 y = baseY + spacing * i;

            // x = -0.3 places text slightly left of center.
            printText(-0.3f, y, color, kMenuItems[i]);
        }
    }

    // -------------------------------------------------------------------
    // MainMenu::drawHowToPlay
    // -------------------------------------------------------------------
    void MainMenu::drawHowToPlay() const
    {
        // Simple placeholder screen in normalized coordinates.
        printText(-0.4f, 0.7f, 0xFF00FFFFu, "How To Play (WIP)");

        printText(-0.9f, 0.3f, 0xFFFFFFFFu,
            "- Use arrow keys to move (later).");
        printText(-0.9f, 0.1f, 0xFFFFFFFFu,
            "- Reach the end of each seasonal stage.");
        printText(-0.9f, -0.1f, 0xFFFFFFFFu,
            "- Future screens will explain mechanics.");

        printText(-0.9f, -0.5f, 0xFFFFFF00u,
            "Press Enter, Space or ESC to return.");
    }
}
