// ---------------------------------------------------------------------------
// Winter_s1.cpp - Combined Winter Stages (Stage 1 + Stage 2)
// ---------------------------------------------------------------------------

#include "winter.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include "gamestate.hpp"
#include "sprite.hpp"
#include "camera.hpp"
#include "level_loader.hpp"
#include <sstream>
#include <string>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include "collision.hpp"

typedef uint32_t u32;

extern s8 gFontId;


namespace game {

    // -------------------------------------------------------------------
    // Helper function - shared by both stages
    // -------------------------------------------------------------------
    static void printText(f32 x, f32 y, u32 argbColor,
        const char* text, f32 scale = 1.0f)
    {
        f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
        f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
        f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
        f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;
        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    // -------------------------------------------------------------------
    // Snow particle helpers - shared by all winter stages
    // -------------------------------------------------------------------
    static void initSnow(std::array<SnowParticle, MAX_SNOW>& particles)
    {
        for (auto& p : particles)
        {
            p.active = false;  // all start inactive — they spawn in gradually
        }
    }

    static void updateSnow(std::array<SnowParticle, MAX_SNOW>& particles, float& spawnTimer, float dt)
    {
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        // Spawn a new flake every 0.08 seconds
        spawnTimer += dt;
        const float spawnInterval = 0.08f;
        if (spawnTimer >= spawnInterval)
        {
            spawnTimer = 0.0f;
            // Find the first inactive particle and activate it
            for (auto& p : particles)
            {
                if (!p.active)
                {
                    p.x = minX + (static_cast<float>(rand()) / RAND_MAX) * (maxX - minX);
                    p.y = maxY; // spawn at the top
                    p.velX = -20.0f + (static_cast<float>(rand()) / RAND_MAX) * 40.0f;
                    p.velY = -(30.0f + (static_cast<float>(rand()) / RAND_MAX) * 60.0f);
                    p.size = 4.0f + (static_cast<float>(rand()) / RAND_MAX) * 6.0f;
                    p.alpha = 0.4f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
                    p.active = true;
                    break;
                }
            }
        }

        // Move active particles; recycle ones that fall off screen
        for (auto& p : particles)
        {
            if (!p.active) continue;

            p.x += p.velX * dt;
            p.y += p.velY * dt;

            // Wrap horizontally
            if (p.x < minX) p.x = maxX;
            if (p.x > maxX) p.x = minX;

            // When fallen off the bottom, deactivate so it can be respawned at the top
            if (p.y < minY)
            {
                p.active = false;
            }
        }
    }

    static void drawSnow(const std::array<SnowParticle, MAX_SNOW>& particles)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        for (const auto& p : particles)
        {
            if (!p.active) continue;
			//This is to convert the particle's alpha (0.0 to 1.0) into an 8-bit integer (0 to 255) for the ARGB color format.
            u32 alpha8 = static_cast<u32>(p.alpha * 255.0f) & 0xFF;
			//This make the snow white with the variable alpha for transparency, by placing the alpha in the highest byte and setting RGB to 255.
            u32 snowColor = (alpha8 << 24) | 0x00FFFFFF; // white with variable alpha
            gfx::Vec2 pos{ p.x, p.y };
            gfx::Vec2 size{ p.size, p.size };
            gfx::drawRectangle(pos, 0.0f, size, snowColor);
        }
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
    }


    // ===================================================================
    // WINTER STAGE 1 IMPLEMENTATION
    // ===================================================================

    u32 WinterS1::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u; // Spikes: red
        case 3: return 0xFF808080u; // Wall: gray
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------
    // WinterS1 Constructor
    // -------------------------------------------------------------------
    WinterS1::WinterS1()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        // LEVEL DESIGN: 0 = empty, 1 = solid block
        // 32 columns wide, 20 rows tall
        // Bottom row = Row 0, Top row = Row 19
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s1.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // (Check that Assets/Levels/winter_s1.txt exists and has 20 rows x 32 cols.)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

    }

    WinterS1::~WinterS1() = default;

    // -------------------------------------------------------------------
    // WinterS1 update
    // -------------------------------------------------------------------
    int WinterS1::update(float dt)
    {
        // Initialize snow on first update (window is guaranteed ready here)
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }

        // DEBUG: Press UP or W key anywhere to start transition
        if (AEInputCheckTriggered(AEVK_UP))
        {
            if (!camera::isTransitioning())
            {
                float fromY = 0.0f;
                float toY = camera::screenHeight();
                camera::startTransitionY(fromY, toY, 0.3f);
                return 20; // Signal transition to main
            }
        }

        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        // Check if player reached the teleport zone
        if (!camera::isTransitioning())
        {
            // Define teleport zone in grid coordinates
            int teleportCol = 28;
            int teleportRow = 19;

            // Convert grid position to world coordinates
            float gridWorldX, gridWorldY, cellW, cellH;
            gridToWorld(teleportCol, teleportRow, gridWorldX, gridWorldY, cellW, cellH);

            // Check if player is within the teleport zone
            float teleportCenterX = gridWorldX + cellW * 0.5f;
            float teleportCenterY = gridWorldY + cellH * 0.5f;

            // Distance check (within 1.5 cells)
            float dx = gGame.player.pos.x - teleportCenterX;
            float dy = gGame.player.pos.y - teleportCenterY;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < cellW * 1.5f)
            {
                return 20; // Signal INSTANT teleport to Stage 2
            }
        }

        sprite::updateAnimatedTiles(dt);

        // Update snow particles
        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }

    // -------------------------------------------------------------------
    // WinterS1 draw
    // -------------------------------------------------------------------
    void WinterS1::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        // ---- DRAW BACKGROUND ----
        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX();
            float maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY();
            float maxY = AEGfxGetWinMaxY();

            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX), (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        // ---- DRAW LEVEL ----
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        drawTiles();

        if (gridVisible)
            drawGrid();

        //printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Winter Stage 1 - 32x20 Grid");
        //printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        //printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        // Draw teleport indicator (2x1 cells - 2 columns, 1 row)
        // Draw teleport indicator (2x1 cells)
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

                    gfx::Vec2 portalSize{ cellW, cellH };
                    gfx::drawRectangle(portalPos, 0.0f, portalSize, 0xAA00FFFF); // Cyan
                }
            }
        }

        PlayerDraw(gGame.player);

        // ---- DRAW SNOW (on top of everything) ----
        drawSnow(snowParticles);
    }

    // -------------------------------------------------------------------
    // WinterS1 gridToWorld
    // -------------------------------------------------------------------
    void WinterS1::gridToWorld(int col, int row,
        float& xWorld, float& yWorld,
        float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        cellW = (maxX - minX) / static_cast<f32>(gridCols);
        cellH = (maxY - minY) / static_cast<f32>(gridRows);

        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------
    // WinterS1 drawTiles
    // -------------------------------------------------------------------
    void WinterS1::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols)
                    return false;

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
                u32 borderColor = 0xFF000000;

                // -------------------------
                // 1. Draw Tile First
                // -------------------------

                // Melon (animated, type 8)
                if (tileType == 8)
                {

                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(8, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue; // Not solid, skip border drawing
                }

                // Checkpoint (animated, type 10)
                if (tileType == 10)
                {

                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    sprite::drawAnimatedTile(10, pos, size);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue; // Not solid, skip border drawing
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
                    continue; // Not solid, skip border drawing
                }

                // Ice tile
                if (tileType == 11 || tileType == 1)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex)
                        gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }
                // Spikes tile
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            // UP-facing: anchor base to cell bottom, sink slightly to close gap
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            // DOWN-facing: anchor to cell top, sink slightly to close gap
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }

                        float u0 = 0.0f;
                        float v0 = 0.0f;
                        float u1 = 1.0f;
                        float v1 = 1.0f;

                        if (tileType == 9)
                        {
                            v0 = 1.0f;
                            v1 = 0.0f;
                        }

                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Left-facing spike (26) and right-facing spike (27)
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
                // Cell-fit left spike (21) and right spike (22)
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        // Draw sideways: swap width/height so portrait spike lies on side
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }       // left: spike points right
                        else { u0 = 1.0f; u1 = 0.0f; }                       // right: mirrored, points left
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (ID 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // WinterC (ID 4) and WinterT (ID 6) standalone textures
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
                // Normal tiles
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                    {
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    }
                    else
                    {
                        u32 tileColor = getTileColor(tileType);
                        gfx::drawRectangle(pos, 0.0f, size, tileColor);
                    }
                }

                // -------------------------
                // 2. Draw Borders LAST (Only Solid Tiles)
                // -------------------------

                if (!isSolid(row, col))
                    continue;

                // TOP
                if (!isSolid(row + 1, col))
                {
                    gfx::Vec2 topPos{ pos.x, pos.y + size.y * 0.5f - border * 0.5f };
                    gfx::Vec2 topSize{ size.x, border };
                    gfx::drawRectangle(topPos, 0.0f, topSize, borderColor);
                }

                // BOTTOM
                if (!isSolid(row - 1, col))
                {
                    gfx::Vec2 bottomPos{ pos.x, pos.y - size.y * 0.5f + border * 0.5f };
                    gfx::Vec2 bottomSize{ size.x, border };
                    gfx::drawRectangle(bottomPos, 0.0f, bottomSize, borderColor);
                }

                // LEFT
                if (!isSolid(row, col - 1))
                {
                    gfx::Vec2 leftPos{ pos.x - size.x * 0.5f + border * 0.5f, pos.y };
                    gfx::Vec2 leftSize{ border, size.y };
                    gfx::drawRectangle(leftPos, 0.0f, leftSize, borderColor);
                }

                // RIGHT
                if (!isSolid(row, col + 1))
                {
                    gfx::Vec2 rightPos{ pos.x + size.x * 0.5f - border * 0.5f, pos.y };
                    gfx::Vec2 rightSize{ border, size.y };
                    gfx::drawRectangle(rightPos, 0.0f, rightSize, borderColor);
                }
            }
        }
    }

    // -------------------------------------------------------------------
    // WinterS1 drawGrid
    // -------------------------------------------------------------------
    void WinterS1::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;

        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        float cellW = (maxX - minX) / static_cast<f32>(gridCols);
        float cellH = (maxY - minY) / static_cast<f32>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        // Vertical lines
        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            gfx::Vec2 pos{ x, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ thickness, maxY - minY };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }

        // Horizontal lines
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::Vec2 pos{ (minX + maxX) * 0.5f, y };
            gfx::Vec2 size{ maxX - minX, thickness };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }
    }

    // ===================================================================
    // WINTER STAGE 2 IMPLEMENTATION
    // ===================================================================

    u32 WinterS2::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u; // Spikes: red
        case 3: return 0xFF808080u; // Wall: gray
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------
    // WinterS2 Constructor
    // -------------------------------------------------------------------
    WinterS2::WinterS2()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        // LEVEL DESIGN: 0 = empty, 1 = solid block
        // 32 columns wide, 20 rows tall
        // Bottom row = Row 0, Top row = Row 19
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s2.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // (Check that Assets/Levels/winter_s2.txt exists and has 20 rows x 32 cols.)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

    }

    WinterS2::~WinterS2() = default;

    // -------------------------------------------------------------------
    // WinterS2 update
    // -------------------------------------------------------------------
    int WinterS2::update(float dt)
    {
        // Initialize snow on first update (window is guaranteed ready here)
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }


        // Only update player when NOT transitioning
        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        // Check if player reached the teleport zone to Stage 3
        if (!camera::isTransitioning())
        {
            // Define teleport zone in grid coordinates (31,19 and 31,18 - 2 vertical cells)
            int teleportCol = 31;
            int teleportRow1 = 19;  // Top cell
            int teleportRow2 = 18;  // Bottom cell

            // Convert grid position to world coordinates
            float gridWorldX, gridWorldY, cellW, cellH;
            gridToWorld(teleportCol, teleportRow2, gridWorldX, gridWorldY, cellW, cellH);

            // Check if player is within the teleport zone (2 cells tall)
            float teleportCenterX = gridWorldX + cellW * 0.5f;
            float teleportCenterY = gridWorldY + cellH * 1.0f; // Center between 2 cells

            // Distance check (within 1.5 cells)
            float dx = gGame.player.pos.x - teleportCenterX;
            float dy = gGame.player.pos.y - teleportCenterY;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < cellW * 1.5f)
            {
                return 21; // Signal teleport to Stage 3
            }
        }

        sprite::updateAnimatedTiles(dt);

        // Update snow particles
        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }


    // -------------------------------------------------------------------
    // WinterS2 draw
    // -------------------------------------------------------------------
    void WinterS2::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        // ---- DRAW BACKGROUND ----
        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX();
            float maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY();
            float maxY = AEGfxGetWinMaxY();
            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX), (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        // ---- DRAW LEVEL ----
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible)
            drawGrid();

        //printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Winter Stage 2 - 32x20 Grid");
        //printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        //printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        // Draw teleport indicator (1x2 cells at column 31, rows 18-19)
        if (!camera::isTransitioning())
        {
            int teleportCol = 31;
            for (int r = 0; r < 2; r++)
            {
                int row = 18 + r; // Rows 18 and 19
                if (row < gridRows)
                {
                    float gridWorldX, gridWorldY, cellW, cellH;
                    gridToWorld(teleportCol, row, gridWorldX, gridWorldY, cellW, cellH);
                    gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };
                    portalPos.x = std::round(portalPos.x);
                    portalPos.y = std::round(portalPos.y);
                    gfx::Vec2 portalSize{ cellW, cellH };
                    gfx::drawRectangle(portalPos, 0.0f, portalSize, 0xAA00FFFF); // Green portal
                }
            }
        }

        PlayerDraw(gGame.player);

        // ---- DRAW SNOW (on top of everything) ----
        drawSnow(snowParticles);
    }


    // -------------------------------------------------------------------
    // WinterS2 gridToWorld
    // -------------------------------------------------------------------
    void WinterS2::gridToWorld(int col, int row,
        float& xWorld, float& yWorld,
        float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        cellW = (maxX - minX) / static_cast<f32>(gridCols);
        cellH = (maxY - minY) / static_cast<f32>(gridRows);

        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------
    // WinterS2 drawTiles
    // -------------------------------------------------------------------
    void WinterS2::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols)
                    return false;

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
                u32 borderColor = 0xFF000000;

                // -------------------------
                // 1. Draw Tile First
                // -------------------------

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

                // Ice tile
                if (tileType == 11 || tileType == 1)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex)
                        gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }
                // Spikes tile
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            // UP-facing: anchor base to cell bottom, sink slightly to close gap
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            // DOWN-facing: anchor to cell top, sink slightly to close gap
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }

                        float u0 = 0.0f;
                        float v0 = 0.0f;
                        float u1 = 1.0f;
                        float v1 = 1.0f;

                        if (tileType == 9)
                        {
                            v0 = 1.0f;
                            v1 = 0.0f;
                        }

                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Left-facing spike (26) and right-facing spike (27)
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
                // Cell-fit left spike (21) and right spike (22)
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        // Draw sideways: swap width/height so portrait spike lies on side
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }       // left: spike points right
                        else { u0 = 1.0f; u1 = 0.0f; }                       // right: mirrored, points left
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (ID 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // WinterC (ID 4) and WinterT (ID 6) standalone textures
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
                // Normal tiles
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                    {
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    }
                    else
                    {
                        u32 tileColor = getTileColor(tileType);
                        gfx::drawRectangle(pos, 0.0f, size, tileColor);
                    }
                }

                // -------------------------
                // 2. Draw Borders LAST (Only Solid Tiles)
                // -------------------------

                if (!isSolid(row, col))
                    continue;

                // TOP
                if (!isSolid(row + 1, col))
                {
                    gfx::Vec2 topPos{ pos.x, pos.y + size.y * 0.5f - border * 0.5f };
                    gfx::Vec2 topSize{ size.x, border };
                    gfx::drawRectangle(topPos, 0.0f, topSize, borderColor);
                }

                // BOTTOM
                if (!isSolid(row - 1, col))
                {
                    gfx::Vec2 bottomPos{ pos.x, pos.y - size.y * 0.5f + border * 0.5f };
                    gfx::Vec2 bottomSize{ size.x, border };
                    gfx::drawRectangle(bottomPos, 0.0f, bottomSize, borderColor);
                }

                // LEFT
                if (!isSolid(row, col - 1))
                {
                    gfx::Vec2 leftPos{ pos.x - size.x * 0.5f + border * 0.5f, pos.y };
                    gfx::Vec2 leftSize{ border, size.y };
                    gfx::drawRectangle(leftPos, 0.0f, leftSize, borderColor);
                }

                // RIGHT
                if (!isSolid(row, col + 1))
                {
                    gfx::Vec2 rightPos{ pos.x + size.x * 0.5f - border * 0.5f, pos.y };
                    gfx::Vec2 rightSize{ border, size.y };
                    gfx::drawRectangle(rightPos, 0.0f, rightSize, borderColor);
                }
            }
        }
    }

    // -------------------------------------------------------------------
    // WinterS2 drawGrid
    // -------------------------------------------------------------------
    void WinterS2::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;

        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        float screenW = (maxX - minX);
        float screenH = (maxY - minY);

        float cellW = screenW / static_cast<float>(gridCols);
        float cellH = screenH / static_cast<float>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        // Vertical lines
        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            float centerY = (minY + maxY) * 0.5f;
            gfx::Vec2 pos{ x, centerY };
            gfx::Vec2 size{ thickness, screenH };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }

        // Horizontal lines
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::Vec2 pos{ (minX + maxX) * 0.5f, y };
            gfx::Vec2 size{ screenW, thickness };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }
    }

    // ===================================================================
    // WINTER STAGE 3 IMPLEMENTATION
    // ===================================================================

    u32 WinterS3::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u;
        case 3: return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    WinterS3::WinterS3()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s3.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // (Check that Assets/Levels/winter_s3.txt exists and has 20 rows x 32 cols.)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col) {
                //! all breaking ice will be store in vector "iceTiles" and type will be struct "IceTileState"
                if (tileMap[row][col] == 1) {
                    IceTileState ice;
                    ice.row = row;
                    ice.col = col;
                    iceTiles.push_back(ice);
                }
            }
    }

    WinterS3::~WinterS3() = default;

    // -------------------------------------------------------------------
    // WinterS3 update
    // -------------------------------------------------------------------
    int WinterS3::update(float dt)
    {
        // Initialize snow on first update (window is guaranteed ready here)
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }


        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        // Check if player reached the teleport zone to Stage 4.
        // Scan every cell in the top row (row 19) that is marked as a portal tile (type 3),
        // then also fall back to the hardcoded col 1-2 zone so the portal works even if
        // the level file uses a different tile type for the exit.
        if (!camera::isTransitioning())
        {
            int teleportRow = 19;

            float cellW0, cellH0, dummy;
            float dummyX, dummyY;
            gridToWorld(0, teleportRow, dummyX, dummyY, cellW0, cellH0);

            bool triggered = false;

            // Pass 1: scan the whole top row for portal tiles (tile type 3)
            for (int c = 0; c < gridCols && !triggered; ++c)
            {
                if (tileMap[teleportRow][c] == 3)
                {
                    float gx, gy, cw, ch;
                    gridToWorld(c, teleportRow, gx, gy, cw, ch);
                    float cx = gx + cw * 0.5f;
                    float cy = gy + ch * 0.5f;
                    float dx = gGame.player.pos.x - cx;
                    float dy = gGame.player.pos.y - cy;
                    if ((dx * dx + dy * dy) < (cw * 2.0f) * (cw * 2.0f))
                        triggered = true;
                }
            }

            // Pass 2: fallback hardcoded zone (cols 1-2, row 19) with generous radius
            if (!triggered)
            {
                float gx, gy, cw, ch;
                gridToWorld(1, teleportRow, gx, gy, cw, ch);
                float cx = gx + cw * 1.0f; // midpoint between col 1 and col 2
                float cy = gy + ch * 0.5f;
                float dx = gGame.player.pos.x - cx;
                float dy = gGame.player.pos.y - cy;
                if ((dx * dx + dy * dy) < (cw * 2.0f) * (cw * 2.0f))
                    triggered = true;
            }

            if (triggered)
                return 22; // Signal teleport to Stage 4
        }


        for (auto& trigger : g_triggeredIceTiles) {
            for (auto& ice : iceTiles) {
                //! check the ice pos whether equal and never set state before
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered) {
                    ice.triggered = true;
                }
            }
        }

        //! clear the list to save the memory
        g_triggeredIceTiles.clear();

        //! check only if breakingIce is triggered but havent being destroyed
        for (auto& ice : iceTiles) {
            if (ice.triggered && !ice.destroyed) {
                ice.timer += dt;
                //! update the frame
                int newFrame = static_cast<int>(ice.timer / sprite::crackFrameTime);
                //! if reach end of the craking frame, then set to 0(destroyed) and set the state
                if (newFrame >= sprite::crackFrameCount - 1) {
                    tileMap[ice.row][ice.col] = 0;
                    ice.destroyed = true;
                }
                else {
                    ice.crackFrame = newFrame;
                }
            }
        }


        //! if player dead and respawning, reset all ice states 
        if (gGame.player.respawning) {
            for (auto& ice : iceTiles) {
                tileMap[ice.row][ice.col] = 1;
                ice.triggered = false;
                ice.timer = 0.0f;
                ice.crackFrame = 0;
                ice.destroyed = false;
            }
        }

        // Update animated tiles (melon, checkpoint, fire, saw)
        sprite::updateAnimatedTiles(dt);

        // Update snow particles
        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }

    // -------------------------------------------------------------------
    // WinterS3 draw
    // -------------------------------------------------------------------
    void WinterS3::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX();
            float maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY();
            float maxY = AEGfxGetWinMaxY();
            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX), (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible)
            drawGrid();

        //printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Winter Stage 3 - 32x20 Grid");
        //printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        //printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        // Draw teleport indicator: highlight all portal tiles (type 3) in the top row.
        // Falls back to cols 1-2 if no type-3 tile is found, matching the detection logic.
        if (!camera::isTransitioning())
        {
            int teleportRow = 19;
            bool drewAny = false;

            for (int col = 0; col < gridCols; ++col)
            {
                if (tileMap[teleportRow][col] == 3)
                {
                    float gridWorldX, gridWorldY, cellW, cellH;
                    gridToWorld(col, teleportRow, gridWorldX, gridWorldY, cellW, cellH);
                    gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };
                    portalPos.x = std::round(portalPos.x);
                    portalPos.y = std::round(portalPos.y);
                    gfx::Vec2 portalSize{ cellW, cellH };
                    gfx::drawRectangle(portalPos, 0.0f, portalSize, 0xAA00FFFFu);
                    drewAny = true;
                }
            }

            // Fallback: draw at cols 1-2 if no portal tile found in tilemap
            if (!drewAny)
            {
                for (int c = 0; c < 2; ++c)
                {
                    int col = 1 + c;
                    if (col < gridCols)
                    {
                        float gridWorldX, gridWorldY, cellW, cellH;
                        gridToWorld(col, teleportRow, gridWorldX, gridWorldY, cellW, cellH);
                        gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };
                        portalPos.x = std::round(portalPos.x);
                        portalPos.y = std::round(portalPos.y);
                        gfx::Vec2 portalSize{ cellW, cellH };
                        gfx::drawRectangle(portalPos, 0.0f, portalSize, 0xAA00FFFFu);
                    }
                }
            }
        }

        PlayerDraw(gGame.player);

        // ---- DRAW SNOW (on top of everything) ----
        drawSnow(snowParticles);
    }

    // -------------------------------------------------------------------
    // WinterS3 gridToWorld
    // -------------------------------------------------------------------
    void WinterS3::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------
    // WinterS3 drawTiles
    // -------------------------------------------------------------------
    void WinterS3::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols)
                    return false;

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
                u32 borderColor = 0xFF000000;

                // -------------------------
                // 1. Draw Tile First
                // -------------------------

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


                // Ice tile
                if (tileType == 11)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex)
                        gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }

                //Breaking Ice tile (type 1)
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

                // Spikes tile
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            // UP-facing: anchor base to cell bottom, sink slightly to close gap
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            // DOWN-facing: anchor to cell top, sink slightly to close gap
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }

                        float u0 = 0.0f;
                        float v0 = 0.0f;
                        float u1 = 1.0f;
                        float v1 = 1.0f;

                        if (tileType == 9)
                        {
                            v0 = 1.0f;
                            v1 = 0.0f;
                        }

                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Left-facing spike (26) and right-facing spike (27)
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
                // Cell-fit left spike (21) and right spike (22)
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        // Draw sideways: swap width/height so portrait spike lies on side
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }       // left: spike points right
                        else { u0 = 1.0f; u1 = 0.0f; }                       // right: mirrored, points left
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (ID 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // WinterC (ID 4) and WinterT (ID 6) standalone textures
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
                // Normal tiles
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                    {
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    }
                    else
                    {
                        u32 tileColor = getTileColor(tileType);
                        gfx::drawRectangle(pos, 0.0f, size, tileColor);
                    }
                }

                // -------------------------
                // 2. Draw Borders LAST (Only Solid Tiles)
                // -------------------------

                if (!isSolid(row, col))
                    continue;

                // TOP
                if (!isSolid(row + 1, col))
                {
                    gfx::Vec2 topPos{ pos.x, pos.y + size.y * 0.5f - border * 0.5f };
                    gfx::Vec2 topSize{ size.x, border };
                    gfx::drawRectangle(topPos, 0.0f, topSize, borderColor);
                }

                // BOTTOM
                if (!isSolid(row - 1, col))
                {
                    gfx::Vec2 bottomPos{ pos.x, pos.y - size.y * 0.5f + border * 0.5f };
                    gfx::Vec2 bottomSize{ size.x, border };
                    gfx::drawRectangle(bottomPos, 0.0f, bottomSize, borderColor);
                }

                // LEFT
                if (!isSolid(row, col - 1))
                {
                    gfx::Vec2 leftPos{ pos.x - size.x * 0.5f + border * 0.5f, pos.y };
                    gfx::Vec2 leftSize{ border, size.y };
                    gfx::drawRectangle(leftPos, 0.0f, leftSize, borderColor);
                }

                // RIGHT
                if (!isSolid(row, col + 1))
                {
                    gfx::Vec2 rightPos{ pos.x + size.x * 0.5f - border * 0.5f, pos.y };
                    gfx::Vec2 rightSize{ border, size.y };
                    gfx::drawRectangle(rightPos, 0.0f, rightSize, borderColor);
                }
            }
        }
    }

    // -------------------------------------------------------------------
    // WinterS3 drawGrid
    // -------------------------------------------------------------------
    void WinterS3::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();
        float screenW = (maxX - minX);
        float screenH = (maxY - minY);
        float cellW = screenW / static_cast<float>(gridCols);
        float cellH = screenH / static_cast<float>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            float centerY = (minY + maxY) * 0.5f;
            gfx::Vec2 pos{ x, centerY };
            gfx::Vec2 size{ thickness, screenH };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }

        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::Vec2 pos{ (minX + maxX) * 0.5f, y };
            gfx::Vec2 size{ screenW, thickness };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }
    }

    // ===================================================================
    // WINTER STAGE 4 IMPLEMENTATION
    // ===================================================================

    u32 WinterS4::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u;
        case 3: return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------
    // WinterS4 Constructor
    // -------------------------------------------------------------------
    WinterS4::WinterS4()
        : gridVisible(false)
        , tileMap{}
        , snowInitialized(false)
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s4.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // (Check that Assets/Levels/winter_s4.txt exists and has 20 rows x 32 cols.)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col) {

                //! all breaking ice will be store in vector "iceTiles" and type will be struct "IceTileState"
                if (tileMap[row][col] == 1) {
                    IceTileState ice;
                    ice.row = row;
                    ice.col = col;
                    iceTiles.push_back(ice);
                }
            }

    }

    WinterS4::~WinterS4() = default;

    // -------------------------------------------------------------------
    // WinterS4 update
    // -------------------------------------------------------------------
    int WinterS4::update(float dt)
    {
        // Initialize snow on first update (window is guaranteed ready here)
        if (!snowInitialized)
        {
            initSnow(snowParticles);
            snowInitialized = true;
        }

        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }


        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        sprite::updateAnimatedTiles(dt);


        for (auto& trigger : g_triggeredIceTiles)
            for (auto& ice : iceTiles)
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered) ice.triggered = true;

        //! clear the list to save the memory
        g_triggeredIceTiles.clear();

        //! handle ice have been triggered but tileType havent change to 0(destroyed)
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

        //! if player dead and respawning, reset all ice states 
        if (gGame.player.respawning) {
            for (auto& ice : iceTiles) {
                tileMap[ice.row][ice.col] = 1;
                ice.triggered = false;
                ice.timer = 0.0f;
                ice.crackFrame = 0;
                ice.destroyed = false;
            }
        }


        // Check if player reached the teleport zone to Summer Stage 1
        if (!camera::isTransitioning())
        {
            int teleportCol = 29;
            int teleportRow = 19;
            float gridWorldX, gridWorldY, cellW, cellH;
            gridToWorld(teleportCol, teleportRow, gridWorldX, gridWorldY, cellW, cellH);
            float teleportCenterX = gridWorldX + cellW * 1.0f;
            float teleportCenterY = gridWorldY + cellH * 0.5f;
            float dx = gGame.player.pos.x - teleportCenterX;
            float dy = gGame.player.pos.y - teleportCenterY;
            float distance = sqrtf(dx * dx + dy * dy);
            if (distance < cellW * 1.5f)
                return 23; // Signal teleport to Summer Stage 1
        }
        // Update snow particles
        updateSnow(snowParticles, snowSpawnTimer, dt);

        return 0;
    }

    // -------------------------------------------------------------------
    // WinterS4 draw
    // -------------------------------------------------------------------
    void WinterS4::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX();
            float maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY();
            float maxY = AEGfxGetWinMaxY();
            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX), (maxY - minY) };
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible)
            drawGrid();



        // Draw teleport indicator to Summer Stage 1 (2 cells at col 1-2, row 19)
        if (!camera::isTransitioning())
        {
            for (int c = 0; c < 30; c++)
            {
                int col = 29 + c;
                float gridWorldX, gridWorldY, cellW, cellH;
                gridToWorld(col, 19, gridWorldX, gridWorldY, cellW, cellH);
                gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };
                portalPos.x = std::round(portalPos.x);
                portalPos.y = std::round(portalPos.y);
                gfx::Vec2 portalSize{ cellW, cellH };
                gfx::drawRectangle(portalPos, 0.0f, portalSize, 0xAAFF8800u); // Orange = leads to Summer
            }
        }

        PlayerDraw(gGame.player);

        // ---- DRAW SNOW (on top of everything) ----
        drawSnow(snowParticles);
    }

    // -------------------------------------------------------------------
    // WinterS4 gridToWorld
    // -------------------------------------------------------------------
    void WinterS4::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------
    // WinterS4 drawTiles
    // -------------------------------------------------------------------
    void WinterS4::drawTiles() const
    {
        auto isSolid = [&](int r, int c) -> bool
            {
                if (r < 0 || r >= gridRows || c < 0 || c >= gridCols)
                    return false;

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
                u32 borderColor = 0xFF000000;

                // -------------------------
                // 1. Draw Tile First
                // -------------------------

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

                //Breaking Ice tile (type 1)
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

                // Ice tile
                if (tileType == 11)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex)
                        gfx::drawSprite(iceTex, pos, 0.0f, size, 0, 0, 1, 1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                }
                // Spikes tile
                if (tileType == 9 || tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            // UP-facing: anchor base to cell bottom, sink slightly to close gap
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                            spikePos.y -= size.y * 0.12f;
                        }
                        else {
                            // DOWN-facing: anchor to cell top, sink slightly to close gap
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                            spikePos.y += size.y * 0.12f;
                        }

                        float u0 = 0.0f;
                        float v0 = 0.0f;
                        float u1 = 1.0f;
                        float v1 = 1.0f;

                        if (tileType == 9)
                        {
                            v0 = 1.0f;
                            v1 = 0.0f;
                        }

                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }
                // Left-facing spike (26) and right-facing spike (27)
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
                // Cell-fit left spike (21) and right spike (22)
                else if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        // Draw sideways: swap width/height so portrait spike lies on side
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }       // left: spike points right
                        else { u0 = 1.0f; u1 = 0.0f; }                       // right: mirrored, points left
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                }
                // Grass tile (ID 23)
                else if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // artifacts
                else if (tileType == 34) {
                    AEGfxTexture* winterArtifactsTex = sprite::winterArtifacts();
                    gfx::Vec2 artifactsSize{ size.x * 0.9f, size.y * 0.9f };
                    if (winterArtifactsTex) gfx::drawSprite(winterArtifactsTex, pos, 0.0f, artifactsSize, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                // WinterC (ID 4) and WinterT (ID 6) standalone textures
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
                // Normal tiles
                else
                {
                    float u0{}, v0{}, u1{}, v1{};
                    AEGfxTexture* tex = sprite::tileset();
                    if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                    {
                        gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                    }
                    else
                    {
                        u32 tileColor = getTileColor(tileType);
                        gfx::drawRectangle(pos, 0.0f, size, tileColor);
                    }
                }

                // -------------------------
                // 2. Draw Borders LAST (Only Solid Tiles)
                // -------------------------

                if (!isSolid(row, col))
                    continue;

                // TOP
                if (!isSolid(row + 1, col))
                {
                    gfx::Vec2 topPos{ pos.x, pos.y + size.y * 0.5f - border * 0.5f };
                    gfx::Vec2 topSize{ size.x, border };
                    gfx::drawRectangle(topPos, 0.0f, topSize, borderColor);
                }

                // BOTTOM
                if (!isSolid(row - 1, col))
                {
                    gfx::Vec2 bottomPos{ pos.x, pos.y - size.y * 0.5f + border * 0.5f };
                    gfx::Vec2 bottomSize{ size.x, border };
                    gfx::drawRectangle(bottomPos, 0.0f, bottomSize, borderColor);
                }

                // LEFT
                if (!isSolid(row, col - 1))
                {
                    gfx::Vec2 leftPos{ pos.x - size.x * 0.5f + border * 0.5f, pos.y };
                    gfx::Vec2 leftSize{ border, size.y };
                    gfx::drawRectangle(leftPos, 0.0f, leftSize, borderColor);
                }

                // RIGHT
                if (!isSolid(row, col + 1))
                {
                    gfx::Vec2 rightPos{ pos.x + size.x * 0.5f - border * 0.5f, pos.y };
                    gfx::Vec2 rightSize{ border, size.y };
                    gfx::drawRectangle(rightPos, 0.0f, rightSize, borderColor);
                }
            }
        }
    }

    // -------------------------------------------------------------------
    // WinterS4 drawGrid
    // -------------------------------------------------------------------
    void WinterS4::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();
        float screenW = (maxX - minX);
        float screenH = (maxY - minY);
        float cellW = screenW / static_cast<float>(gridCols);
        float cellH = screenH / static_cast<float>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            float centerY = (minY + maxY) * 0.5f;
            gfx::Vec2 pos{ x, centerY };
            gfx::Vec2 size{ thickness, screenH };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }

        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::Vec2 pos{ (minX + maxX) * 0.5f, y };
            gfx::Vec2 size{ screenW, thickness };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }
    }

} // namespace game