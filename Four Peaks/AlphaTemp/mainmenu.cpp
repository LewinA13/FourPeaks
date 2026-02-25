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
#include "AEFrameRateController.h"
#include "sprite.hpp"
#include <cstdint>
#include <cstring>

extern s8 gFontId;
static bool gIsFullscreen = false;


namespace game
{

    // -------------------------------------------------------------------------
    // Animated Background
    // -------------------------------------------------------------------------
    static const int BG_FRAME_COUNT = 6;

    static AEGfxTexture* bgFrames[BG_FRAME_COUNT];
    static AEGfxVertexList* bgMesh = nullptr;

    int   backgroundFrame = 0;
    float backgroundTimer = 0.0f;
    constexpr int   backgroundFrameCount = 6;
    constexpr float backgroundFrameTime = 0.09f;   // seconds per frame (~11 fps)


    // -------------------------------------------------------------------------
    // Low-level print
    // -------------------------------------------------------------------------
    static void printText(f32 x, f32 y, u32 argbColor,
        const char* text, f32 scale = 1.0f)
    {
        f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
        f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
        f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
        f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;
        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    // -------------------------------------------------------------------------
    // Returns the half-width of a string so callers can compute centered x.
    // charWidth is the width of one character at scale 1.0 in AE's coord space.
    // Tune this single constant if text drifts left or right globally.
    // -------------------------------------------------------------------------
    static f32 textHalfWidth(const char* text, f32 scale = 1.0f)
    {
        const f32 charWidth = 0.014f;   // measured: ~0.151 units for "How To Play" (11 chars) at scale 1.0
        return static_cast<int>(strlen(text)) * charWidth * scale * 0.5f;
    }

    // Dynamic centered print — x shifts based on actual string length
    static void printCentered(f32 y, u32 color,
        const char* text, f32 scale = 1.0f)
    {
        f32 x = -textHalfWidth(text, scale);
        printText(x + 0.01f, y, color, text, scale);
    }

    // -------------------------------------------------------------------------
    // Menu entries
    // -------------------------------------------------------------------------
    static const char* const kMenuItems[] =
    {
        "Play",
        "How To Play",
        "Settings",
        "Tutorial",
        "Exit"
    };

    static const int kMenuItemCount =
        static_cast<int>(sizeof(kMenuItems) / sizeof(kMenuItems[0]));

    static const u32 kColorNormal = 0xFFFFFFFF;
    static const u32 kColorSelected = 0xFF00FF00;

    // -------------------------------------------------------------------------
    // Constructor — load frames + build the full-screen quad mesh
    // -------------------------------------------------------------------------
    // Settings state
    static float gVolume = 1.0f;   // 0.0 – 1.0
    static bool  gMuted = false;

    MainMenu::MainMenu()
        : selectedIndex(0)
        , showHowTo(false)
        , showSettings(false)
    {
        // Load all 6 background frames
        bgFrames[0] = AEGfxTextureLoad("Assets/mmf1.png");
        bgFrames[1] = AEGfxTextureLoad("Assets/mmf2.png");
        bgFrames[2] = AEGfxTextureLoad("Assets/mmf3.png");
        bgFrames[3] = AEGfxTextureLoad("Assets/mmf4.png");
        bgFrames[4] = AEGfxTextureLoad("Assets/mmf5.png");
        bgFrames[5] = AEGfxTextureLoad("Assets/mmf6.png");

        // Build a full-screen textured quad (only needs to be created once)
        if (!bgMesh)
        {
            float minX = AEGfxGetWinMinX();
            float maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY();
            float maxY = AEGfxGetWinMaxY();

            AEGfxMeshStart();

            // Two triangles that cover the whole window
            AEGfxTriAdd(
                minX, maxY, 0xFFFFFFFF, 0.0f, 0.0f,   // top-left
                maxX, maxY, 0xFFFFFFFF, 1.0f, 0.0f,   // top-right
                minX, minY, 0xFFFFFFFF, 0.0f, 1.0f);  // bottom-left

            AEGfxTriAdd(
                maxX, maxY, 0xFFFFFFFF, 1.0f, 0.0f,   // top-right
                maxX, minY, 0xFFFFFFFF, 1.0f, 1.0f,   // bottom-right
                minX, minY, 0xFFFFFFFF, 0.0f, 1.0f);  // bottom-left

            bgMesh = AEGfxMeshEnd();
        }
    }

    // -------------------------------------------------------------------------
    // Fullscreen toggle
    // -------------------------------------------------------------------------
    void ToggleFullscreen()
    {
        gIsFullscreen = !gIsFullscreen;
        AESysSetFullScreen(gIsFullscreen ? 1 : 0);
    }

    // -------------------------------------------------------------------------
    // Update
    // -------------------------------------------------------------------------
    int MainMenu::update()
    {
        float dt = AEFrameRateControllerGetFrameTime();

        // Advance background animation
        backgroundTimer += dt;
        if (backgroundTimer >= backgroundFrameTime)
        {
            backgroundTimer -= backgroundFrameTime;
            backgroundFrame = (backgroundFrame + 1) % backgroundFrameCount;
        }

        // ---- How To Play sub-screen ----
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

        // ---- Settings sub-screen ----
        if (showSettings)
        {
            // Volume UP — right arrow or '='
            if (AEInputCheckTriggered(AEVK_RIGHT))
            {
                gMuted = false;
                gVolume = min(gVolume + 0.1f, 1.0f);
            }
            // Volume DOWN — left arrow or '-'
            if (AEInputCheckTriggered(AEVK_LEFT))
            {
                gVolume = max(gVolume - 0.1f, 0.0f);
                gMuted = (gVolume <= 0.0f);

            }
            // Mute toggle — M key
            if (AEInputCheckTriggered(AEVK_M))
            {
                gMuted = !gMuted;
            }
            // Fullscreen toggle — F key or Enter
            if (AEInputCheckTriggered(AEVK_I))
            {
                ToggleFullscreen();
            }
            // Back — Escape or Space
            if (AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_SPACE))
            {
                showSettings = false;
            }
            return 0;
        }


        // ---- Main menu navigation ----
        if (AEInputCheckTriggered(AEVK_DOWN))
            selectedIndex = (selectedIndex + 1) % kMenuItemCount;

        if (AEInputCheckTriggered(AEVK_UP))
            selectedIndex = (selectedIndex + kMenuItemCount - 1) % kMenuItemCount;

        if (AEInputCheckTriggered(AEVK_RETURN) ||
            AEInputCheckTriggered(AEVK_SPACE))
        {
            switch (selectedIndex)
            {
            case 0: return 1;                        // Play  → winter state
            case 1: showHowTo = true; return 0;   // How To Play
            case 2: showSettings = true; return 0;   // Settings
            case 3: return 4;                        // Tutorial (handle in game loop)
            case 4: return 2;                        // Exit
            }
        }

        if (AEInputCheckTriggered(AEVK_I))
            ToggleFullscreen();

        return 0;
    }

    // -------------------------------------------------------------------------
    // Draw
    // -------------------------------------------------------------------------
    void MainMenu::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);

        // ------------------------------------------------------------------
        // Delete after completing allignment (Uselss for Production)
        // ------------------------------------------------------------------
        //for (f32 cy = -0.95f; cy <= 0.95f; cy += 0.06f)
        //    printText(0.0f, y, 0xFFFF0000u, "|", 0.8f);

        // ------------------------------------------------------------------
        // 1. Draw the animated background
        // ------------------------------------------------------------------
        if (bgMesh && bgFrames[backgroundFrame])
        {
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxSetBlendMode(AE_GFX_BM_NONE);
            AEGfxSetTransparency(1.0f);

            AEGfxTextureSet(bgFrames[backgroundFrame], 0.0f, 0.0f);

            // Identity transform — mesh already covers the full window
            AEMtx33 identityMtx;
            AEMtx33Identity(&identityMtx);
            AEGfxSetTransform(identityMtx.m);
            AEGfxSetTransparency(1.0f);
            AEGfxMeshDraw(bgMesh, AE_GFX_MDM_TRIANGLES);
        }

        // ------------------------------------------------------------------
        // 2. Switch back to colour mode for all text
        // ------------------------------------------------------------------
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        if (showHowTo)
        {
            drawHowToPlay();
            return;
        }

        if (showSettings)
        {
            drawSettings();
            return;
        }

        // Title — large and prominent
        printCentered(0.60f, 0xFFFFFF00u, "FOUR SEASONS PLATFORMER", 1.9f);

        // Subtitle — well below title
        printCentered(0.45f, 0xFF88CCFFu, "- A Journey Through the Seasons -", 0.75f);

        // Separator — large gap below subtitle before menu starts
        printCentered(0.35f, 0xFF446644u, "================================", 0.8f);

        // Menu items — start low, leaving a big gap from the subtitle
        const f32 spacing = 0.13f;
        f32       startY = 0.20f;

        for (int i = 0; i < kMenuItemCount; ++i)
        {
            f32 y = startY - i * spacing;

            if (i == selectedIndex)
            {
                // Compute where the text starts so arrows sit flush beside it
                f32 hw = textHalfWidth(kMenuItems[i], 1.1f);  // half text width
                f32 arrowGap = 0.07f;                               // gap between arrow and text edge

                printCentered(y, 0xFF00FF44u, kMenuItems[i], 1.1f);
                printText(-hw + 0.01f - arrowGap, y, 0xFF00FF44u, ">", 1.1f);
                printText(hw + 0.01f + arrowGap - 0.03f, y, 0xFF00FF44u, "<", 1.1f);
            }
            else
            {
                printCentered(y, 0xFFCCCCCCu, kMenuItems[i], 1.0f);
            }
        }

        // Bottom separator and hint — sit just below last item
        const f32 bottomY = startY - (kMenuItemCount - 1) * spacing - 0.12f;
        printCentered(bottomY, 0xFF446644u, "================================", 0.8f);
        printCentered(bottomY - 0.1f, 0xFF666666u, "UP / DOWN to navigate   ENTER to select", 0.65f);
    }

    // -------------------------------------------------------------------------
    // Draw How-To screen
    // -------------------------------------------------------------------------
    void MainMenu::drawHowToPlay() const
    {
        printCentered(0.7f, 0xFF00FFFFu, "How To Play");
        printCentered(0.3f, 0xFFFFFFFFu, "- Use arrow keys to move.");
        printCentered(0.1f, 0xFFFFFFFFu, "- Reach the end of each stage.");
        printCentered(-0.1f, 0xFFFFFFFFu, "- More mechanics coming soon.");
        printCentered(-0.5f, 0xFFFFFF00u, "Press Enter, Space or ESC to return.");
    }

    // -------------------------------------------------------------------------
    // Draw Settings screen
    // -------------------------------------------------------------------------
    void MainMenu::drawSettings() const
    {
        // Title
        printCentered(0.75f, 0xFF00FFFFu, "Settings", 1.3f);

        // --- Fullscreen ---
        printCentered(0.40f, 0xFFFFFF00u, "Fullscreen");
        printCentered(0.20f, 0xFFFFFFFFu,
            gIsFullscreen ? "[ ON ]  Press I or Enter to toggle"
            : "[ OFF ] Press I or Enter to toggle");

        // --- Volume ---
        printCentered(-0.05f, 0xFFFFFF00u, "Volume");

        // Build a simple ASCII bar: [########  ] 
        char volBar[32];
        int  filled = static_cast<int>(gVolume * 10.0f + 0.5f);   // 0-10 segments
        volBar[0] = '[';
        for (int i = 0; i < 10; ++i)
            volBar[1 + i] = (i < filled) ? '#' : '-';
        volBar[11] = ']';
        volBar[12] = '\0';

        u32 volColor = gMuted ? 0xFF888888u : 0xFFFFFFFFu;
        printCentered(-0.22f, volColor, volBar, 1.0f);
        printCentered(-0.38f, 0xFFAAAAAA, "Left / Right arrows to adjust   M to mute");

        if (gMuted)
            printCentered(-0.52f, 0xFFFF4444u, "MUTED");

        // --- Back ---
        printCentered(-0.75f, 0xFFFFFF00u, "Press ESC or Space to go back");
    }

} // namespace game