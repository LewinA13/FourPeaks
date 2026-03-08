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
        // reusing existing melon spritesheet loading in sprite.cpp
        AEGfxTexture* melonTex = sprite::coin();
        if (!melonTex)
            return;

        // only using frame 0 so no animation
        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
        if (!sprite::getCoinUv(0, u0, v0, u1, v1))
            return;

        // save current gameplay camera position.
        float oldCamX = 0.0f;
        float oldCamY = 0.0f;
        AEGfxGetCamPosition(&oldCamX, &oldCamY);

        // put camera at screen centre so HUD is fixed to the screen.
        AEGfxSetCamPosition(0.0f, 0.0f);
        gfx::drawRectangle({ -99999.0f, -99999.0f }, 0.0f, { 1.0f, 1.0f }, 0x00000000u);

        // -----------------------------
        // Draw melon icon
        // -----------------------------
        const gfx::Vec2 iconPos{ -730.0f, 395.0f };
        const gfx::Vec2 iconSize{ 40.0f,  40.0f };

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE); // tells AE next thing i draw is texture image
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);     // tells AE to blend, basically make sure png works as intended
        gfx::drawSprite(melonTex, iconPos, 0.0f, iconSize, u0, v0, u1, v1);

        // -----------------------------
        // Draw melon count text
        // -----------------------------
        // puts number to the right of the melon icon.
        const std::string text = std::to_string(melonCount);
        printText(-0.86f, 0.86f, 0xFFFFFFFFu, text.c_str(), 1.1f);

        // reset default render state after drawing HUD.
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        // restore original gameplay camera
        AEGfxSetCamPosition(oldCamX, oldCamY);
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
        printText(0.55f, 0.88f, 0xFFFFFFFFu, buffer, 1.0f);

        // restore gameplay camera.
        AEGfxSetCamPosition(oldCamX, oldCamY);
    }
}