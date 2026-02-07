// ---------------------------------------------------------------------------
// Summer_s1.cpp - Combined Summer Stages (Stage 1 + Stage 2)
// ---------------------------------------------------------------------------

#include "Summer_s1.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include "gamestate.hpp"
#include "sprite.hpp"
#include "camera.hpp"
#include <cstdint>
#include <cmath>

typedef uint32_t u32;

extern s8 gFontId;

namespace game {

    // -------------------------------------------------------------------
    // Helper function - shared by all stages
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

    u32 SummerS1::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u; // Spikes: red
        case 3: return 0xFF808080u; // Wall: gray
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------
    // SummerS1 Constructor
    // -------------------------------------------------------------------
    SummerS1::SummerS1()
        : gridVisible(true)
        , tileMap{}
    {
        // LEVEL DESIGN: 0 = empty, 1 = solid block
        // 32 columns wide, 20 rows tall
        // Bottom row = Row 0, Top row = Row 19
        int levelLayout[gridRows][gridCols] = {
            // Row 0 (bottom ground)
            {1,5,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0},
            {6,6,6,6,6,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,4,0,0,0,0,0,0,4,2,2,2,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,6,6,6,6,6,6,6,6,0,0,0,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,0,10,0,0,0,0,0,0,0,0,0,6,6,6,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,2,2,2,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,0,0,0,0,0,0,0,0},
            {4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,6,4,4,4,0,0,0,0,0,0,0,0},
            {4,4,2,2,2,2,2,1,1,1,1,1,1,1,1,1,0,0,0,0,0,4,4,4,0,0,0,0,0,0,0,0},
            {4,4,0,0,0,0,0,8,0,0,8,0,0,0,0,0,0,0,0,0,0,4,2,2,0,0,0,0,0,0,0,0},
            {4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0},
            {4,4,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0},
            {4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,6,6,6,6,4,0},
            {4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0},
            // Row 16-19 (empty sky)
            {4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0},
            {4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,4,0},
            {4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,4,0},
            {4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,4,0}
        };

        // Copy layout into member tileMap.
        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                tileMap[row][col] = levelLayout[row][col];
    }

    SummerS1::~SummerS1() = default;

    // -------------------------------------------------------------------
    // SummerS1 update
    // -------------------------------------------------------------------
    int SummerS1::update(float dt)
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
    // SummerS1 draw
    // -------------------------------------------------------------------
    void SummerS1::draw() const
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

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 1 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

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
    // SummerS1 gridToWorld
    // -------------------------------------------------------------------
    void SummerS1::gridToWorld(int col, int row,
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
    // SummerS1 drawTiles
    // -------------------------------------------------------------------
    void SummerS1::drawTiles() const
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

                // --------------------------------------------
                // White wall pieces (NO tileset UV system)
                // 1 = top, 3 = left, 4 = right, 5 = bottom
                // --------------------------------------------
                if (tileType == 1 || tileType == 3 || tileType == 4 || tileType == 5)
                {
                    AEGfxTexture* wallTex = nullptr;

                    switch (tileType)
                    {
                    case 1: wallTex = sprite::wallTop();    break;
                    case 3: wallTex = sprite::wallLeft();   break;
                    case 4: wallTex = sprite::wallRight();  break;
                    case 5: wallTex = sprite::wallBottom(); break;
                    default: break;
                    }

                    if (wallTex)
                        gfx::drawSprite(wallTex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFFFFFFF);

                    continue;
                }

                // Fallback for any other unknown tiles (so they don't disappear)
                u32 tileColor = getTileColor(tileType);
                gfx::drawRectangle(pos, 0.0f, size, tileColor);

            }
        }
    }

    // -------------------------------------------------------------------
    // SummerS1 drawGrid
    // -------------------------------------------------------------------
    void SummerS1::drawGrid() const
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

    u32 SummerS2::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u; // Spikes: red
        case 3: return 0xFF808080u; // Wall: gray
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------
    // SummerS2 Constructor
    // -------------------------------------------------------------------
    SummerS2::SummerS2()
        : gridVisible(true)
        , tileMap{}
    {
        // LEVEL DESIGN: 0 = empty, 1 = solid block
        // 32 columns wide, 20 rows tall
        // Bottom row = Row 0, Top row = Row 19
        int levelLayout[gridRows][gridCols] = {
            // Row 0 (bottom ground)
            {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0},
            {6,6,6,4,4,4,4,4,4,4,4,6,6,6,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,4,4,2,2,4,2,2,4,0,0,0,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,6,6,6,6,6,6,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,0,8,0,0,8,0,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,4,4,4,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,6,6,0,0,6,0,0,6,0,0,0,4,6,6,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,10,0,0,10,0,0,0,4,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,4,0,0,0,6,6,4,4,0,0,0,0},
            {0,9,9,9,0,0,0,9,9,0,0,0,0,0,4,0,0,6,0,0,4,0,0,0,0,0,2,4,0,0,0,0},
            {4,6,6,6,6,6,6,6,6,6,6,6,6,6,6,0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,0},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,0},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,4,0,0,4,0,0,0,0},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,4,0,0,6,0,0,0,0},
            {4,0,0,0,0,0,0,6,6,6,6,6,6,6,6,0,0,4,4,0,6,0,0,0,4,0,0,0,0,6,6,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,4,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,6,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        };

        // Copy layout into member tileMap.
        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                tileMap[row][col] = levelLayout[row][col];
    }

    SummerS2::~SummerS2() = default;

    // -------------------------------------------------------------------
    // SummerS2 update
    // -------------------------------------------------------------------
    int SummerS2::update(float dt)
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

        sprite::updateAnimatedTiles(dt);
        return 0;
    }

    // -------------------------------------------------------------------
    // SummerS2 draw
    // -------------------------------------------------------------------
    void SummerS2::draw() const
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

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 2 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        PlayerDraw(gGame.player);
    }

    // -------------------------------------------------------------------
    // SummerS2 gridToWorld
    // -------------------------------------------------------------------
    void SummerS2::gridToWorld(int col, int row,
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
    // SummerS2 drawTiles
    // -------------------------------------------------------------------
    void SummerS2::drawTiles() const
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

                // --------------------------------------------

                if (tileType == 1 || tileType == 3 || tileType == 4 || tileType == 5)
                {
                    AEGfxTexture* wallTex = nullptr;

                    switch (tileType)
                    {
                    case 1: wallTex = sprite::wallTop();    break;
                    case 3: wallTex = sprite::wallLeft();   break;
                    case 4: wallTex = sprite::wallRight();  break;
                    case 5: wallTex = sprite::wallBottom(); break;
                    default: break;
                    }

                    if (wallTex)
                        gfx::drawSprite(wallTex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFFFFFFF);

                    continue;
                }

                // Fallback for any other unknown tiles (so they don't disappear)
                u32 tileColor = getTileColor(tileType);
                gfx::drawRectangle(pos, 0.0f, size, tileColor);

            }
        }
    }

    // -------------------------------------------------------------------
    // SummerS2 drawGrid
    // -------------------------------------------------------------------
    void SummerS2::drawGrid() const
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

} // namespace game
