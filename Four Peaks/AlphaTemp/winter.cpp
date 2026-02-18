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

    // ===================================================================
    // SUMMER STAGE 1 IMPLEMENTATION
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
        : gridVisible(true)
        , tileMap{}
    {
        // LEVEL DESIGN: 0 = empty, 1 = solid block
        // 32 columns wide, 20 rows tall
        // Bottom row = Row 0, Top row = Row 19
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s1.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // No hardcoded fallback: keep the map empty if the file is missing/invalid.
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
        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }

        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            return 2;
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

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Winter Stage 1 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        // melon counter and display
        std::ostringstream ss;
        ss << "Melons: " << gGame.player.melonsCollected;

        std::string hud = ss.str(); // keep it alive
        printText(-0.95f, 0.5f, 0xFFFFFF00u, hud.c_str(), 1);

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

                if (sprite::drawAnimatedTile(tileType, pos, size))
                    continue;

                // Ice tile (tileType == 9)
                if (tileType == 9)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex)
                    {
                        gfx::drawSprite(iceTex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
                    }
                    else
                    {
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                    }
                    continue;
                }

                // Spikes tile
                if (tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        gfx::Vec2 spikePos = pos;
                        spikePos.y += (spikeSize.y - size.y) * 0.5f;

                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    }
                    else
                    {
                        u32 tileColor = getTileColor(tileType);
                        gfx::drawRectangle(pos, 0.0f, size, tileColor);
                    }
                    continue;
                }

                // Normal tiles
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
    // SUMMER STAGE 2 IMPLEMENTATION
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
        : gridVisible(true)
        , tileMap{}
    {
        // LEVEL DESIGN: 0 = empty, 1 = solid block
        // 32 columns wide, 20 rows tall
        // Bottom row = Row 0, Top row = Row 19
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s2.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // No hardcoded fallback: keep the map empty if the file is missing/invalid.
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
        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }

        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            return 2;
        }

        // Only update player when NOT transitioning
        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        // ADD THIS: Check if player reached the teleport zone to Stage 3
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

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Winter Stage 2 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        // ADD THIS: Draw teleport indicator (1x2 cells at column 31, rows 18-19)
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

                // -------------------------------------------------
                // SPIKES
                // tileType == 2 : spike facing UP
                // tileType == 9 : spike facing DOWN
                // -------------------------------------------------
                if (tileType == 2 || tileType == 9)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            // UP-facing
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                        }
                        else {
                            // DOWN-facing
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
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

                if (sprite::drawAnimatedTile(tileType, pos, size))
                    continue;

                // Normal tiles
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
    // SUMMER STAGE 3 IMPLEMENTATION
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
        : gridVisible(true)
        , tileMap{}
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s3.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // No hardcoded fallback: keep the map empty if the file is missing/invalid.
            // (Check that Assets/Levels/winter_s3.txt exists and has 20 rows x 32 cols.)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

    }

    WinterS3::~WinterS3() = default;

    // -------------------------------------------------------------------
    // WinterS3 update
    // -------------------------------------------------------------------
    int WinterS3::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }

        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            return 2;
        }

        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        // ADD THIS: Check if player reached the teleport zone to Stage 4
        if (!camera::isTransitioning())
        {
            // Define teleport zone in grid coordinates (columns 2-3, row 19)
            int teleportRow = 19;
            int teleportCol1 = 1;
            int teleportCol2 = 2;

            // Convert grid position to world coordinates
            float gridWorldX, gridWorldY, cellW, cellH;
            gridToWorld(teleportCol1, teleportRow, gridWorldX, gridWorldY, cellW, cellH);

            // Check if player is within the teleport zone (2 cells wide)
            float teleportCenterX = gridWorldX + cellW * 1.0f; // Center between 2 cells
            float teleportCenterY = gridWorldY + cellH * 0.5f;

            // Distance check (within 1.5 cells)
            float dx = gGame.player.pos.x - teleportCenterX;
            float dy = gGame.player.pos.y - teleportCenterY;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < cellW * 1.5f)
            {
                return 22; // Signal teleport to Stage 4
            }
        }

        sprite::updateAnimatedTiles(dt);
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

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Winter Stage 3 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        // ADD THIS: Draw teleport indicator (2x1 cells at row 19, columns 2-3)
        if (!camera::isTransitioning())
        {
            int teleportRow = 19;
            for (int c = 0; c < 2; c++)
            {
                int col = 1 + c; // Columns 2 and 3
                if (col < gridCols)
                {
                    float gridWorldX, gridWorldY, cellW, cellH;
                    gridToWorld(col, teleportRow, gridWorldX, gridWorldY, cellW, cellH);
                    gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };
                    portalPos.x = std::round(portalPos.x);
                    portalPos.y = std::round(portalPos.y);
                    gfx::Vec2 portalSize{ cellW, cellH };
                    gfx::drawRectangle(portalPos, 0.0f, portalSize, 0xAA00FFFF); // portal draw 
                }
            }
        }

        PlayerDraw(gGame.player);
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

                if (tileType == 2 || tileType == 9)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };
                        gfx::Vec2 spikePos = pos;

                        if (tileType == 2) {
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                        }
                        else {
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                        }

                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9)
                        {
                            v0 = 1.0f;
                            v1 = 0.0f;
                        }

                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (sprite::drawAnimatedTile(tileType, pos, size))
                    continue;

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
    // SUMMER STAGE 4 IMPLEMENTATION
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
    // Winter S4 constructor
    // -------------------------------------------------------------------
    WinterS4::WinterS4()
        : gridVisible(true)
        , tileMap{}
    {
        const bool loaded = level::loadTileMap("Assets/Levels/winter_s4.txt", gridRows, gridCols, &tileMap[0][0]);


        if (!loaded)
        {
            // No hardcoded fallback: keep the map empty if the file is missing/invalid.
            // (Check that Assets/Levels/winter_s4.txt exists and has 20 rows x 32 cols.)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
        }

    }

    WinterS4::~WinterS4() = default;
    // -------------------------------------------------------------------
    // WinterS4 update
    // -------------------------------------------------------------------
    int WinterS4::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G))
        {
            gridVisible = !gridVisible;
        }

        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            return 2;
        }

        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        sprite::updateAnimatedTiles(dt);
        return 0;
    }

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

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Winter Stage 4 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        PlayerDraw(gGame.player);
    }

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

    void WinterS4::drawTiles() const
    {
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

                if (tileType == 2 || tileType == 9)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };
                        gfx::Vec2 spikePos = pos;

                        if (tileType == 2) {
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                        }
                        else {
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                        }

                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9)
                        {
                            v0 = 1.0f;
                            v1 = 0.0f;
                        }

                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (sprite::drawAnimatedTile(tileType, pos, size))
                    continue;

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
        }
    }

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



