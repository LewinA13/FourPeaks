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
#include "gamestate.hpp"
#include <cstdint>
#include <cstring>
#include <fstream>
#include "graphics.hpp"

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
        f32 w = 0.f, h = 0.f;
        AEGfxGetPrintSize(gFontId, text, scale, &w, &h);
        return w * 0.5f;
    }

    // Dynamic centered print — x shifts based on actual string length
    static void printCentered(f32 y, u32 color,
        const char* text, f32 scale = 1.0f)
    {
        f32 x = -textHalfWidth(text, scale);
        printText(x, y, color, text, scale);
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
        "Credit",
        "Exit"
    };

    static const int kMenuItemCount =
        static_cast<int>(sizeof(kMenuItems) / sizeof(kMenuItems[0]));

    static const u32 kColorNormal = 0xFFFFFFFF;
    static const u32 kColorSelected = 0xFF00FF00;

    // -------------------------------------------------------------------------
    // Volume persistence — saves / loads musicVol and sfxVol to a text file
    // -------------------------------------------------------------------------
    static constexpr const char* kSettingsFile = "Assets/settings.txt";

    static void saveVolumeSettings()
    {
        std::ofstream out(kSettingsFile);
        if (out.is_open())
        {
            out << "musicVol " << gGame.musicVol << "\n";
            out << "sfxVol " << gGame.sfxVol << "\n";
        }
    }

    static void loadVolumeSettings()
    {
        std::ifstream in(kSettingsFile);
        if (!in.is_open()) return;   // first run — keep defaults

        char key[32];
        float val;
        while (in >> key >> val)
        {
            if (std::strcmp(key, "musicVol") == 0) gGame.musicVol = val;
            if (std::strcmp(key, "sfxVol") == 0) gGame.sfxVol = val;
        }
    }

    // -------------------------------------------------------------------------
    // Constructor — load frames + build the full-screen quad mesh
    // -------------------------------------------------------------------------
    // Settings state


    MainMenu::MainMenu()
        : selectedIndex(0)
        , showHowTo(false)
        , showSettings(false)
        , settingsRow(0)
    {
        // Load all 6 background frames
        bgFrames[0] = AEGfxTextureLoad("Assets/background_/mmf1.png");
        bgFrames[1] = AEGfxTextureLoad("Assets/background_/mmf2.png");
        bgFrames[2] = AEGfxTextureLoad("Assets/background_/mmf3.png");
        bgFrames[3] = AEGfxTextureLoad("Assets/background_/mmf4.png");
        bgFrames[4] = AEGfxTextureLoad("Assets/background_/mmf5.png");
        bgFrames[5] = AEGfxTextureLoad("Assets/background_/mmf6.png");

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

        // Restore volume settings from last session
        loadVolumeSettings();
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
        float dt = static_cast<float>(AEFrameRateControllerGetFrameTime());

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
            // settingsRow: 0 = Music volume, 1 = SFX volume
            // UP / DOWN — switch between rows
            if (AEInputCheckTriggered(AEVK_UP))
                settingsRow = (settingsRow + 1) % 2;
            if (AEInputCheckTriggered(AEVK_DOWN))
                settingsRow = (settingsRow + 1) % 2;

            // Convenience reference to whichever volume is active
            float& activeVol = (settingsRow == 0) ? gGame.musicVol : gGame.sfxVol;

            // LEFT / RIGHT — adjust volume of selected row
            if (AEInputCheckTriggered(AEVK_RIGHT))
                activeVol = min(activeVol + 0.1f, 1.0f);
            if (AEInputCheckTriggered(AEVK_LEFT))
                activeVol = max(activeVol - 0.1f, 0.0f);

            // M — mute / unmute selected row
            if (AEInputCheckTriggered(AEVK_M))
            {
                static float sMusicMuteSave = 1.0f;
                static float sSfxMuteSave = 1.0f;
                float& muteSave = (settingsRow == 0) ? sMusicMuteSave : sSfxMuteSave;

                if (activeVol > 0.0f) {
                    muteSave = activeVol;
                    activeVol = 0.0f;
                }
                else {
                    activeVol = muteSave;
                }
            }

            // Fullscreen toggle — I key
            if (AEInputCheckTriggered(AEVK_I))
                ToggleFullscreen();

            // Back — Escape or Space
            if (AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_SPACE))
            {
                saveVolumeSettings();   // persist before leaving
                showSettings = false;
                settingsRow = 0;   // reset cursor for next visit
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
            case 0: return 1;                           // Play  →  stage selection
            case 1: showHowTo = true; return 0;         // How To Play
            case 2: showSettings = true; return 0;      // Settings
            case 3: return 4;                           // Tutorial (handle in game loop)
            case 4: return 5;                           // Credits
            case 5: return 2;                           // Exit
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
        //  printText(0.0f, cy, 0xFFFF0000u, "|", 0.8f);

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
        printCentered(0.60f, 0xFFFFFF00u, "FOUR SEASONS PLATFORMER", 3.5f);

        // Subtitle — well below title
        printCentered(0.35f, 0xFF88CCFFu, "- A Journey Through the Seasons -", 1.25f);

        // Separator — large gap below subtitle before menu starts
        printCentered(0.15f, 0xFF446644u, "=============================================", 1.0f);

        // Menu items — start low, leaving a big gap from the subtitle
        const f32 spacing = 0.13f;
        f32       startY = 0.00f;

        for (int i = 0; i < kMenuItemCount; ++i)
        {
            f32 y = startY - i * spacing;

            if (i == selectedIndex)
            {
                // Compute where the text starts so arrows sit flush beside it
                f32 hw = textHalfWidth(kMenuItems[i], 2.0f);  // half text width
                f32 arrowGap = 0.07f;                               // gap between arrow and text edge

                printCentered(y, 0xFF00FF44u, kMenuItems[i], 2.0f);
                printText(-hw + 0.01f - arrowGap, y, 0xFF00FF44u, ">", 2.0f);
                printText(hw + 0.01f + arrowGap - 0.03f, y, 0xFF00FF44u, "<", 2.0f);
            }
            else
            {
                printCentered(y, 0xFFCCCCCCu, kMenuItems[i], 1.0f);
            }
        }

        // Bottom separator and hint — sit just below last item
        const f32 bottomY = startY - (kMenuItemCount - 1) * spacing - 0.12f;
        printCentered(bottomY, 0xFF446644u, "=============================================", 1.0f);
        printCentered(bottomY - 0.1f, 0xFF666666u, "UP / DOWN to navigate   ENTER to select", 0.85f);
    }

    // -------------------------------------------------------------------------
    // Draw How-To screen
    // -------------------------------------------------------------------------
    void MainMenu::drawHowToPlay() const
    {
        // Make sure we are drawing with menu camera centred
        AEGfxSetCamPosition(0.0f, 0.0f);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);

        // Big obvious debug box first
        gfx::drawRectangle(
            { 0.0f, -25.0f },
            0.0f,
            { 600.0f, 500.0f },
            0x88000000
        );

        // Smaller box behind header only
        gfx::drawRectangle(
            { 0.0f, 320.0f },
            0.0f,
            { 600.0f, 110.0f },
            0x88000000
        );



        printCentered(0.7f, 0xFF00FFFFu, "How To Play", 1.5);
        printCentered(0.3f, 0xFFFFFFFFu, "- Use arrow keys to move.", 1.5);
        printCentered(0.1f, 0xFFFFFFFFu, "- Reach the end of each stage.", 1.5);
        printCentered(-0.1f, 0xFFFFFFFFu, "- More mechanics coming soon.", 1.5);
        printCentered(-0.3f, 0xFFFFFF00u, "Press Enter, Space or ESC to return.", 1.5);
    }

    // -------------------------------------------------------------------------
    // Draw Settings screen
    // -------------------------------------------------------------------------
    void MainMenu::drawSettings() const
    {
        printCentered(0.75f, 0xFF00FFFFu, "Settings", 1.3f);

        // ------------------------------------------------------------------
        // Fullscreen row
        // ------------------------------------------------------------------
        printCentered(0.50f, 0xFFFFFF00u, "Fullscreen");
        printCentered(0.35f, 0xFFFFFFFFu,
            gIsFullscreen ? "[ ON ]  Press I to toggle"
            : "[ OFF ] Press I to toggle");

        // ------------------------------------------------------------------
        // Helper lambda — builds the bar string and picks a color
        // ------------------------------------------------------------------
        auto buildBar = [](float vol, char* out) {
            int filled = static_cast<int>(vol * 10.0f + 0.5f);
            out[0] = '[';
            for (int i = 0; i < 10; ++i)
                out[1 + i] = (i < filled) ? '#' : '-';
            out[11] = ']';
            out[12] = '\0';
            };

        // ------------------------------------------------------------------
        // Music volume row  (settingsRow == 0  → highlighted)
        // ------------------------------------------------------------------
        {
            bool   selected = (settingsRow == 0);
            u32    labelCol = selected ? 0xFF00FF44u : 0xFFFFFF00u;
            bool   muted = (gGame.musicVol <= 0.0f);
            u32    barCol = muted ? 0xFF888888u : (selected ? 0xFF00FF44u : 0xFFFFFFFFu);

            printCentered(0.12f, labelCol, selected ? "> Music Volume <" : "  Music Volume  ");

            char bar[16];
            buildBar(gGame.musicVol, bar);
            printCentered(-0.03f, barCol, bar);
            if (muted) printCentered(-0.15f, 0xFFFF4444u, "MUTED");
        }

        // ------------------------------------------------------------------
        // SFX volume row  (settingsRow == 1  → highlighted)
        // ------------------------------------------------------------------
        {
            bool   selected = (settingsRow == 1);
            u32    labelCol = selected ? 0xFF00FF44u : 0xFFFFFF00u;
            bool   muted = (gGame.sfxVol <= 0.0f);
            u32    barCol = muted ? 0xFF888888u : (selected ? 0xFF00FF44u : 0xFFFFFFFFu);

            printCentered(-0.30f, labelCol, selected ? "> SFX Volume <" : "  SFX Volume  ");

            char bar[16];
            buildBar(gGame.sfxVol, bar);
            printCentered(-0.45f, barCol, bar);
            if (muted) printCentered(-0.57f, 0xFFFF4444u, "MUTED");
        }

        // ------------------------------------------------------------------
        // Hint text
        // ------------------------------------------------------------------
        printCentered(-0.68f, 0xFFAAAAAA,
            "UP/DOWN to switch   LEFT/RIGHT to adjust   M to mute");
        printCentered(-0.80f, 0xFFFFFF00u, "Press ESC or Space to go back");
    }

} // namespace game