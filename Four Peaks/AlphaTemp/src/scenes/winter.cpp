// ----------------------------------------------------------------------------
// Done By: Hong Yang, Arun, Skyler, Justin
// ----------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Winter_s1.cpp - Combined Winter Stages (Stage 1 + Stage 2)
// ---------------------------------------------------------------------------

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

#include "scenes/winter.hpp"
#include "AEEngine.h"
#include "engine/graphics.hpp"
#include "gameplay/player.hpp"
#include "core/gamestate.hpp"
#include "engine/sprite.hpp"
#include "engine/camera.hpp"
#include "core/level_loader.hpp"
#include <sstream>
#include <string>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include "engine/collision.hpp"

typedef uint32_t u32;

extern s8 gFontId;


namespace game {

    // -------------------------------------------------------------------------
    // Marks every particle in the array as inactive so they spawn in gradually
    // during the first few update calls rather than all appearing at once.
    // -------------------------------------------------------------------------
    static void initSnow(std::array<SnowParticle, MAX_SNOW>& particles)
    {
        for (auto& p : particles)
        {
            p.active = false;
        }
    }

    // -------------------------------------------------------------------------
    // Spawns one new snowflake every 0.08 seconds at a random X position along
    // the top of the viewport, then moves all active flakes downward each frame.
    // Flakes that fall below the bottom edge are deactivated for reuse.
    // Flakes that drift off the left or right edge wrap to the opposite side.
    // -------------------------------------------------------------------------
    static void updateSnow(std::array<SnowParticle, MAX_SNOW>& particles,
        float& spawnTimer, float dt)
    {
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        spawnTimer += dt;
        const float spawnInterval = 0.08f;
        if (spawnTimer >= spawnInterval)
        {
            spawnTimer = 0.0f;
            for (auto& p : particles)
            {
                if (!p.active)
                {
                    p.x = minX + (static_cast<float>(rand()) / RAND_MAX) * (maxX - minX);
                    p.y = maxY;
                    p.velX = -20.0f + (static_cast<float>(rand()) / RAND_MAX) * 40.0f;
                    p.velY = -(30.0f + (static_cast<float>(rand()) / RAND_MAX) * 60.0f);
                    p.size = 4.0f + (static_cast<float>(rand()) / RAND_MAX) * 6.0f;
                    p.alpha = 0.4f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
                    p.active = true;
                    break;
                }
            }
        }

        for (auto& p : particles)
        {
            if (!p.active) continue;

            p.x += p.velX * dt;
            p.y += p.velY * dt;

            if (p.x < minX) p.x = maxX;
            if (p.x > maxX) p.x = minX;

            if (p.y < minY)
                p.active = false;
        }
    }

    // -------------------------------------------------------------------------
    // Renders every active snowflake as a small white rectangle with per-particle
    // alpha. Temporarily switches to blended colour mode and restores it after.
    // -------------------------------------------------------------------------
    static void drawSnow(const std::array<SnowParticle, MAX_SNOW>& particles)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        for (const auto& p : particles)
        {
            if (!p.active) continue;
            // Convert the particle's alpha (0.0-1.0) to an 8-bit value for the
            // ARGB colour format, then build a white colour with variable alpha.
            u32 alpha8 = static_cast<u32>(p.alpha * 255.0f) & 0xFF;
            u32 snowColor = (alpha8 << 24) | 0x00FFFFFF;
            gfx::Vec2 pos{ p.x, p.y };
            gfx::Vec2 size{ p.size, p.size };
            gfx::drawRectangle(pos, 0.0f, size, snowColor);
        }
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
    }


    // =========================================================================
    // WinterS1
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 WinterS1::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the tile map from Assets/Levels/winter_s1.txt (32 cols x 20 rows).
    // Clears the map to zero if loading fails.
    // -------------------------------------------------------------------------
    WinterS1::WinterS1()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s1.txt",
            gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }
    }

    // -------------------------------------------------------------------------
    // Default destructor — no dynamic resources to release beyond the tile map.
    // -------------------------------------------------------------------------
    WinterS1::~WinterS1() = default;

    // -------------------------------------------------------------------------
    // Advances stage logic: initialises snow on the first call, toggles the
    // debug grid, updates the player, checks the teleport zone, advances
    // animated tiles, and updates snow particles.
    // Returns 20 to transition to WinterS2, or 0 to remain in this stage.
    // -------------------------------------------------------------------------
    int WinterS1::update(float dt)
    {
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
            gridVisible = !gridVisible;

        if (!camera::isTransitioning())
            PlayerUpdate(gGame.player, dt);

        // Teleport zone: cols 28-29, row 19
        {
            float gx, gy, cw, ch;
            gridToWorld(28, 19, gx, gy, cw, ch);
            float zoneLeft = gx;
            float zoneRight = gx + cw * 2.0f;
            float zoneBot = gy;
            float zoneTop = gy + ch;
            if (gGame.player.pos.x >= zoneLeft && gGame.player.pos.x <= zoneRight &&
                gGame.player.pos.y >= zoneBot && gGame.player.pos.y <= zoneTop)
                return 20;
        }

        sprite::updateAnimatedTiles(dt);
        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the background, all tiles, the optional debug grid, the teleport
    // zone indicator, the player, and the snow particle layer.
    // -------------------------------------------------------------------------
    void WinterS1::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX),         (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        drawTiles();

        if (gridVisible)
            drawGrid();

        if (!camera::isTransitioning())
        {
            int teleportCol = 28;
            int teleportRow = 19;
            for (int c = 0; c < 2; c++)
            {
                int col = teleportCol + c;
                if (col < gridCols)
                {
                    float gridWorldX, gridWorldY, cellW, cellH;
                    gridToWorld(col, teleportRow, gridWorldX, gridWorldY, cellW, cellH);
                    gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };
                    portalPos.x = std::round(portalPos.x);
                    portalPos.y = std::round(portalPos.y);
                    gfx::drawRectangle(portalPos, 0.0f, { cellW, cellH }, 0xAAFFFFFFu);
                }
            }
        }

        PlayerDraw(gGame.player, gridVisible);
        drawSnow(snowParticles);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space top-left position and
    // cell dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void WinterS1::gridToWorld(int col, int row,
        float& xWorld, float& yWorld,
        float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<f32>(gridCols);
        cellH = (maxY - minY) / static_cast<f32>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Spike tiles are offset and scaled to close the visible gap at their bases.
    // Side-facing spikes (21/22/26/27) are drawn with adjusted UV coordinates.
    // Border lines are drawn on the exposed edges of solid tiles only.
    // -------------------------------------------------------------------------
    void WinterS1::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
                int t = tileMap[r][c];
                return (t == 4 || t == 6 || t == 1);
            };

        for (int row = 0; row < gridRows; ++row)
        {
            for (int col = 0; col < gridCols; ++col)
            {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);

                gfx::Vec2 pos{ xWorld + cellW * 0.5f, yWorld + cellH * 0.5f };
                gfx::Vec2 size{ cellW, cellH };
                pos.x = std::round(pos.x);
                pos.y = std::round(pos.y);

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32   borderColor = 0xFF000000;

                // Melon (animated, type 8)
                if (tileType == 8)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(8, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                // Checkpoint (animated, type 10)
                if (tileType == 10)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(10, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                if (tileType == 19)
                {
                    AEGfxTexture* tex = sprite::sign();
                    gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.05f };
                    gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
                    if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
                }

                if (tileType == 12)
                {
                    AEGfxTexture* crackTex = sprite::crack();
                    if (crackTex)
                    {
                        float u0{}, v0{}, u1{}, v1{};
                        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                        gfx::drawSprite(crackTex, pos, 0.0f, size, u0, v0, u1, v1);
                        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                        AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    }
                    continue;
                }

                // Ice tile (type 11 or 1)
                if (tileType == 11 || tileType == 1)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex) gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }

                // Vertical spikes: type 2 points up, type 9 points down.
                // Sunk ~12% into the tile to close the gap at the base.
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };
                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) { v0 = 1.0f; v1 = 0.0f; }
                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Horizontal spikes: type 26 faces right, type 27 faces left.
                else if (tileType == 26 || tileType == 27)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Cell-fit side spikes: type 21 points right, type 22 points left.
                // Drawn with swapped width/height so the portrait sprite lies sideways.
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }
                        else { u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (type 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // Winter corner tile (type 4)
                else if (tileType == 4)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Winter top tile (type 6)
                else if (tileType == 6)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t2 = sprite::winterT();
                    if (t2) gfx::drawSprite(t2, pos, 0.0f, size, 0, 0, 1, 1);
                    else    gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Animated tiles (fire=24, saw=25 handled by drawAnimatedTile)
                else if (sprite::drawAnimatedTile(tileType, pos, size))
                {
                    // handled
                }
                // Remaining tiles: look up UV from the tileset atlas, or fall back to colour
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }

                // ---- Borders (solid tiles only) ----
                if (!isSolid(row, col)) continue;

                if (!isSolid(row + 1, col))
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row - 1, col))
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row, col - 1))
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                if (!isSolid(row, col + 1))
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as thin rectangles aligned to tile cell
    // boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void WinterS1::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;

        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();

        float cellW = (maxX - minX) / static_cast<f32>(gridCols);
        float cellH = (maxY - minY) / static_cast<f32>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            gfx::drawRectangle({ x, (minY + maxY) * 0.5f }, 0.0f, { thickness, maxY - minY }, gridColor);
        }
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::drawRectangle({ (minX + maxX) * 0.5f, y }, 0.0f, { maxX - minX, thickness }, gridColor);
        }
    }


    // =========================================================================
    // WinterS2
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 WinterS2::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the tile map from Assets/Levels/winter_s2.txt (32 cols x 20 rows).
    // Clears the map to zero if loading fails.
    // -------------------------------------------------------------------------
    WinterS2::WinterS2()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s2.txt",
            gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }
    }

    // -------------------------------------------------------------------------
    // Default destructor — no dynamic resources to release beyond the tile map.
    // -------------------------------------------------------------------------
    WinterS2::~WinterS2() = default;

    // -------------------------------------------------------------------------
    // Advances stage logic: initialises snow on the first call, toggles the
    // debug grid, updates the player, checks the teleport zone, advances
    // animated tiles, and updates snow particles.
    // Returns 21 to transition to WinterS3, or 0 to remain in this stage.
    // -------------------------------------------------------------------------
    int WinterS2::update(float dt)
    {
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
            gridVisible = !gridVisible;

        if (!camera::isTransitioning())
            PlayerUpdate(gGame.player, dt);

        // Teleport zone: col 31, rows 18-19
        {
            float gx, gy, cw, ch;
            gridToWorld(31, 18, gx, gy, cw, ch);
            float zoneLeft = gx;
            float zoneRight = gx + cw;
            float zoneBot = gy;
            float zoneTop = gy + ch * 2.0f;
            if (gGame.player.pos.x >= zoneLeft && gGame.player.pos.x <= zoneRight &&
                gGame.player.pos.y >= zoneBot && gGame.player.pos.y <= zoneTop)
                return 21;
        }

        sprite::updateAnimatedTiles(dt);
        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the background, all tiles, the optional debug grid, the teleport
    // zone indicator, the player, and the snow particle layer.
    // -------------------------------------------------------------------------
    void WinterS2::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX),         (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible) drawGrid();

        if (!camera::isTransitioning())
        {
            int teleportCol = 31;
            for (int r = 0; r < 2; r++)
            {
                int row = 18 + r;
                if (row < gridRows)
                {
                    float gridWorldX, gridWorldY, cellW, cellH;
                    gridToWorld(teleportCol, row, gridWorldX, gridWorldY, cellW, cellH);
                    gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };
                    portalPos.x = std::round(portalPos.x);
                    portalPos.y = std::round(portalPos.y);
                    gfx::drawRectangle(portalPos, 0.0f, { cellW, cellH }, 0xAAFFFFFFu);
                }
            }
        }

        PlayerDraw(gGame.player, gridVisible);
        drawSnow(snowParticles);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space top-left position and
    // cell dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void WinterS2::gridToWorld(int col, int row,
        float& xWorld, float& yWorld,
        float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<f32>(gridCols);
        cellH = (maxY - minY) / static_cast<f32>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Spike tiles are offset and scaled to close the visible gap at their bases.
    // Side-facing spikes (21/22/26/27) are drawn with adjusted UV coordinates.
    // Border lines are drawn on the exposed edges of solid tiles only.
    // -------------------------------------------------------------------------
    void WinterS2::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
                int t = tileMap[r][c];
                return (t == 4 || t == 6 || t == 1);
            };

        for (int row = 0; row < gridRows; ++row)
        {
            for (int col = 0; col < gridCols; ++col)
            {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);

                gfx::Vec2 pos{ xWorld + cellW * 0.5f, yWorld + cellH * 0.5f };
                gfx::Vec2 size{ cellW, cellH };
                pos.x = std::round(pos.x);
                pos.y = std::round(pos.y);

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32   borderColor = 0xFF000000;

                // Melon (animated, type 8)
                if (tileType == 8)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(8, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                // Checkpoint (animated, type 10)
                if (tileType == 10)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(10, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                if (tileType == 19)
                {
                    AEGfxTexture* tex = sprite::sign();
                    gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.05f };
                    gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
                    if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
                }

                // Ice tile (type 11 or 1)
                if (tileType == 11 || tileType == 1)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex) gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }

                // Vertical spikes: type 2 points up, type 9 points down.
                // Sunk ~12% into the tile to close the gap at the base.
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };
                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) { v0 = 1.0f; v1 = 0.0f; }
                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Horizontal spikes: type 26 faces right, type 27 faces left.
                else if (tileType == 26 || tileType == 27)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Cell-fit side spikes: type 21 points right, type 22 points left.
                // Drawn with swapped width/height so the portrait sprite lies sideways.
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }
                        else { u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (type 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // Winter corner tile (type 4)
                else if (tileType == 4)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Winter top tile (type 6)
                else if (tileType == 6)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t2 = sprite::winterT();
                    if (t2) gfx::drawSprite(t2, pos, 0.0f, size, 0, 0, 1, 1);
                    else    gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Animated tiles (fire=24, saw=25 handled by drawAnimatedTile)
                else if (sprite::drawAnimatedTile(tileType, pos, size))
                {
                    // handled
                }
                // Remaining tiles: look up UV from the tileset atlas, or fall back to colour
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }

                // ---- Borders (solid tiles only) ----
                if (!isSolid(row, col)) continue;

                if (!isSolid(row + 1, col))
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row - 1, col))
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row, col - 1))
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                if (!isSolid(row, col + 1))
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as thin rectangles aligned to tile cell
    // boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void WinterS2::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;

        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float screenW = (maxX - minX);
        float screenH = (maxY - minY);

        float cellW = screenW / static_cast<float>(gridCols);
        float cellH = screenH / static_cast<float>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            gfx::drawRectangle({ x, (minY + maxY) * 0.5f }, 0.0f, { thickness, screenH }, gridColor);
        }
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::drawRectangle({ (minX + maxX) * 0.5f, y }, 0.0f, { screenW, thickness }, gridColor);
        }
    }


    // =========================================================================
    // WinterS3
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 WinterS3::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the tile map from Assets/Levels/winter_s3.txt (32 cols x 20 rows),
    // then scans it to build the initial iceTiles list for every type-1
    // (breakable ice) cell found. Clears the map to zero if loading fails.
    // -------------------------------------------------------------------------
    WinterS3::WinterS3()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s3.txt",
            gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                if (tileMap[row][col] == 1) {
                    IceTileState ice;
                    ice.row = row;
                    ice.col = col;
                    iceTiles.push_back(ice);
                }
    }

    // -------------------------------------------------------------------------
    // Resets all IceTileState entries to their initial untriggered values and
    // restores each tile in the map to type 1.
    // Call this before transitioning back into the stage from another scene.
    // -------------------------------------------------------------------------
    void WinterS3::resetBreakableIce()
    {
        for (auto& ice : iceTiles) {
            tileMap[ice.row][ice.col] = 1;
            ice.triggered = false;
            ice.timer = 0.0f;
            ice.crackFrame = 0;
            ice.destroyed = false;
        }
    }

    // -------------------------------------------------------------------------
    // Default destructor — no dynamic resources to release beyond iceTiles.
    // -------------------------------------------------------------------------
    WinterS3::~WinterS3() = default;

    // -------------------------------------------------------------------------
    // Advances stage logic: initialises snow on the first call, toggles the
    // debug grid, updates the player, checks the teleport zone, processes ice
    // tile crack timers and tile removal, resets ice on player respawn, advances
    // animated tiles, and updates snow particles.
    // Returns 22 to transition to WinterS4, or 0 to remain in this stage.
    // -------------------------------------------------------------------------
    int WinterS3::update(float dt)
    {
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
            gridVisible = !gridVisible;

        if (!camera::isTransitioning())
            PlayerUpdate(gGame.player, dt);

        // Teleport zone: scan row 19 for type-3 portal tiles; fallback to cols 1-2
        {
            bool triggered = false;
            int  teleportRow = 19;
            for (int c = 0; c < gridCols && !triggered; ++c)
            {
                if (tileMap[teleportRow][c] == 3)
                {
                    float gx, gy, cw, ch;
                    gridToWorld(c, teleportRow, gx, gy, cw, ch);
                    if (gGame.player.pos.x >= gx && gGame.player.pos.x <= gx + cw &&
                        gGame.player.pos.y >= gy && gGame.player.pos.y <= gy + ch)
                        triggered = true;
                }
            }
            if (!triggered)
            {
                float gx, gy, cw, ch;
                gridToWorld(1, teleportRow, gx, gy, cw, ch);
                if (gGame.player.pos.x >= gx && gGame.player.pos.x <= gx + cw * 2.0f &&
                    gGame.player.pos.y >= gy && gGame.player.pos.y <= gy + ch)
                    triggered = true;
            }
            if (triggered) return 22;
        }

        // Consume the collision system's trigger list and mark matching ice tiles
        for (auto& trigger : g_triggeredIceTiles)
            for (auto& ice : iceTiles)
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered)
                    ice.triggered = true;
        g_triggeredIceTiles.clear();

        // Advance crack animation; destroy tile once the final frame is reached
        for (auto& ice : iceTiles) {
            if (ice.triggered && !ice.destroyed) {
                ice.timer += dt;
                int newFrame = static_cast<int>(ice.timer / sprite::crackFrameTime);
                if (newFrame >= sprite::crackFrameCount - 1) {
                    tileMap[ice.row][ice.col] = 0;
                    ice.destroyed = true;
                }
                else {
                    ice.crackFrame = newFrame;
                }
            }
        }

        // Reset all ice states when the player respawns
        if (gGame.player.respawning) {
            for (auto& ice : iceTiles) {
                tileMap[ice.row][ice.col] = 1;
                ice.triggered = false;
                ice.timer = 0.0f;
                ice.crackFrame = 0;
                ice.destroyed = false;
            }
        }

        sprite::updateAnimatedTiles(dt);
        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the background, all tiles (including cracking ice frames), the
    // optional debug grid, the teleport zone indicator, the player, and the
    // snow particle layer.
    // -------------------------------------------------------------------------
    void WinterS3::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX),         (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible) drawGrid();

        // Highlight all type-3 portal tiles in row 19; fall back to cols 1-2
        if (!camera::isTransitioning())
        {
            int  teleportRow = 19;
            bool drewAny = false;

            for (int col = 0; col < gridCols; ++col)
            {
                if (tileMap[teleportRow][col] == 3)
                {
                    float gridWorldX, gridWorldY, cellW, cellH;
                    gridToWorld(col, teleportRow, gridWorldX, gridWorldY, cellW, cellH);
                    gfx::Vec2 portalPos{ std::round(gridWorldX + cellW * 0.5f),
                                         std::round(gridWorldY + cellH * 0.5f) };
                    gfx::drawRectangle(portalPos, 0.0f, { cellW, cellH }, 0xAAFFFFFFu);
                    drewAny = true;
                }
            }

            if (!drewAny)
            {
                for (int c = 0; c < 2; ++c)
                {
                    int col = 1 + c;
                    if (col < gridCols)
                    {
                        float gridWorldX, gridWorldY, cellW, cellH;
                        gridToWorld(col, teleportRow, gridWorldX, gridWorldY, cellW, cellH);
                        gfx::Vec2 portalPos{ std::round(gridWorldX + cellW * 0.5f),
                                             std::round(gridWorldY + cellH * 0.5f) };
                        gfx::drawRectangle(portalPos, 0.0f, { cellW, cellH }, 0xAAFFFFFFu);
                    }
                }
            }
        }

        PlayerDraw(gGame.player, gridVisible);
        drawSnow(snowParticles);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space top-left position and
    // cell dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void WinterS3::gridToWorld(int col, int row,
        float& xWorld, float& yWorld,
        float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Ice tiles (type 1) look up their per-tile crack frame from iceTiles and
    // select the matching UV region from the crack sprite sheet, then skip
    // the border pass since cracking ice is not treated as solid.
    // Spike tiles are offset and scaled to close the visible gap at their bases.
    // Border lines are drawn on the exposed edges of remaining solid tiles.
    // -------------------------------------------------------------------------
    void WinterS3::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
                int t = tileMap[r][c];
                return (t == 4 || t == 6 || t == 1);
            };

        for (int row = 0; row < gridRows; ++row)
        {
            for (int col = 0; col < gridCols; ++col)
            {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);

                gfx::Vec2 pos{ xWorld + cellW * 0.5f, yWorld + cellH * 0.5f };
                gfx::Vec2 size{ cellW, cellH };
                pos.x = std::round(pos.x);
                pos.y = std::round(pos.y);

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32   borderColor = 0xFF000000;

                // Melon (animated, type 8)
                if (tileType == 8)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(8, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                // Checkpoint (animated, type 10)
                if (tileType == 10)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(10, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                if (tileType == 19)
                {
                    AEGfxTexture* tex = sprite::sign();
                    gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.05f };
                    gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
                    if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
                }

                // Non-breakable ice tile (type 11)
                if (tileType == 11)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex) gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }

                // Breakable ice tile (type 1): look up per-tile crack frame
                if (tileType == 1)
                {
                    AEGfxTexture* crackTex = sprite::crack();
                    if (crackTex)
                    {
                        int thisCrackFrame = 0;
                        for (const auto& ice : iceTiles) {
                            if (ice.row == row && ice.col == col) {
                                thisCrackFrame = ice.crackFrame;
                                break;
                            }
                        }
                        float u0{}, v0{}, u1{}, v1{};
                        sprite::getCrackUv(thisCrackFrame, u0, v0, u1, v1);
                        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                        gfx::drawSprite(crackTex, pos, 0.0f, size, u0, v0, u1, v1);
                        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                        AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    }
                    continue;
                }

                // Vertical spikes: type 2 points up, type 9 points down.
                // Sunk ~12% into the tile to close the gap at the base.
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };
                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) { v0 = 1.0f; v1 = 0.0f; }
                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Horizontal spikes: type 26 faces right, type 27 faces left.
                else if (tileType == 26 || tileType == 27)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Cell-fit side spikes: type 21 points right, type 22 points left.
                // Drawn with swapped width/height so the portrait sprite lies sideways.
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }
                        else { u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (type 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // Winter corner tile (type 4)
                else if (tileType == 4)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Winter top tile (type 6)
                else if (tileType == 6)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t2 = sprite::winterT();
                    if (t2) gfx::drawSprite(t2, pos, 0.0f, size, 0, 0, 1, 1);
                    else    gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Animated tiles (fire=24, saw=25 handled by drawAnimatedTile)
                else if (sprite::drawAnimatedTile(tileType, pos, size))
                {
                    // handled
                }
                // Remaining tiles: look up UV from the tileset atlas, or fall back to colour
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }

                // ---- Borders (solid tiles only) ----
                if (!isSolid(row, col)) continue;

                if (!isSolid(row + 1, col))
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row - 1, col))
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row, col - 1))
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                if (!isSolid(row, col + 1))
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as thin rectangles aligned to tile cell
    // boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void WinterS3::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float screenW = (maxX - minX), screenH = (maxY - minY);
        float cellW = screenW / static_cast<float>(gridCols);
        float cellH = screenH / static_cast<float>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            gfx::drawRectangle({ x, (minY + maxY) * 0.5f }, 0.0f, { thickness, screenH }, gridColor);
        }
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::drawRectangle({ (minX + maxX) * 0.5f, y }, 0.0f, { screenW, thickness }, gridColor);
        }
    }


    // =========================================================================
    // WinterS4
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 WinterS4::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the tile map from Assets/Levels/winter_s4.txt (32 cols x 20 rows),
    // then scans it to build the initial iceTiles list for every type-1
    // (breakable ice) cell found. Clears the map to zero if loading fails.
    // -------------------------------------------------------------------------
    WinterS4::WinterS4()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s4.txt",
            gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                if (tileMap[row][col] == 1) {
                    IceTileState ice;
                    ice.row = row;
                    ice.col = col;
                    iceTiles.push_back(ice);
                }
    }

    // -------------------------------------------------------------------------
    // Resets all IceTileState entries to their initial untriggered values and
    // restores each tile in the map to type 1.
    // Call this before transitioning back into the stage from another scene.
    // -------------------------------------------------------------------------
    void WinterS4::resetBreakableIce()
    {
        for (auto& ice : iceTiles) {
            tileMap[ice.row][ice.col] = 1;
            ice.triggered = false;
            ice.timer = 0.0f;
            ice.crackFrame = 0;
            ice.destroyed = false;
        }
    }

    // -------------------------------------------------------------------------
    // Default destructor — no dynamic resources to release beyond iceTiles.
    // -------------------------------------------------------------------------
    WinterS4::~WinterS4() = default;

    // -------------------------------------------------------------------------
    // Advances stage logic: initialises snow on the first call, toggles the
    // debug grid, updates the player, advances animated tiles, processes ice
    // tile crack timers and tile removal, resets ice on player respawn, checks
    // the teleport zone, and updates snow particles.
    // Returns 23 to transition to SummerS1, or 0 to remain in this stage.
    // -------------------------------------------------------------------------
    int WinterS4::update(float dt)
    {
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
            gridVisible = !gridVisible;

        if (!camera::isTransitioning())
            PlayerUpdate(gGame.player, dt);

        sprite::updateAnimatedTiles(dt);

        // Consume the collision system's trigger list and mark matching ice tiles
        for (auto& trigger : g_triggeredIceTiles)
            for (auto& ice : iceTiles)
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered)
                    ice.triggered = true;
        g_triggeredIceTiles.clear();

        // Advance crack animation; destroy tile once the final frame is reached
        for (auto& ice : iceTiles) {
            if (ice.triggered && !ice.destroyed) {
                ice.timer += dt;
                int newFrame = static_cast<int>(ice.timer / sprite::crackFrameTime);
                if (newFrame >= sprite::crackFrameCount - 1) {
                    tileMap[ice.row][ice.col] = 0;
                    ice.destroyed = true;
                }
                else {
                    ice.crackFrame = newFrame;
                }
            }
        }

        // Reset all ice states when the player respawns
        if (gGame.player.respawning) {
            for (auto& ice : iceTiles) {
                tileMap[ice.row][ice.col] = 1;
                ice.triggered = false;
                ice.timer = 0.0f;
                ice.crackFrame = 0;
                ice.destroyed = false;
            }
        }

        // Teleport zone: col 29 to the right screen edge, row 19
        {
            float gx, gy, cw, ch;
            gridToWorld(29, 19, gx, gy, cw, ch);
            float zoneLeft = gx;
            float zoneRight = AEGfxGetWinMaxX();
            float zoneBot = gy;
            float zoneTop = gy + ch;
            if (gGame.player.pos.x >= zoneLeft && gGame.player.pos.x <= zoneRight &&
                gGame.player.pos.y >= zoneBot && gGame.player.pos.y <= zoneTop)
                return 23;
        }

        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the background, all tiles (including cracking ice frames), the
    // optional debug grid, the teleport zone indicator, the player, and the
    // snow particle layer.
    // -------------------------------------------------------------------------
    void WinterS4::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX),         (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible) drawGrid();

        // Teleport indicator: cols 29 onwards, row 19
        if (!camera::isTransitioning())
        {
            for (int c = 0; c < 30; c++)
            {
                int col = 29 + c;
                float gridWorldX, gridWorldY, cellW, cellH;
                gridToWorld(col, 19, gridWorldX, gridWorldY, cellW, cellH);
                gfx::Vec2 portalPos{ std::round(gridWorldX + cellW * 0.5f),
                                     std::round(gridWorldY + cellH * 0.5f) };
                gfx::drawRectangle(portalPos, 0.0f, { cellW, cellH }, 0xAAFFFFFFu);
            }
        }

        PlayerDraw(gGame.player, gridVisible);
        drawSnow(snowParticles);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space top-left position and
    // cell dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void WinterS4::gridToWorld(int col, int row,
        float& xWorld, float& yWorld,
        float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Ice tiles (type 1) look up their per-tile crack frame from iceTiles and
    // select the matching UV region from the crack sprite sheet, then skip
    // the border pass since cracking ice is not treated as solid.
    // The winter artifact (type 34) bobs vertically using a sine wave offset.
    // Spike tiles are offset and scaled to close the visible gap at their bases.
    // Border lines are drawn on the exposed edges of remaining solid tiles.
    // -------------------------------------------------------------------------
    void WinterS4::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
                int t = tileMap[r][c];
                return (t == 4 || t == 6 || t == 1);
            };

        for (int row = 0; row < gridRows; ++row)
        {
            for (int col = 0; col < gridCols; ++col)
            {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);

                gfx::Vec2 pos{ xWorld + cellW * 0.5f, yWorld + cellH * 0.5f };
                gfx::Vec2 size{ cellW, cellH };
                pos.x = std::round(pos.x);
                pos.y = std::round(pos.y);

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32   borderColor = 0xFF000000;

                // Melon (animated, type 8)
                if (tileType == 8)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(8, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                // Checkpoint (animated, type 10)
                if (tileType == 10)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(10, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                if (tileType == 19)
                {
                    AEGfxTexture* tex = sprite::sign();
                    gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.05f };
                    gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
                    if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
                }

                // Breakable ice tile (type 1): look up per-tile crack frame
                if (tileType == 1)
                {
                    AEGfxTexture* crackTex = sprite::crack();
                    if (crackTex)
                    {
                        int thisCrackFrame = 0;
                        for (const auto& ice : iceTiles) {
                            if (ice.row == row && ice.col == col) {
                                thisCrackFrame = ice.crackFrame;
                                break;
                            }
                        }
                        float u0{}, v0{}, u1{}, v1{};
                        sprite::getCrackUv(thisCrackFrame, u0, v0, u1, v1);
                        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                        gfx::drawSprite(crackTex, pos, 0.0f, size, u0, v0, u1, v1);
                        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                        AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    }
                    continue;
                }

                // Non-breakable ice tile (type 11)
                if (tileType == 11)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex) gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }

                // Vertical spikes: type 2 points up, type 9 points down.
                // Sunk ~12% into the tile to close the gap at the base.
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };
                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) { v0 = 1.0f; v1 = 0.0f; }
                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Horizontal spikes: type 26 faces right, type 27 faces left.
                else if (tileType == 26 || tileType == 27)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Cell-fit side spikes: type 21 points right, type 22 points left.
                // Drawn with swapped width/height so the portrait sprite lies sideways.
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }
                        else { u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (type 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // Winter artifact (type 34): bobs vertically using a sine wave offset
                else if (tileType == 34) {
                    AEGfxTexture* winterArtifactsTex = sprite::winterArtifacts();
                    float bobOffset = sinf((float)AEGetTime(nullptr) * 2.0f) * (cellH * 0.08f);
                    gfx::Vec2 artifactsPos{ pos.x, pos.y + bobOffset };
                    gfx::Vec2 artifactsSize{ size.x * 0.9f, size.y * 0.9f };
                    if (winterArtifactsTex) gfx::drawSprite(winterArtifactsTex, artifactsPos, 0.0f, artifactsSize, 0, 0, 1, 1);
                    else                    gfx::drawRectangle(artifactsPos, 0.0f, size, 0xFF00AA00u);
                }
                // Winter corner tile (type 4)
                else if (tileType == 4)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Winter top tile (type 6)
                else if (tileType == 6)
                {
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    AEGfxTexture* t2 = sprite::winterT();
                    if (t2) gfx::drawSprite(t2, pos, 0.0f, size, 0, 0, 1, 1);
                    else    gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                }
                // Animated tiles (fire=24, saw=25 handled by drawAnimatedTile)
                else if (sprite::drawAnimatedTile(tileType, pos, size))
                {
                    // handled
                }
                // Remaining tiles: look up UV from the tileset atlas, or fall back to colour
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }

                // ---- Borders (solid tiles only) ----
                if (!isSolid(row, col)) continue;

                if (!isSolid(row + 1, col))
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row - 1, col))
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(row, col - 1))
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                if (!isSolid(row, col + 1))
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as thin rectangles aligned to tile cell
    // boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void WinterS4::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float screenW = (maxX - minX), screenH = (maxY - minY);
        float cellW = screenW / static_cast<float>(gridCols);
        float cellH = screenH / static_cast<float>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            gfx::drawRectangle({ x, (minY + maxY) * 0.5f }, 0.0f, { thickness, screenH }, gridColor);
        }
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::drawRectangle({ (minX + maxX) * 0.5f, y }, 0.0f, { screenW, thickness }, gridColor);
        }
    }

} // namespace game