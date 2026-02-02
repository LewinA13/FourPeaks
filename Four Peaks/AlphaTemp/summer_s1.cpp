// ---------------------------------------------------------------------------
// winter_s1.cpp
// ---------------------------------------------------------------------------

#include "Summer_s1.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include <cstdint>
#include "gamestate.hpp"
#include "sprite.hpp"
#include <cmath>
#include "camera.hpp"
#include "Summer_s2.hpp"



typedef uint32_t u32;
extern s8 gFontId;

namespace game
{
    static void printText(f32 x, f32 y, u32 argbColor,
        const char* text, f32 scale = 1.0f)
    {
        f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
        f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
        f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
        f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;

        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    u32 game::SummerS1::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u;  // Spikes: red
        case 3: return 0xFF808080u;  // Wall: gray
        default: return 0x00000000u;
        }
    }


    // -------------------------------------------------------------------
    // Constructor 
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
            {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0},
            {6,6,6,6,6,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,4,0,0,0,0,0,0,4,2,2,2,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,6,6,6,6,6,6,6,6,0,0,0,4,4,4,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,0,10,0,0,0,0,0,0,0,0,0,6,6,6,0,4,4,4,4,4,4,4,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,2,2,2,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,0,0,0,0,0,0,0,0},
            {4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,6,4,4,4,0,0,0,0,0,0,0,0},
            {4,4,2,2,2,2,2,9,9,9,9,9,9,9,9,9,0,0,0,0,0,4,4,4,0,0,0,0,0,0,0,0},
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
    // update
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
				camera::startTransitionY(fromY, toY, 0.3f); // tst time = 1 sec
                return 20; // Signal transition to main
            }
        }

        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        // In SummerS1::update(float dt) - add this before "return 0;"

        // Check if player reached the teleport zone (example: top-right area)
        if (!camera::isTransitioning())
        {
            // Define teleport zone in grid coordinates
            int teleportCol = 28;  // Column 27 (right side of map)
            int teleportRow = 19;  // Row 18 (near top)

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
                // Start camera transition to Summer_s2
                float fromY = 0.0f;
                float toY = camera::screenHeight();
                camera::startTransitionY(fromY, toY, 1.0f); // 1 second transition
                return 20; // Signal transition to main
            }
        }


        sprite::updateAnimatedTiles(dt);

        return 0;
    }

    // -------------------------------------------------------------------
    // draw
    // -------------------------------------------------------------------
    void SummerS1::draw() const
    {
		// just in case, set background to black
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);


        // ---- DRAW BACKGROUND FIRST ----
        AEGfxTexture* bg = sprite::background();
        if (bg)
        {
            float minX = AEGfxGetWinMinX();
            float maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY();
            float maxY = AEGfxGetWinMaxY();

            gfx::Vec2 center{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ (maxX - minX), (maxY - minY) };

            // full UVs of the image
            gfx::drawSprite(bg, center, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        // ---- NOW DRAW YOUR LEVEL ----
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        drawTiles();

        if (gridVisible)
            drawGrid();

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 1 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        // Draw teleport indicator (2x1 cells - 2 columns, 1 row)
        if (!camera::isTransitioning())
        {
            int teleportCol = 28;  // Starting column (left edge)
            int teleportRow = 19;  // Row position

            // Draw 2 adjacent cells horizontally
            for (int c = 0; c < 2; c++)
            {
                int col = teleportCol + c;

                if (col < gridCols)
                {
                    float gridWorldX, gridWorldY, cellW, cellH;
                    gridToWorld(col, teleportRow, gridWorldX, gridWorldY, cellW, cellH);

                    gfx::Vec2 portalPos{ gridWorldX + cellW * 0.5f, gridWorldY + cellH * 0.5f };

                    // Round to match tile alignment
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
    // gridToWorld
    // -------------------------------------------------------------------

    void SummerS1::gridToWorld(int col, int row,
        float& xWorld, float& yWorld,
        float& cellW, float& cellH) const
    {
        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);

        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;  // Camera-relative
    }


    // -------------------------------------------------------------------
    // drawTiles
    // -------------------------------------------------------------------
    void game::SummerS1::drawTiles() const
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


				// ice tile (tileType == 9)
                if (tileType == 9)
                {
                    AEGfxTexture* iceTex = sprite::ice();
                    if (iceTex)
                    {
                        gfx::drawSprite(iceTex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
                    }
                    else
                    {
                        // Fallback: light blue color
                        gfx::drawRectangle(pos, 0.0f, size, 0xFFAADDFF);
                    }
                    continue;
                }

                // 1) Spikes tile: draw idle.png
                if (tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        // spikes taller 
                        float heightScale = 1.5f;  // Adjust this value (1.5 to 2.5)
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        // Move position UP so base stays 
                        gfx::Vec2 spikePos = pos;
                        spikePos.y += (spikeSize.y - size.y) * 0.5f;

                        // Full image UVs (v0=top, v1=bottom)
                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    }
                    else
                    {
                        // fallback if texture missing
                        u32 tileColor = getTileColor(tileType);
                        gfx::drawRectangle(pos, 0.0f, size, tileColor);
                    }
                    continue; // important: don't also draw tileset/rect logic below
                }


                // 2) Normal tiles: draw from tileset if it has UVs
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
    // drawGrid
    // -------------------------------------------------------------------
    void SummerS1::drawGrid() const
    {
        const u32 gridColor = 0x80FFFFFF;

        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        float cellW = (maxX - minX) / static_cast<float>(gridCols);
        float cellH = (maxY - minY) / static_cast<float>(gridRows);

        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        // Vertical lines.
        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            gfx::Vec2 pos{ x, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ thickness, maxY - minY };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }

        // Horizontal lines.
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::Vec2 pos{ (minX + maxX) * 0.5f, y };
            gfx::Vec2 size{ maxX - minX, thickness };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }
    }
}
