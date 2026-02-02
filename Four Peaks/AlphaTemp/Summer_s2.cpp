// ---------------------------------------------------------------------------
// winter_s2.cpp
// ---------------------------------------------------------------------------

#include "Summer_s2.hpp"
#include "Summer_s1.hpp"
#include "sprite.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include "gamestate.hpp"
#include "camera.hpp"
#include <cstdint>
#include <cmath>


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

    u32 game::SummerS2::getTileColor(int tileType) const {
        switch (tileType) {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u;  // Spikes: red
        case 3: return 0xFF808080u;  // Wall: gray
        default: return 0x00000000u;
        }
    }


    // -------------------------------------------------------------------
    // Constructor - Design your 32x20 level here!
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
            {4,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0}, //Ground Plat
            {6,0,0,4,4,4,4,4,4,4,4,6,6,6,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0}, //Ground Plat
            {0,0,0,4,4,2,2,4,2,2,4,0,0,0,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0}, //1st Above G
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,6,6,6,6,6,6,4,0,0,0,0,0,0,0,0,0,0,0}, //2nd Above G
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,0,8,0,0,8,0,4,0,0,0,0,0,0,0,0,0,0,0}, //000110010010000000001
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0}, //000110010010000000001
            {0,0,0,4,4,0,0,4,0,0,4,0,0,0,4,4,4,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0}, //000110010010001110001
            {0,0,0,6,6,0,0,6,0,0,6,0,0,0,4,6,6,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0}, //000110010010001110001
            {0,0,0,0,0,0,0,10,0,0,10,0,0,0,4,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0}, //000000000000001000001
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,4,0,0,0,6,6,4,4,0,0,0,0}, //0000000000000010000010001111
            {0,9,9,9,0,0,0,9,9,0,0,0,0,0,4,0,0,6,0,0,4,0,0,0,0,0,2,4,0,0,0,0}, //0000000000000010010010000021
            {4,6,6,6,6,6,6,6,6,6,6,6,6,6,6,0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,0}, //2222111221111110000010000001
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,4,0,0,0,0}, //2000000000000000000010000001
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,4,0,0,4,0,0,0,0}, //2000000000000000000010001001
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,4,0,0,6,0,0,0,0}, //2000000000000000000010001001
            {4,0,0,0,0,0,0,6,6,6,6,6,6,6,6,0,0,4,4,0,6,0,0,0,4,0,0,0,0,6,6,4}, //20000001111111100110100010000111
            // Row 16-19 (empty sky)
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,4,0,0,0,0,0,0,4}, //2000000000000000011000001
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,6,0,0,0,0,0,0,4}, //2000000000000000011000001
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6}, //2000000000000000000000001
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}  //2000000000000000000000000
        };

        // Copy layout into member tileMap.
        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                tileMap[row][col] = levelLayout[row][col];

    }

    SummerS2::~SummerS2() = default;

    // -------------------------------------------------------------------
    // update
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

        // Only update player when NOT transitioning (matches Stage 1 behavior)
        if (!camera::isTransitioning())
        {
            PlayerUpdate(gGame.player, dt);
        }

        sprite::updateAnimatedTiles(dt);
        return 0;


        PlayerUpdate(gGame.player, dt);

        sprite::updateAnimatedTiles(dt);

        return 0;
    }

    // -------------------------------------------------------------------
    // draw
    // -------------------------------------------------------------------

    void SummerS2::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        // ---- DRAW BACKGROUND (same as Stage 1) ----
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
    // gridToWorld
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
    // drawTiles
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

                // Helps reduce shimmering / seams
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
                        // Make spikes taller (1.5x height), same width
                        float heightScale = 1.5f;  // Adjust this value (1.5 to 2.5)
                        gfx::Vec2 spikeSize{ size.x, size.y * heightScale };

                        // Adjust position based on direction
                        gfx::Vec2 spikePos = pos;
                        if (tileType == 2) {
                            // UP-facing: move position UP so base stays at bottom
                            spikePos.y += (spikeSize.y - size.y) * 0.5f;
                        }
                        else {
                            // DOWN-facing: move position DOWN so base stays at top
                            spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                        }

                        // Default UVs (facing UP)
                        float u0 = 0.0f;
                        float v0 = 0.0f;
                        float u1 = 1.0f;
                        float v1 = 1.0f;

                        // Flip vertically for DOWN-facing spikes
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

                // -------------------------------------------------
                // Normal tiles: using the Winter tileset (winter_.png)
                // -------------------------------------------------
                float u0{}, v0{}, u1{}, v1{};
                AEGfxTexture* tex = sprite::tileset();

                if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                {
                    gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                }
                else
                {
                    // fallback for unknown IDs
                    u32 tileColor = getTileColor(tileType);
                    gfx::drawRectangle(pos, 0.0f, size, tileColor);
                }
            }
        }
    }




    // -------------------------------------------------------------------
    // drawGrid
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

        float stage2BottomY = maxY;

        float cellW = screenW / static_cast<float>(gridCols);
        float cellH = screenH / static_cast<float>(gridRows);
        float thickness = (cellW < cellH ? cellW : cellH) * 0.04f;

        // Vertical lines
        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            float centerY = stage2BottomY + screenH * 0.5f;
            gfx::Vec2 pos{ x, centerY };
            gfx::Vec2 size{ thickness, screenH };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }

        // Horizontal lines
        for (int row = 0; row <= gridRows; ++row)
        {
            float y = stage2BottomY + row * cellH;
            gfx::Vec2 pos{ (minX + maxX) * 0.5f, y };
            gfx::Vec2 size{ screenW, thickness };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }
    }
}
