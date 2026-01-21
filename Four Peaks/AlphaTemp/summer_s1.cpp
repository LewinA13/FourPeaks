// ---------------------------------------------------------------------------
// summer_s1.cpp
// ---------------------------------------------------------------------------

#include "summer_s1.hpp"
#include "AEEngine.h"
#include "AEFrameRateController.h"
#include "graphics.hpp"
#include <cstdint>

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

    u32 SummerS1::getTileColor(int tileType) const
    {
        switch (tileType)
        {
        case 1: return 0xFF224B94u;
        case 2: return 0xFFA3B013u;
        case 3: return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------
    // Constructor - Design your 32x20 level here!
    // -------------------------------------------------------------------
    SummerS1::SummerS1()
        : gridVisible(true)
        , tileMap{}
    {
        int levelLayout[gridRows][gridCols] = {
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,0,0,0,1,2,2,2,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
            {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},
            {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},
            {1,1,2,2,2,2,2,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,0,0,0,0,0,0,0,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
            {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0},
            {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0}
        };

        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                tileMap[row][col] = levelLayout[row][col];

        // Spawn / initialise player for this level
        PlayerInit(player);
    }

    SummerS1::~SummerS1()
    {
        PlayerShutdown(player);
    }

    // -------------------------------------------------------------------
    // update
    // -------------------------------------------------------------------
    int SummerS1::update()
    {
        float dt = static_cast<float>(AEFrameRateControllerGetFrameTime());
        PlayerUpdate(player, dt);

        if (AEInputCheckTriggered(AEVK_G))
            gridVisible = !gridVisible;

        if (AEInputCheckTriggered(AEVK_ESCAPE))
            return 2;

        return 0;
    }

    // -------------------------------------------------------------------
    // draw
    // -------------------------------------------------------------------
    void SummerS1::draw() const
    {
        AEGfxSetBackgroundColor(0.3f, 0.6f, 0.8f);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        drawTiles();

        // Player on top of tiles
        PlayerDraw(player);

        if (gridVisible)
            drawGrid();

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 1 - 32x20 Grid");
        printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");
    }

    // -------------------------------------------------------------------
    // gridToWorld
    // -------------------------------------------------------------------
    void SummerS1::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
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

    void SummerS1::drawTiles() const
    {
        for (int row = 0; row < gridRows; ++row)
        {
            for (int col = 0; col < gridCols; ++col)
            {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;

                u32 tileColor = getTileColor(tileType);
                float xWorld, yWorld, cellW, cellH;
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);

                gfx::Vec2 pos{ xWorld + cellW * 0.5f, yWorld + cellH * 0.5f };
                gfx::Vec2 size{ cellW, cellH };
                gfx::drawRectangle(pos, 0.0f, size, tileColor);
            }
        }
    }

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

        for (int col = 0; col <= gridCols; ++col)
        {
            float x = minX + col * cellW;
            gfx::Vec2 pos{ x, (minY + maxY) * 0.5f };
            gfx::Vec2 size{ thickness, maxY - minY };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }

        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::Vec2 pos{ (minX + maxX) * 0.5f, y };
            gfx::Vec2 size{ maxX - minX, thickness };
            gfx::drawRectangle(pos, 0.0f, size, gridColor);
        }
    }
}
