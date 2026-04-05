// ----------------------------------------------------------------------------
// Done By: Arun
// ----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Copyright (C) 2026 Team Game++ (Four Peaks)
// All rights reserved.
//
// This file is part of the Four Peaks project. All code, design, and original
// assets are the work of LewinA and team members unless otherwise stated.
//
// Audio assets are sourced from Soundly and used under appropriate licensing.
//
// Reproduction, distribution, or modification of this file or its contents,
// in whole or in part, without prior written permission is strictly prohibited.
//
//---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// thankyou.cpp
// ---------------------------------------------------------------------------
// "Thank You for Playing" end screen shown after Autumn Stage 4.
// ---------------------------------------------------------------------------

#include "scenes/thankyou.hpp"
#include "AEEngine.h"
#include "engine/graphics.hpp"
#include "core/gamestate.hpp"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <array>
#include <cstdio>

typedef std::uint32_t u32;
extern s8 gFontId;

namespace
{
    // ===================================================================
    // printText
    // Draws text at (x, y) with given color and scale
    // ===================================================================
    static void printText(float x, float y, u32 argb, const char* text, float scale = 1.0f)
    {
        const float a = ((argb >> 24) & 0xFF) / 255.0f;
        const float r = ((argb >> 16) & 0xFF) / 255.0f;
        const float g = ((argb >> 8) & 0xFF) / 255.0f;
        const float b = ((argb >> 0) & 0xFF) / 255.0f;
        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    // ===================================================================
    // printCentered
    // Draws text centered horizontally at given y with color and scale
    // ===================================================================
    static void printCentered(float y, u32 argb, const char* text, float scale = 1.0f)
    {
        float width = 0.0f;
        float height = 0.0f;
        AEGfxGetPrintSize(gFontId, text, scale, &width, &height);
        printText(-width * 0.5f, y, argb, text, scale);
    }

    // ===================================================================
    // withAlpha
    // Returns color with applied alpha multiplier
    // ===================================================================
    static u32 withAlpha(u32 color, float alpha)
    {
        u32 baseA = (color >> 24) & 0xFF;
        u32 fadedA = static_cast<u32>(baseA * alpha);
        return (fadedA << 24) | (color & 0x00FFFFFFu);
    }

    // ===================================================================
    // formatRunTime
    // Converts a float time in seconds to a formatted string MM:SS:CS
    // ===================================================================
    static void formatRunTime(float totalSeconds, char* buffer, size_t bufferSize)
    {
        if (totalSeconds < 0.0f)
            totalSeconds = 0.0f;

        const int totalCentiseconds = static_cast<int>(totalSeconds * 100.0f + 0.5f);
        const int minutes = totalCentiseconds / 6000;
        const int seconds = (totalCentiseconds / 100) % 60;
        const int centiseconds = totalCentiseconds % 100;

        std::snprintf(buffer, bufferSize, "Final Time: %02d:%02d:%02d", minutes, seconds, centiseconds);
    }

    struct Star
    {
        float x, y;
        float speed;
        float size;
        float alpha;
        float twinklePhase;
        float twinkleSpeed;
    };

    constexpr int STAR_COUNT = 60;
    std::array<Star, STAR_COUNT> gStars{};
    bool gStarsInitialized = false;

    // ===================================================================
    // initStars
    // Initialize positions, speeds, sizes, and twinkle properties of stars
    // ===================================================================
    static void initStars()
    {
        for (std::array<Star, STAR_COUNT>::reference s : gStars)
        {
            s.x = ((std::rand() % 2000) / 1000.0f) - 1.0f;
            s.y = ((std::rand() % 2000) / 1000.0f) - 1.0f;
            s.speed = 0.02f + (std::rand() % 100) / 2000.0f;
            s.size = 0.004f + (std::rand() % 10) / 3000.0f;
            s.alpha = 0.4f + (std::rand() % 60) / 100.0f;
            s.twinklePhase = (std::rand() % 628) / 100.0f;
            s.twinkleSpeed = 1.5f + (std::rand() % 300) / 100.0f;
        }
        gStarsInitialized = true;
    }

    // ===================================================================
    // updateStars
    // Updates star positions and twinkle phases per frame
    // ===================================================================
    static void updateStars(float dt)
    {
        for (std::array<Star, STAR_COUNT>::reference s : gStars)
        {
            s.y += s.speed * dt;
            s.twinklePhase += s.twinkleSpeed * dt;
            if (s.y > 1.1f)
            {
                s.y = -1.1f;
                s.x = ((std::rand() % 2000) / 1000.0f) - 1.0f;
            }
        }
    }

    // ===================================================================
    // drawStars
    // Draws all stars with twinkle effect and global alpha
    // ===================================================================
    static void drawStars(float globalAlpha)
    {
        const float minX = AEGfxGetWinMinX();
        const float maxX = AEGfxGetWinMaxX();
        const float minY = AEGfxGetWinMinY();
        const float maxY = AEGfxGetWinMaxY();
        const float hw = (maxX - minX) * 0.5f;
        const float hh = (maxY - minY) * 0.5f;
        const float cx = (minX + maxX) * 0.5f;
        const float cy = (minY + maxY) * 0.5f;

        for (std::array<Star, STAR_COUNT>::const_reference s : gStars)
        {
            const float twinkle = 0.5f + 0.5f * std::sinf(s.twinklePhase);
            const float a = s.alpha * twinkle * globalAlpha;
            const u32 col = (static_cast<u32>(a * 255) << 24) | 0x00FFFFCCu;

            const float wx = cx + s.x * hw;
            const float wy = cy + s.y * hh;
            const float sz = s.size * (maxX - minX);

            gfx::drawRectangle({ wx, wy }, 0.0f, { sz, sz }, col);
        }
    }
}

namespace game
{
    // ===================================================================
    // ThankYouScreen::Constructor
    // Initializes stars, loads background texture, and creates mesh
    // ===================================================================
    ThankYouScreen::ThankYouScreen()
    {
        timer = 0.0f;

        if (!gStarsInitialized)
            initStars();

        bgTex = AEGfxTextureLoad("Assets/thankyou.png");

        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        AEGfxTriAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        bgMesh = AEGfxMeshEnd();
    }

    // ===================================================================
    // ThankYouScreen::Destructor
    // Frees background mesh and texture; resets star initialization
    // ===================================================================
    ThankYouScreen::~ThankYouScreen()
    {
        if (bgMesh) { AEGfxMeshFree(bgMesh); bgMesh = nullptr; }
        if (bgTex) { AEGfxTextureUnload(bgTex); bgTex = nullptr; }
        gStarsInitialized = false;
    }

    // ===================================================================
    // ThankYouScreen::update
    // Updates timer and stars; returns 1 if user presses key or time exceeded
    // ===================================================================
    int ThankYouScreen::update(float dt)
    {
        timer += dt;
        updateStars(dt);

        if (AEInputCheckTriggered(AEVK_RETURN) ||
            AEInputCheckTriggered(AEVK_SPACE) ||
            AEInputCheckTriggered(AEVK_ESCAPE))
        {
            return 1;
        }

        if (timer >= DISPLAY_DURATION)
            return 1;

        return 0;
    }

    // ===================================================================
    // ThankYouScreen::draw
    // Draws background, overlay, stars, and all text elements
    // ===================================================================
    void ThankYouScreen::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        float alpha = 1.0f;
        if (timer < FADE_TIME)
            alpha = timer / FADE_TIME;
        else if (timer > DISPLAY_DURATION - FADE_TIME)
            alpha = (DISPLAY_DURATION - timer) / FADE_TIME;

        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        const float minX = AEGfxGetWinMinX();
        const float maxX = AEGfxGetWinMaxX();
        const float minY = AEGfxGetWinMinY();
        const float maxY = AEGfxGetWinMaxY();
        const float scW = maxX - minX;
        const float scH = maxY - minY;
        const float cx = (minX + maxX) * 0.5f;
        const float cy = (minY + maxY) * 0.5f;

        if (bgTex && bgMesh)
        {
            AEMtx33 t;
            AEMtx33Scale(&t, scW, scH);
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
            AEGfxSetTransparency(alpha * 0.5f);
            AEGfxTextureSet(bgTex, 0.0f, 0.0f);
            AEGfxSetTransform(t.m);
            AEGfxMeshDraw(bgMesh, AE_GFX_MDM_TRIANGLES);
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetBlendMode(AE_GFX_BM_NONE);
            AEGfxSetTransparency(1.0f);
        }

        {
            const u32 overlayCol = static_cast<u32>(alpha * 0.85f * 255) << 24;
            gfx::drawRectangle({ cx, cy }, 0.0f, { scW, scH }, overlayCol);
        }

        drawStars(alpha);

        // Move the divider lines farther away so they do not hug the text.
        {
            const u32 lineCol = (static_cast<u32>(alpha * 200) << 24) | 0x00FFD700u;
            gfx::drawRectangle({ cx, cy + scH * 0.31f }, 0.0f, { scW * 0.70f, 2.0f }, lineCol);
            gfx::drawRectangle({ cx, cy - scH * 0.34f }, 0.0f, { scW * 0.70f, 2.0f }, lineCol);
        }

        char timeBuffer[64] = {};
        formatRunTime(gGame.runTimeSeconds, timeBuffer, sizeof(timeBuffer));

        printCentered(0.42f, withAlpha(0xFFFFD700u, alpha), "THANK YOU FOR PLAYING", 2.0f);
        printCentered(0.20f, withAlpha(0xFFFFFFFFu, alpha), "Four Peaks", 1.4f);
        printCentered(-0.02f, withAlpha(0xFFAAAAAAu, alpha), "Developed by", 0.9f);
        printCentered(-0.12f, withAlpha(0xFF88EEFFu, alpha), "Team Game++", 1.1f);
        printCentered(-0.27f, withAlpha(0xFFFFFFFFu, alpha), timeBuffer, 0.95f);
        printCentered(-0.50f, withAlpha(0xFFCCCCCCu, alpha), "Press ENTER or SPACE", 0.8f);
        printCentered(-0.60f, withAlpha(0xFFCCCCCCu, alpha), "to return to the menu", 0.8f);
    }
}