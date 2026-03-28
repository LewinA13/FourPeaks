#include "pause.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include "gamestate.hpp"
#include "camera.hpp"
#include <cstdio>
#include "mainmenu.hpp"

extern s8 gFontId;

namespace
{
    static void printText(f32 x, f32 y, u32 argbColor, const char* text, f32 scale = 1.0f)
    {
        const f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
        const f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
        const f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
        const f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;

        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    static float textHalfWidth(const char* text, float scale = 1.0f)
    {
        float w = 0.0f;
        float h = 0.0f;
        AEGfxGetPrintSize(gFontId, text, scale, &w, &h);
        return w * 0.5f;
    }

    static void printCentered(float y, u32 color, const char* text, float scale = 1.0f)
    {
        float x = -textHalfWidth(text, scale);
        printText(x, y, color, text, scale);
    }

    static const u32 kColorNormal = 0xFFFFFFFF;
    static const u32 kColorSelected = 0xFF00FF00;
    static const u32 kOverlayColor = 0x99000000;
    static const u32 kBoxColor = 0xEE1A1A1A;
    static const u32 kBoxOutline = 0xFFFFFFFF;

    static const char* const kPauseItems[] =
    {
        "resume",
        "settings",
        "cheats",
        "fullscreen",
        "main menu",
        "exit"
    };

    static const int kPauseItemCount = 6;

    // Tracks current fullscreen state; toggled alongside the actual window call
    static bool sIsFullscreen = false;

    static void toggleFullscreen()
    {
        sIsFullscreen = !sIsFullscreen;

        AESysSetFullScreen(sIsFullscreen);
    }

    static void drawOutlineBox(const gfx::Vec2& pos, const gfx::Vec2& size)
    {
        // box fill
        gfx::drawRectangle(pos, 0.0f, size, kBoxColor);

        // top
        gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - 3.0f }, 0.0f, { size.x, 6.0f }, kBoxOutline);

        // bottom
        gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + 3.0f }, 0.0f, { size.x, 6.0f }, kBoxOutline);

        // left
        gfx::drawRectangle({ pos.x - size.x * 0.5f + 3.0f, pos.y }, 0.0f, { 6.0f, size.y }, kBoxOutline);

        // right
        gfx::drawRectangle({ pos.x + size.x * 0.5f - 3.0f, pos.y }, 0.0f, { 6.0f, size.y }, kBoxOutline);
    }
}

namespace pause
{
    Action update()
    {
        if (!gGame.pauseActive)
            return Action::None;

        // settings page inside pause
        if (gGame.pauseShowSettings)
        {
            if (AEInputCheckTriggered(AEVK_UP))
                gGame.pauseSettingsRow = (gGame.pauseSettingsRow - 1 + 2) % 2;

            if (AEInputCheckTriggered(AEVK_DOWN))
                gGame.pauseSettingsRow = (gGame.pauseSettingsRow + 1) % 2;

            float& activeVol = (gGame.pauseSettingsRow == 0) ? gGame.musicVol : gGame.sfxVol;

            if (AEInputCheckTriggered(AEVK_RIGHT))
            {
                activeVol += 0.1f;
                if (activeVol > 1.0f) activeVol = 1.0f;
            }

            if (AEInputCheckTriggered(AEVK_LEFT))
            {
                activeVol -= 0.1f;
                if (activeVol < 0.0f) activeVol = 0.0f;
            }

            // esc or space or enter goes back to main pause page
            if (AEInputCheckTriggered(AEVK_ESCAPE) ||
                AEInputCheckTriggered(AEVK_SPACE) ||
                AEInputCheckTriggered(AEVK_RETURN))
            {
                game::SaveVolumeSettings();
                gGame.pauseShowSettings = false;
                gGame.pauseSettingsRow = 0;
            }

            return Action::None;
        }

        // main pause page navigation
        if (AEInputCheckTriggered(AEVK_DOWN))
            gGame.pauseSelectedIndex = (gGame.pauseSelectedIndex + 1) % kPauseItemCount;

        if (AEInputCheckTriggered(AEVK_UP))
            gGame.pauseSelectedIndex = (gGame.pauseSelectedIndex - 1 + kPauseItemCount) % kPauseItemCount;


        if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
        {
            switch (gGame.pauseSelectedIndex)
            {
            case 0:
                return Action::Resume;

            case 1:
                gGame.pauseShowSettings = true;
                gGame.pauseSettingsRow = 0;
                return Action::None;

            case 2:
                // toggle all cheats here
                gGame.cheatsOn = !gGame.cheatsOn;
                if (!gGame.cheatsOn)
                    gGame.noClip = false;
                return Action::None;

            case 3:
                toggleFullscreen();
                return Action::None;

            case 4:
                return Action::MainMenu;

            case 5:
                return Action::ExitGame;

            default:
                break;
            }
        }

        return Action::None;
    }

    void draw()
    {
        if (!gGame.pauseActive)
            return;

        float oldCamX = 0.0f;
        float oldCamY = 0.0f;
        AEGfxGetCamPosition(&oldCamX, &oldCamY);

        // switch everything in pause to ui space
        AEGfxSetCamPosition(0.0f, 0.0f);

        // draw grey overlay across the whole screen
        gfx::drawRectangle(
            { 0.0f, 0.0f },
            0.0f,
            { camera::screenWidth(), camera::screenHeight() },
            0xB0000000
        );

        // draw the pause box in the center of the screen
        drawOutlineBox({ 0.0f, 0.0f }, { 520.0f, 570.0f });

        if (gGame.pauseShowSettings)
        {
            printCentered(0.40f, kColorNormal, "Paused", 1.25f);
            printCentered(0.23f, kColorNormal, "Settings", 1.0f);

            char musicText[64];
            char sfxText[64];
            std::snprintf(musicText, sizeof(musicText), "Music: %.0f%%", gGame.musicVol * 100.0f);
            std::snprintf(sfxText, sizeof(sfxText), "Sfx: %.0f%%", gGame.sfxVol * 100.0f);

            printCentered(0.02f, gGame.pauseSettingsRow == 0 ? kColorSelected : kColorNormal, musicText, 0.95f);
            printCentered(-0.12f, gGame.pauseSettingsRow == 1 ? kColorSelected : kColorNormal, sfxText, 0.95f);

            printCentered(-0.38f, 0xFFBBBBBB, "Left and right to adjust", 0.75f);
            printCentered(-0.50f, 0xFFBBBBBB, "Esc enter or space to go back", 0.75f);
        }
        else
        {
            printCentered(0.40f, kColorNormal, "Paused", 1.25f);

            char cheatText[64];
            std::snprintf(
                cheatText,
                sizeof(cheatText),
                "Cheats : %s",
                gGame.cheatsOn ? "On" : "Off"
            );

            char fullscreenText[64];
            std::snprintf(
                fullscreenText,
                sizeof(fullscreenText),
                "Fullscreen : [%s]",
                sIsFullscreen ? "X" : " "
            );

            const char* displayText[kPauseItemCount] =
            {
                "Resume",
                "Settings",
                cheatText,
                fullscreenText,
                "Main menu",
                "Exit"
            };

            float startY = 0.18f;
            for (int i = 0; i < kPauseItemCount; ++i)
            {
                const u32 color = (i == gGame.pauseSelectedIndex) ? kColorSelected : kColorNormal;
                printCentered(startY - i * 0.14f, color, displayText[i], 0.95f);
            }

            printCentered(-0.72f, 0xFFBBBBBB, "Up and down to move", 0.75f);
            printCentered(-0.84f, 0xFFBBBBBB, "Enter or space to confirm", 0.75f);
        }

        // restore gameplay camera after pause draw
        AEGfxSetCamPosition(oldCamX, oldCamY);
    }
}