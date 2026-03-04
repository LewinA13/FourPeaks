#include "tutorial.hpp"

#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include "gamestate.hpp"
#include "sprite.hpp"
#include "level_loader.hpp"

#include <cstdint>
#include <cmath>

typedef std::uint32_t u32;
extern s8 gFontId;

namespace
{
    static void printText(f32 x, f32 y, u32 argbColor, const char* text, f32 scale = 1.0f)
    {
        f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
        f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
        f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
        f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;
        AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
    }

    static u32 getTileColor(int tileType)
    {
        switch (tileType)
        {
        case 6: return 0xFF555555u; // underground
        case 7: return 0xFF888888u; // top ground
        case 2: return 0xFFFF3333u; // spikes
        case 8: return 0xFFFFFF00u; // melon (usually drawn animated)
        default: return 0x00000000u;
        }
    }

    static void gridToWorld(int col, int row, int gridCols, int gridRows,
        float& xWorld, float& yWorld, float& cellW, float& cellH)
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

    static void drawGridLines(int gridCols, int gridRows)
    {
        const u32 gridColor = 0x80FFFFFF;

        float minX = AEGfxGetWinMinX();
        float maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY();
        float maxY = AEGfxGetWinMaxY();

        float cellW = (maxX - minX) / static_cast<f32>(gridCols);
        float cellH = (maxY - minY) / static_cast<f32>(gridRows);
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

    static void drawTilesFromMap(const int tileMap[20][32])
    {
        constexpr int gridRows = 20;
        constexpr int gridCols = 32;

        for (int row = 0; row < gridRows; ++row)
        {
            for (int col = 0; col < gridCols; ++col)
            {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);

                gfx::Vec2 pos{ xWorld + cellW * 0.5f, yWorld + cellH * 0.5f };
                gfx::Vec2 size{ cellW, cellH };

                pos.x = std::round(pos.x);
                pos.y = std::round(pos.y);

                // Animated tiles: melon(8), checkpoint(10), crack(1)
                if (sprite::drawAnimatedTile(tileType, pos, size))
                    continue;

                // Spikes (2)
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
                        gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    }
                    continue;
                }

                // Inverted spike (9)
                if (tileType == 9)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 spikeSize{ size.x, size.y * 1.5f };
                        gfx::Vec2 spikePos = pos;
                        spikePos.y -= (spikeSize.y - size.y) * 0.5f;
                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, 0.0f, 1.0f, 1.0f, 0.0f);
                    }
                    continue;
                }

                // Left-facing spike (26) and right-facing spike (27)
                if (tileType == 26 || tileType == 27)
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
                    continue;
                }

                // Grass tile (ID 23)
                if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                    continue;
                }

                // Standalone seasonal tiles (replace old sprites for IDs 1,3,5,7)
                if (tileType == 1)
                {
                    AEGfxTexture* t = sprite::spring1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    continue;
                }
                if (tileType == 3)
                {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    continue;
                }
                if (tileType == 5)
                {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    continue;
                }
                if (tileType == 7)
                {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    continue;
                }

                if (tileType == 19)
                {
                    AEGfxTexture* tex = sprite::sign();
                    size.x *= 0.8f;
                    size.y *= 0.8f;
                    pos.y -= (cellH - size.y) * 0.5f;  // allign to btm
                    if (tex) gfx::drawSprite(tex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(pos, 0.0f, size, 0xFF88FF88u);
                }

                // Tileset UV tiles (6/7 etc.)
                float u0{}, v0{}, u1{}, v1{};
                AEGfxTexture* tex = sprite::tileset();
                if (tex && sprite::getTileUv(tileType, u0, v0, u1, v1))
                    gfx::drawSprite(tex, pos, 0.0f, size, u0, v0, u1, v1);
                else
                    gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
            }
        }
    }

    static bool reachedGoalTopRight(const Player& p)
    {
        constexpr int goalCol = 31;
        constexpr int goalRow = 19;
        constexpr int gridRows = 20;
        constexpr int gridCols = 32;

        float xWorld{}, yWorld{}, cellW{}, cellH{};
        gridToWorld(goalCol, goalRow, gridCols, gridRows, xWorld, yWorld, cellW, cellH);

        float cx = xWorld + cellW * 0.5f;
        float cy = yWorld + cellH * 0.5f;

        float dx = p.pos.x - cx;
        float dy = p.pos.y - cy;

        return (std::sqrt(dx * dx + dy * dy) < cellW * 1.0f);
    }

    // ------------------------------------------------------------
    // Tutorial background (full-screen sprite)
    // ------------------------------------------------------------
    static void drawTutorialBackground()
    {
        AEGfxTexture* bg = sprite::tutorialBackground();
        if (!bg)
            return;

        const float minX = AEGfxGetWinMinX();
        const float maxX = AEGfxGetWinMaxX();
        const float minY = AEGfxGetWinMinY();
        const float maxY = AEGfxGetWinMaxY();

        gfx::Vec2 pos{ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f };
        gfx::Vec2 size{ (maxX - minX), (maxY - minY) };

        // Whole texture
        gfx::drawSprite(bg, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
    }

    // ------------------------------------------------------------
    // Simple UI "buttons" (clickable text regions)
    // Returns:
    //   0  = no action
    //   30 = go Tutorial2
    //   31 = go Tutorial3
    //   32 = go WinterS1
    // ------------------------------------------------------------
    struct UiButton
    {
        const char* label;
        f32 x;
        f32 y;
        f32 w;
        f32 h;
        int action;
    };

    static void getMouseNormalized(f32& nx, f32& ny)
    {
        s32 mx = 0, my = 0;
        AEInputGetCursorPosition(&mx, &my);

        const f32 w = (f32)AEGfxGetWindowWidth();
        const f32 h = (f32)AEGfxGetWindowHeight();

        // Screen (0..w, 0..h) -> Normalized (-1..1, -1..1)
        nx = (w > 0.0f) ? ((mx / w) * 2.0f - 1.0f) : 0.0f;
        ny = (h > 0.0f) ? (1.0f - (my / h) * 2.0f) : 0.0f;
    }

    static bool pointInRect(f32 px, f32 py, const UiButton& b)
    {
        return (px >= b.x && px <= (b.x + b.w) && py >= b.y && py <= (b.y + b.h));
    }

    static int tutorialNavButtons(int currentTutorial) // 1/2/3
    {
        // Top-right row of buttons.
        // (x,y) is bottom-left in normalized coords.
        UiButton buttons[] =
        {
            { "T2",  0.55f, 0.90f, 0.10f, 0.07f, 30 },
            { "T3",  0.67f, 0.90f, 0.10f, 0.07f, 31 },
            { "W1",  0.79f, 0.90f, 0.12f, 0.07f, 32 },
        };

        // Draw button labels (always visible)
        printText(0.55f, 0.90f, 0xFFFFFFFFu, "[T2]", 1.0f);
        printText(0.67f, 0.90f, 0xFFFFFFFFu, "[T3]", 1.0f);
        printText(0.79f, 0.90f, 0xFFFFFFFFu, "[W1]", 1.0f);

        // Also show which tutorial you're in
        (void)currentTutorial;

        // Keyboard fallback (so you can test fast)
        if (AEInputCheckTriggered(AEVK_F2)) return 30;
        if (AEInputCheckTriggered(AEVK_F3)) return 31;
        if (AEInputCheckTriggered(AEVK_1))  return 32; // jump to Winter S1

        // Mouse click
        if (!AEInputCheckTriggered(AEVK_LBUTTON))
            return 0;

        f32 mxN{}, myN{};
        getMouseNormalized(mxN, myN);

        for (const UiButton& b : buttons)
        {
            if (pointInRect(mxN, myN, b))
                return b.action;
        }

        return 0;
    }
}

namespace game
{
    // ---------------- Tutorial 1 ----------------
    Tutorial1::Tutorial1()
    {
        level::loadTileMap("Assets/Levels/tutorial_1.txt", 20, 32, &tileMap[0][0]);
    }

    int (*Tutorial1::getTileMap())[32] { return tileMap; }

    int Tutorial1::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

        // UI navigation buttons
        if (int nav = tutorialNavButtons(1))
            return nav;

        PlayerUpdate(gGame.player, dt);
        sprite::updateAnimatedTiles(dt);

        if (reachedGoalTopRight(gGame.player)) return 30; // -> Tutorial2
        return 0;
    }

    void Tutorial1::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);
        drawTutorialBackground();
        drawTilesFromMap(tileMap);
        if (gridVisible) drawGridLines(32, 20);

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Tutorial 1");
        printText(-0.95f, 0.8f, 0xFFFFFFFFu, "G: grid   ESC: menu");

        PlayerDraw(gGame.player);
    }

    // ---------------- Tutorial 2 ----------------
    Tutorial2::Tutorial2()
    {
        level::loadTileMap("Assets/Levels/tutorial_2.txt", 20, 32, &tileMap[0][0]);
    }

    int (*Tutorial2::getTileMap())[32] { return tileMap; }

    int Tutorial2::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

        // UI navigation buttons
        if (int nav = tutorialNavButtons(2))
            return nav;

        PlayerUpdate(gGame.player, dt);
        sprite::updateAnimatedTiles(dt);

        if (reachedGoalTopRight(gGame.player)) return 31; // -> Tutorial3
        return 0;
    }

    void Tutorial2::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);
        drawTutorialBackground();
        drawTilesFromMap(tileMap);
        if (gridVisible) drawGridLines(32, 20);

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Tutorial 2");
        PlayerDraw(gGame.player);
    }

    // ---------------- Tutorial 3 ----------------
    Tutorial3::Tutorial3()
    {
        level::loadTileMap("Assets/Levels/tutorial_3.txt", 20, 32, &tileMap[0][0]);
    }

    int (*Tutorial3::getTileMap())[32] { return tileMap; }

    int Tutorial3::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

        // UI navigation buttons
        if (int nav = tutorialNavButtons(3))
            return nav;

        PlayerUpdate(gGame.player, dt);
        sprite::updateAnimatedTiles(dt);

        if (reachedGoalTopRight(gGame.player)) return 32; // -> WinterS1
        return 0;
    }

    void Tutorial3::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);
        drawTutorialBackground();
        drawTilesFromMap(tileMap);
        if (gridVisible) drawGridLines(32, 20);

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Tutorial 3");
        PlayerDraw(gGame.player);
    }
}
