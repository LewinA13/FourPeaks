// ---------------------------------------------------------------------------
// mainmenu.cpp
// ---------------------------------------------------------------------------
//
// Very simple text-based menu with three options.
// Navigation: Up / Down arrows, confirm with Enter or Space.
// Uses normalized coordinates (-1..1) for AEGfxPrint.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// mainmenu.cpp
// ---------------------------------------------------------------------------

#include "mainmenu.hpp"
#include "AEEngine.h"
#include "AEFrameRateController.h"
#include "sprite.hpp"
#include <cstdint>
#include <cstring>

extern s8 gFontId;
static bool gIsFullscreen = false;


namespace game
{

    //For Animated Background
    static const int BG_FRAME_COUNT = 6;

    static AEGfxTexture* bgFrames[BG_FRAME_COUNT];
    static AEGfxVertexList* bgMesh = nullptr;

    int backgroundFrame = 0;
    float backgroundTimer = 0.0f;
    constexpr int backgroundFrameCount = 6;
    constexpr float backgroundFrameTime = 0.09f;


    // ---------------------------------------------------------------------
    // Low-level print
    // ---------------------------------------------------------------------
    static void printText(f32 x, f32 y, u32 argbColor,
        const char* text, f32 scale = 1.0f)
    {
        f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
        f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
        f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
        f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;

        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    // ---------------------------------------------------------------------
    // Proper centered print (tuned for Alpha Engine default font)
    // ---------------------------------------------------------------------
    static void printCentered(f32 y, u32 color,
        const char* text, f32 scale = 1.0f)
    {
        // Calibrated horizontal correction for AE font
        const f32 correction = 0.12f * scale;

        // Anchor at screen center
        f32 x = -correction;

        printText(x, y, color, text, scale);
    }

    // ---------------------------------------------------------------------
    // Menu entries
    // ---------------------------------------------------------------------
    static const char* const kMenuItems[] =
    {
        "Play",
        "How To Play",
        "Exit"
    };

    static const int kMenuItemCount =
        static_cast<int>(sizeof(kMenuItems) / sizeof(kMenuItems[0]));

    static const u32 kColorNormal = 0xFFFFFFFF;
    static const u32 kColorSelected = 0xFF00FF00;

    // ---------------------------------------------------------------------
    // Constructor
    // ---------------------------------------------------------------------
    MainMenu::MainMenu()
        : selectedIndex(0)
        , showHowTo(false)
    {
        // --------------------------
        // Load Background Frames
        // --------------------------

        bgFrames[0] = AEGfxTextureLoad("Assets/mmf1.png");
        bgFrames[1] = AEGfxTextureLoad("Assets/mmf2.png");
        bgFrames[2] = AEGfxTextureLoad("Assets/mmf3.png");
        bgFrames[3] = AEGfxTextureLoad("Assets/mmf4.png");
        bgFrames[4] = AEGfxTextureLoad("Assets/mmf5.png");
        bgFrames[5] = AEGfxTextureLoad("Assets/mmf6.png");
    }

    // ---------------------------------------------------------------------
    // Fullscreen toggle
    // ---------------------------------------------------------------------
    void ToggleFullscreen()
    {
        gIsFullscreen = !gIsFullscreen;
        AESysSetFullScreen(gIsFullscreen ? 1 : 0);
    }

    // ---------------------------------------------------------------------
    // Update
    // ---------------------------------------------------------------------
    int MainMenu::update()
    {
        float dt = AEFrameRateControllerGetFrameTime();

        backgroundTimer += dt;

        if (backgroundTimer >= backgroundFrameTime)
        {
            backgroundTimer -= backgroundFrameTime;
            backgroundFrame = (backgroundFrame + 1) % backgroundFrameCount;
        }


        if (showHowTo)
        {
            if (AEInputCheckTriggered(AEVK_RETURN) ||
                AEInputCheckTriggered(AEVK_SPACE) ||
                AEInputCheckTriggered(AEVK_ESCAPE))
            {
                showHowTo = false;
            }
            return 0;
        }

        if (AEInputCheckTriggered(AEVK_DOWN))
            selectedIndex = (selectedIndex + 1) % kMenuItemCount;

        if (AEInputCheckTriggered(AEVK_UP))
            selectedIndex = (selectedIndex + kMenuItemCount - 1) % kMenuItemCount;

        if (AEInputCheckTriggered(AEVK_RETURN) ||
            AEInputCheckTriggered(AEVK_SPACE))
        {
            switch (selectedIndex)
            {
            case 0: return 1; // Play
            case 1: showHowTo = true; return 0;
            case 2: return 2; // Exit
            }
        }

        if (AEInputCheckTriggered(AEVK_I))
            ToggleFullscreen();

        return 0;
    }

    // ---------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------
    void MainMenu::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);

        // ------------------------------------------------
        // DRAW ANIMATED BACKGROUND FIRST
        // ------------------------------------------------
        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX();
            float maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY();
            float maxY = AEGfxGetWinMaxY();

            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX), (maxY - minY) };

        }
        // Switch back to color mode for text
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        if (showHowTo)
        {
            drawHowToPlay();
            return;
        }

        // Title
        printCentered(0.6f, 0xFFFFFF00u,
            "Four Seasons Platformer", 1.5f);

        const f32 spacing = 0.25f;
        f32 totalHeight = (kMenuItemCount - 1) * spacing;
        f32 startY = totalHeight * 0.5f;

        for (int i = 0; i < kMenuItemCount; ++i)
        {
            u32 color = (i == selectedIndex)
                ? kColorSelected
                : kColorNormal;

            f32 y = startY - i * spacing;
            printCentered(y, color, kMenuItems[i], 1.0f);
        }
    }

    // ---------------------------------------------------------------------
    // Draw How-To screen
    // ---------------------------------------------------------------------
    void MainMenu::drawHowToPlay() const
    {
        printCentered(0.7f, 0xFF00FFFFu, "How To Play");

        printCentered(0.3f, 0xFFFFFFFFu, "- Use arrow keys to move.");
        printCentered(0.1f, 0xFFFFFFFFu, "- Reach the end of each stage.");
        printCentered(-0.1f, 0xFFFFFFFFu, "- More mechanics coming soon.");

        printCentered(-0.5f, 0xFFFFFF00u,
            "Press Enter, Space or ESC to return.");
    }
}