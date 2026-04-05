// ---------------------------------------------------------------------------
// mainmenu.cpp
// implementation of mainmenu.hpp functions. This state shows the animated background and menu options, and routes to other states based on player input.
// ---------------------------------------------------------------------------

#include "../scenes/mainmenu.hpp"
#include "AEEngine.h"
#include "AEFrameRateController.h"
#include "../engine/sprite.hpp"
#include "../core/gamestate.hpp"
#include <cstdint>
#include <cstring>
#include <fstream>
#include "../engine/graphics.hpp"
#include "../gameplay/player.hpp"

// Type aliases
extern s8 gFontId;
extern s8 gFontTitle;
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
    constexpr float backgroundFrameTime = 0.09f;   //Adjust according to desired speed


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
        //Gets the width and height of th text string if it were to be printed at scale 1
        AEGfxGetPrintSize(gFontId, text, scale, &w, &h);
        //Obtains the half-width aka middle point of the text string.
        return w * 0.5f;
    }

    // Dynamic centered print — x shifts based on actual string length
// ---------------------------------------------------------------------------
// Print Centered
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    static void printCentered(f32 y, u32 color,
        const char* text, f32 scale = 1.0f)
    {
        //Allows the printing of text to be at the center of screen and adding the negative value shifts the text to the left. Allowing centering fit
        f32 x = -textHalfWidth(text, scale);
        printText(x, y, color, text, scale);
    }

// ---------------------------------------------------------------------------
// Print Title Centered
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    static void printTitleCentered(f32 y, u32 color, const char* text, f32 scale = 1.0f)
    {
        f32 w = 0.f, h = 0.f;
        AEGfxGetPrintSize(gFontTitle, text, scale, &w, &h);
        f32 x = -(w * 0.5f);

        f32 a = ((color >> 24) & 0xFF) / 255.0f;
        f32 r = ((color >> 16) & 0xFF) / 255.0f;
        f32 g = ((color >> 8) & 0xFF) / 255.0f;
        f32 b = ((color >> 0) & 0xFF) / 255.0f;

        AEGfxPrint(gFontTitle, text, x, y, scale, r, g, b, a); // unpack color same as printText
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

// ---------------------------------------------------------------------------
// Save Volume Settings
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void SaveVolumeSettings()
    {
        std::ofstream out(kSettingsFile);
        if (out.is_open())
        {
            out << "musicVol " << gGame.musicVol << "\n";
            out << "sfxVol " << gGame.sfxVol << "\n";
        }
    }

// ---------------------------------------------------------------------------
// Load Volume Settings
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void LoadVolumeSettings()
    {
        std::ifstream in(kSettingsFile);
        if (!in.is_open()) return;

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


// ---------------------------------------------------------------------------
// Main Menu
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    MainMenu::MainMenu()
        : selectedIndex(0)
        , showHowTo(false)
        , showSettings(false)
        , settingsRow(0)
        , confirmDeleteSave(false)
        , deleteSaveResult(false)
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
        LoadVolumeSettings();
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
            // settingsRow:
            // 0 = music volume
            // 1 = sfx volume
            // 2 = delete save file

            if (AEInputCheckTriggered(AEVK_UP))
                settingsRow = (settingsRow + 3 - 1) % 3;

            if (AEInputCheckTriggered(AEVK_DOWN))
                settingsRow = (settingsRow + 1) % 3;

            // fullscreen toggle stays available from settings
            if (AEInputCheckTriggered(AEVK_I))
                ToggleFullscreen();

            // volume rows only
            if (settingsRow == 0 || settingsRow == 1)
            {
                float& activeVol = (settingsRow == 0) ? gGame.musicVol : gGame.sfxVol;

                if (AEInputCheckTriggered(AEVK_RIGHT))
                    activeVol = min(activeVol + 0.1f, 1.0f);

                if (AEInputCheckTriggered(AEVK_LEFT))
                    activeVol = max(activeVol - 0.1f, 0.0f);

                if (AEInputCheckTriggered(AEVK_M))
                {
                    static float sMusicMuteSave = 1.0f;
                    static float sSfxMuteSave = 1.0f;
                    float& muteSave = (settingsRow == 0) ? sMusicMuteSave : sSfxMuteSave;

                    if (activeVol > 0.0f)
                    {
                        muteSave = activeVol;
                        activeVol = 0.0f;
                    }
                    else
                    {
                        activeVol = muteSave;
                    }
                }
            }

            // delete save row
            if (settingsRow == 2)
            {
                if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
                {
                    if (!confirmDeleteSave)
                    {
                        // first press arms the delete
                        confirmDeleteSave = true;
                        deleteSaveResult = false;
                    }
                    else
                    {
                        // second press confirms it
                        PlayerDeleteCheckpoint("checkpoint.txt");
                        PlayerResetProgress(gGame.player);

                        SaveVolumeSettings();

                        confirmDeleteSave = false;
                        deleteSaveResult = true;

                        // stay inside settings after deleting
                        showSettings = true;
                        settingsRow = 2;

                        return 0;
                    }
                }
            }

            // back out of settings
            if (AEInputCheckTriggered(AEVK_ESCAPE))
            {
                SaveVolumeSettings();
                showSettings = false;
                settingsRow = 0;
                confirmDeleteSave = false;
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
        printTitleCentered(0.60f, 0xFFFFFF00u, "FOUR PEAKS", 1.0f);

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
            { 0.0f, -55.0f },
            0.0f,
            { 600.0f, 600.0f },
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
    
        // Movement
        printCentered(0.35f, 0xFFFFFF00u, "-- Movement --", 1.1f);
        printCentered(0.23f, 0xFFFFFFFFu, "W / A / S / D   -   Move", 1.1f);
        printCentered(0.11f, 0xFFFFFFFFu, "SPACE   -   Jump  (hold longer = jump higher)", 1.1f);
        printCentered(-0.01f, 0xFFFFFFFFu, "SHIFT or K   -   Dash", 1.1f);

        // Advanced
        printCentered(-0.17f, 0xFFFFFF00u, "-- Advanced --", 1.1f);
        printCentered(-0.29f, 0xFFFFFFFFu, "L   -   Wall Hang", 1.1f);
        printCentered(-0.41f, 0xFFFFFFFFu, "L + W   -   Wall Climb Up", 1.1f);
        printCentered(-0.53f, 0xFFFFFFFFu, "L + S   -   Slide Down", 1.1f);
        printCentered(-0.65f, 0xFFFFFFFFu, "SPACE   -   Wall Jump", 1.1f);

    }

    // -------------------------------------------------------------------------
    // Draw Settings screen
    // -------------------------------------------------------------------------
    void MainMenu::drawSettings() const
    {
        printCentered(0.55f, 0xFF00FFFFu, "Settings", 1.3f);

        // ------------------------------------------------------------------
        // Fullscreen row
        // ------------------------------------------------------------------
        printCentered(0.40f, 0xFFFFFF00u, "Fullscreen");
        printCentered(0.33f, 0xFFFFFFFFu,
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

            printCentered(0.15f, labelCol, selected ? "> Music Volume <" : "  Music Volume  ");

            char bar[16];
            buildBar(gGame.musicVol, bar);
            printCentered(0.07f, barCol, bar);
            if (muted) printCentered(0.04f, 0xFFFF4444u, "MUTED");
        }

        // ------------------------------------------------------------------
        // SFX volume row  (settingsRow == 1  → highlighted)
        // ------------------------------------------------------------------
        {
            bool   selected = (settingsRow == 1);
            u32    labelCol = selected ? 0xFF00FF44u : 0xFFFFFF00u;
            bool   muted = (gGame.sfxVol <= 0.0f);
            u32    barCol = muted ? 0xFF888888u : (selected ? 0xFF00FF44u : 0xFFFFFFFFu);

            printCentered(-0.1f, labelCol, selected ? "> SFX Volume <" : "  SFX Volume  ");

            char bar[16];
            buildBar(gGame.sfxVol, bar);
            printCentered(-0.15f, barCol, bar);
            if (muted) printCentered(-0.18f, 0xFFFF4444u, "MUTED");
        }

        // ------------------------------------------------------------------
        // Delete save row
        // ------------------------------------------------------------------
        {
            bool selected = (settingsRow == 2);
            u32 labelCol = selected ? 0xFFFF4444u : 0xFFFFFF00u;

            printCentered(-0.3f, labelCol,
                selected ? "> Delete Save File <" : "  Delete Save File  ");

            if (confirmDeleteSave && selected)
            {
                printCentered(-0.40f, 0xFFFF6666u, "Press Enter again to confirm");
                printCentered(-0.50f, 0xFFAAAAAAu, "This will restart all progress");
            }
            else
            {
                printCentered(-0.40f, 0xFFAAAAAAu, "Enter to delete all checkpoint progress");
            }
        }

        // ------------------------------------------------------------------
        // Hint text
        // ------------------------------------------------------------------
        printCentered(-0.7f, 0xFFAAAAAA,
            "UP/DOWN to switch   LEFT/RIGHT to adjust   M to mute");
        printCentered(-0.75f, 0xFFFFFF00u, "Press ESC to go back");
    }

} // namespace game
