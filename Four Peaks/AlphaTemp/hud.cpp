#include "hud.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include "sprite.hpp"

#include <string>
#include <cstdint>

typedef uint32_t u32;

// get external font already set
extern s8 gFontId;

namespace hud
{
    namespace
    {
        // Small helper for HUD text.
        static void printText(f32 x, f32 y, u32 argbColor,
            const char* text, f32 scale = 1.0f)
        {
            const f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
            const f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
            const f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
            const f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;

            AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
        }
    }

    void drawMelonCounter(int melonCount)
    {
        AEGfxTexture* melonTex = sprite::coin();
        if (!melonTex)
            return;

        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
        if (!sprite::getCoinUv(0, u0, v0, u1, v1))
            return;

        // Get current camera centre
        float camX = 0.0f;
        float camY = 0.0f;
        AEGfxGetCamPosition(&camX, &camY);

        // HUD offsets relative to camera centre
        const gfx::Vec2 boxPos{ camX - 700.0f, camY + 360.0f };
        const gfx::Vec2 boxSize{ 120.0f, 120.0f };

        const gfx::Vec2 iconPos{ camX - 730.0f, camY + 395.0f };
        const gfx::Vec2 iconSize{ 40.0f,  40.0f };

        // Draw HUD background box
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        gfx::drawRectangle(boxPos, 0.0f, boxSize, 0xAA000000u);

        // Draw melon icon
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        gfx::drawSprite(melonTex, iconPos, 0.0f, iconSize, u0, v0, u1, v1);

        // Draw text in screen space
        const std::string text = std::to_string(melonCount);
        printText(-0.86f, 0.86f, 0xFFFFFFFFu, text.c_str(), 1.1f);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
    }

    void drawDeathCounter(int deathCount)
    {
        // save current gameplay camera position
        float oldCamX = 0.0f;
        float oldCamY = 0.0f;
        AEGfxGetCamPosition(&oldCamX, &oldCamY);
        AEGfxSetCamPosition(0.0f, 0.0f);

        // text slightly below the melon counter.
        const std::string text = "Deaths: " + std::to_string(deathCount);
        printText(-0.925f, 0.78f, 0xFFFFFFFFu, text.c_str(), 1.0f);

        // Restore gameplay camera.
        AEGfxSetCamPosition(oldCamX, oldCamY);
    }

    void drawRunTimer(float totalSeconds)
    {
        // save current gameplay camera position.
        float oldCamX = 0.0f;
        float oldCamY = 0.0f;
        AEGfxGetCamPosition(&oldCamX, &oldCamY);
        AEGfxSetCamPosition(0.0f, 0.0f);

        // clamp negative time just in case.
        if (totalSeconds < 0.0f)
            totalSeconds = 0.0f;

        // convert seconds to MM:SS:CC
        int totalCentiseconds = static_cast<int>(totalSeconds * 100.0f + 0.5f);
        int minutes = totalCentiseconds / 6000;
        int seconds = (totalCentiseconds / 100) % 60;
        int centiseconds = totalCentiseconds % 100;

        char buffer[64];
        sprintf_s(buffer, "Time: %02d:%02d:%02d", minutes, seconds, centiseconds);

        // draw near the top-right area of the screen.
        printText(-0.95f, 0.70f, 0xFFFFFFFFu, buffer, 1.0f);

        // restore gameplay camera.
        AEGfxSetCamPosition(oldCamX, oldCamY);
    }

    void drawArtifactsHud(const bool collected[4])
    {
        float camX = 0.0f;
        float camY = 0.0f;
        AEGfxGetCamPosition(&camX, &camY);

        // small compact box below the timer panel
        const gfx::Vec2 panelSize{ 150.0f, 42.0f };
        const gfx::Vec2 panelPos{ camX - 700.0f, camY + 280.0f };

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        gfx::drawRectangle(panelPos, 0.0f, panelSize, 0xAA000000u);

        AEGfxTexture* textures[4] =
        {
            sprite::winterArtifacts(),
            sprite::summerArtifacts(),
            sprite::springArtifacts(),
            sprite::autumnArtifacts()
        };

        const gfx::Vec2 iconSize{ 22.0f, 22.0f };
        const float startX = panelPos.x - 48.0f;
        const float spacing = 32.0f;
        const float iconY = panelPos.y;

        for (int i = 0; i < 4; ++i)
        {
            const gfx::Vec2 slotPos{ startX + spacing * i, iconY };
            const gfx::Vec2 slotBoxSize{ 26.0f, 26.0f };

            // small slot background
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            gfx::drawRectangle(slotPos, 0.0f, slotBoxSize, 0x44000000u);

            if (!textures[i])
                continue;

            if (collected[i])
            {
                gfx::drawSprite(textures[i], slotPos, 0.0f, iconSize,
                    0.0f, 0.0f, 1.0f, 1.0f);
            }
            else
            {
                gfx::drawSpriteTinted(textures[i], slotPos, 0.0f, iconSize,
                    0.0f, 0.0f, 1.0f, 1.0f, 0xFF000000u);
            }
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
    }
}