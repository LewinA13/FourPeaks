// ---------------------------------------------------------------------------
// thankyou.cpp
// ---------------------------------------------------------------------------
// "Thank You for Playing" end screen shown after Autumn Stage 4.
//
// Features:
//   - Smooth fade-in and fade-out
//   - Animated star particles (simple CPU particles, no extra libs)
//   - Scrolling "credits" text lines
//   - Auto-advances to MainMenu after DISPLAY_DURATION seconds
//   - Skippable with ENTER / SPACE / ESCAPE
// ---------------------------------------------------------------------------

#include "thankyou.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include <cmath>
#include <cstdint>
#include <cstdlib>   // rand
#include <array>

typedef std::uint32_t u32;
extern s8 gFontId;

namespace
{
    // -------------------------------------------------------------------------
    // Minimal text helper (same style used across the project)
    // -------------------------------------------------------------------------
    static void printText(float x, float y, u32 argb, const char* text, float scale = 1.0f)
    {
        float a = ((argb >> 24) & 0xFF) / 255.0f;
        float r = ((argb >> 16) & 0xFF) / 255.0f;
        float g = ((argb >> 8) & 0xFF) / 255.0f;
        float b = ((argb >> 0) & 0xFF) / 255.0f;
        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    // -------------------------------------------------------------------------
    // Simple star particle
    // -------------------------------------------------------------------------
    struct Star
    {
        float x, y;        // normalised screen coords (-1..1)
        float speed;       // drift speed (y per second)
        float size;        // half-extent in normalised units
        float alpha;       // current alpha
        float twinklePhase;
        float twinkleSpeed;
    };

    constexpr int STAR_COUNT = 60;
    std::array<Star, STAR_COUNT> gStars{};
    bool gStarsInitialized = false;

    static void initStars()
    {
        for (auto& s : gStars)
        {
            s.x = ((rand() % 2000) / 1000.0f) - 1.0f;   // -1..1
            s.y = ((rand() % 2000) / 1000.0f) - 1.0f;
            s.speed = 0.02f + (rand() % 100) / 2000.0f;  // 0.02..0.07
            s.size = 0.004f + (rand() % 10) / 3000.0f;
            s.alpha = 0.4f + (rand() % 60) / 100.0f;
            s.twinklePhase = (rand() % 628) / 100.0f;    // 0..2?
            s.twinkleSpeed = 1.5f + (rand() % 300) / 100.0f;
        }
        gStarsInitialized = true;
    }

    static void updateStars(float dt)
    {
        for (auto& s : gStars)
        {
            s.y += s.speed * dt;
            s.twinklePhase += s.twinkleSpeed * dt;
            if (s.y > 1.1f)
            {
                // Wrap back to bottom with a new random x
                s.y = -1.1f;
                s.x = ((rand() % 2000) / 1000.0f) - 1.0f;
            }
        }
    }

    static void drawStars(float globalAlpha)
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float hw = (maxX - minX) * 0.5f;
        float hh = (maxY - minY) * 0.5f;
        float cx = (minX + maxX) * 0.5f;
        float cy = (minY + maxY) * 0.5f;

        for (const auto& s : gStars)
        {
            float twinkle = 0.5f + 0.5f * std::sinf(s.twinklePhase);
            float a = s.alpha * twinkle * globalAlpha;
            u32 col = (static_cast<u32>(a * 255) << 24) | 0x00FFFFCCu;

            float wx = cx + s.x * hw;
            float wy = cy + s.y * hh;
            float sz = s.size * (maxX - minX);

            gfx::drawRectangle({ wx, wy }, 0.0f, { sz, sz }, col);
        }
    }

    // -------------------------------------------------------------------------
    // Credit lines to display
    // -------------------------------------------------------------------------
    struct CreditLine { const char* text; float scale; u32 color; };
    constexpr CreditLine CREDITS[] =
    {
        { "THANK YOU FOR PLAYING",     2.0f, 0xFFFFD700u },  // gold
        { "",                          1.0f, 0x00000000u },
        { "Four Peaks Alpha",          1.4f, 0xFFFFFFFFu },
        { "",                          1.0f, 0x00000000u },
        { "Developed by",              0.9f, 0xFFAAAAAAu },
        { "Your Team Name Here",       1.1f, 0xFF88EEFFu },
        { "",                          1.0f, 0x00000000u },
        { "Press ENTER or SPACE",      0.8f, 0xFFCCCCCCu },
        { "to return to the menu",     0.8f, 0xFFCCCCCCu },
    };
    constexpr int CREDIT_COUNT = static_cast<int>(sizeof(CREDITS) / sizeof(CREDITS[0]));
}

namespace game
{
    // -------------------------------------------------------------------------
    ThankYouScreen::ThankYouScreen()
    {
        timer = 0.0f;

        if (!gStarsInitialized)
            initStars();

        // Optional background image
        bgTex = AEGfxTextureLoad("Assets/thankyou.png");

        // Build a simple full-screen quad for the background tint
        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        AEGfxTriAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        bgMesh = AEGfxMeshEnd();
    }

    ThankYouScreen::~ThankYouScreen()
    {
        if (bgMesh) { AEGfxMeshFree(bgMesh);      bgMesh = nullptr; }
        if (bgTex) { AEGfxTextureUnload(bgTex);  bgTex = nullptr; }
        gStarsInitialized = false; // reset so next visit re-initialises
    }

    // -------------------------------------------------------------------------
    int ThankYouScreen::update(float dt)
    {
        timer += dt;
        updateStars(dt);

        if (AEInputCheckTriggered(AEVK_RETURN) ||
            AEInputCheckTriggered(AEVK_SPACE) ||
            AEInputCheckTriggered(AEVK_ESCAPE))
            return 1;  // ? MainMenu

        if (timer >= DISPLAY_DURATION)
            return 1;

        return 0;
    }

    // -------------------------------------------------------------------------
    void ThankYouScreen::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        // --- Fade alpha -------------------------------------------------------
        float alpha = 1.0f;
        if (timer < FADE_TIME)
            alpha = timer / FADE_TIME;
        else if (timer > DISPLAY_DURATION - FADE_TIME)
            alpha = (DISPLAY_DURATION - timer) / FADE_TIME;
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float scW = maxX - minX;
        float scH = maxY - minY;
        float cx = (minX + maxX) * 0.5f;
        float cy = (minY + maxY) * 0.5f;

        // --- Optional background image ----------------------------------------
        if (bgTex && bgMesh)
        {
            AEMtx33 t;
            AEMtx33Scale(&t, scW, scH);
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
            AEGfxSetTransparency(alpha * 0.5f); // dim so text pops
            AEGfxTextureSet(bgTex, 0.0f, 0.0f);
            AEGfxSetTransform(t.m);
            AEGfxMeshDraw(bgMesh, AE_GFX_MDM_TRIANGLES);
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetBlendMode(AE_GFX_BM_NONE);
            AEGfxSetTransparency(1.0f);
        }

        // --- Dark semi-transparent overlay ------------------------------------
        {
            u32 overlayCol = static_cast<u32>(alpha * 0.85f * 255) << 24;  // black with fade
            gfx::drawRectangle({ cx, cy }, 0.0f, { scW, scH }, overlayCol);
        }

        // --- Stars ------------------------------------------------------------
        drawStars(alpha);

        // --- Horizontal gold divider lines ------------------------------------
        {
            u32 lineCol = (static_cast<u32>(alpha * 200) << 24) | 0x00FFD700u;
            gfx::drawRectangle({ cx, cy + scH * 0.22f }, 0.0f, { scW * 0.7f, 2.0f }, lineCol);
            gfx::drawRectangle({ cx, cy - scH * 0.22f }, 0.0f, { scW * 0.7f, 2.0f }, lineCol);
        }

        // --- Credit text lines -----------------------------------------------
        // Lay out lines centred vertically around middle of screen.
        // Each "unit" in AEGfxPrint normalised coords is roughly 0.08 in Y per scale=1.
        constexpr float LINE_STEP = 0.11f;
        float totalH = (CREDIT_COUNT - 1) * LINE_STEP;
        float startY = totalH * 0.5f;

        for (int i = 0; i < CREDIT_COUNT; ++i)
        {
            const auto& cl = CREDITS[i];
            if (cl.text[0] == '\0') continue;

            float y = startY - i * LINE_STEP;

            // Bake fade into the colour alpha channel
            u32 baseA = (cl.color >> 24) & 0xFF;
            u32 fadedA = static_cast<u32>(baseA * alpha);
            u32 col = (fadedA << 24) | (cl.color & 0x00FFFFFFu);

            // Horizontal centre: measure width using AEGfxGetPrintSize
            float tw = 0.0f, th = 0.0f;
            AEGfxGetPrintSize(gFontId, cl.text, cl.scale, &tw, &th);
            float x = -tw * 0.5f;

            printText(x, y, col, cl.text, cl.scale);
        }
    }

} // namespace game