#include "tutorial.hpp"

#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include "gamestate.hpp"
#include "sprite.hpp"
#include "level_loader.hpp"
#include "camera.hpp"

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
        case 6:  return 0xFF555555u;
        case 7:  return 0xFF888888u;
        case 2:  return 0xFFFF3333u;
        case 8:  return 0xFFFFFF00u;
        default: return 0x00000000u;
        }
    }

    static void gridToWorld(int col, int row, int gridCols, int gridRows,
        float& xWorld, float& yWorld, float& cellW, float& cellH)
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();

        cellW = (maxX - minX) / static_cast<f32>(gridCols);
        cellH = (maxY - minY) / static_cast<f32>(gridRows);

        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    static void drawGridLines(int gridCols, int gridRows)
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

    // ---------------------------------------------------------------
    // drawTilesFromMap
    // All original sprites are preserved exactly.
    // Solid tiles (1,3,5,6,7,23) fall through to the border pass.
    // All non-solid tiles (spikes, sign, animated, crack) use continue.
    // ---------------------------------------------------------------
    static void drawTilesFromMap(const int tileMap[20][32])
    {
        constexpr int gridRows = 20;
        constexpr int gridCols = 32;

        auto isSolid = [&](int r, int c) -> bool {
            if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
            int t = tileMap[r][c];
            return (t == 1 || t == 3 || t == 5 || t == 6 || t == 7 || t == 23);
            };

        for (int row = 0; row < gridRows; ++row)
        {
            for (int col = 0; col < gridCols; ++col)
            {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);

                gfx::Vec2 pos{ std::round(xWorld + cellW * 0.5f), std::round(yWorld + cellH * 0.5f) };
                gfx::Vec2 size{ cellW, cellH };

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32   borderColor = 0xFF000000;

                // ---- Animated tiles (melon=8, checkpoint=10) — not solid ----
                if (sprite::drawAnimatedTile(tileType, pos, size))
                    continue;

                // ---- Up-facing spike (2) ----
                if (tileType == 2)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 spikeSize{ size.x, size.y * 1.5f };
                        gfx::Vec2 spikePos = pos;
                        spikePos.y += (spikeSize.y - size.y) * 0.5f;
                        gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    }
                    else gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    continue;
                }

                // ---- Down-facing spike (9) ----
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

                // ---- Cell-fit left spike (21) and right spike (22) ----
                if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 22) { u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                // ---- Left-facing spike (26) and right-facing spike (27) ----
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

                // ---- Sign (19) — not solid ----
                if (tileType == 19)
                {
                    AEGfxTexture* tex = sprite::sign();
                    gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.5f };
                    gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
                    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                    if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
                    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                    AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    continue;
                }

                // ----------------------------------------------------------------
                // Solid tiles — draw sprite then fall through to border pass below.
                // ----------------------------------------------------------------
                if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                else if (tileType == 1)
                {
                    AEGfxTexture* t = sprite::spring1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 3)
                {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 5)
                {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 7)
                {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 4)
                {
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                }
                else if (tileType == 6)
                {
                    AEGfxTexture* t = sprite::winterT();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                }
                else
                {
                    // Tileset UV fallback for any other tile IDs
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

    // ---------------------------------------------------------------
    // Tutorial background
    // ---------------------------------------------------------------
    static void drawTutorialBackground()
    {
        AEGfxTexture* bg = sprite::tutorialBackground();
        if (!bg) return;

        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();

        gfx::drawSprite(bg,
            { (minX + maxX) * 0.5f, (minY + maxY) * 0.5f }, 0.0f,
            { maxX - minX, maxY - minY },
            0.0f, 0.0f, 1.0f, 1.0f);
    }

    // ---------------------------------------------------------------
    // Tutorial navigation buttons
    // ---------------------------------------------------------------
    struct UiButton
    {
        const char* label;
        f32 x, y, w, h;
        int action;
    };

    static void getMouseNormalized(f32& nx, f32& ny)
    {
        s32 mx = 0, my = 0;
        AEInputGetCursorPosition(&mx, &my);

        const f32 w = (f32)AEGfxGetWindowWidth();
        const f32 h = (f32)AEGfxGetWindowHeight();

        nx = (w > 0.0f) ? ((mx / w) * 2.0f - 1.0f) : 0.0f;
        ny = (h > 0.0f) ? (1.0f - (my / h) * 2.0f) : 0.0f;
    }

    static bool pointInRect(f32 px, f32 py, const UiButton& b)
    {
        return (px >= b.x && px <= (b.x + b.w) && py >= b.y && py <= (b.y + b.h));
    }

    static int tutorialNavButtons(int currentTutorial)
    {
        UiButton buttons[] =
        {
            { "T2", 0.55f, 0.90f, 0.10f, 0.07f, 30 },
            { "T3", 0.67f, 0.90f, 0.10f, 0.07f, 31 },
            { "W1", 0.79f, 0.90f, 0.12f, 0.07f, 32 },
        };

        printText(0.55f, 0.90f, 0xFFFFFFFFu, "[T2]", 1.0f);
        printText(0.67f, 0.90f, 0xFFFFFFFFu, "[T3]", 1.0f);
        printText(0.79f, 0.90f, 0xFFFFFFFFu, "[W1]", 1.0f);

        (void)currentTutorial;

        if (AEInputCheckTriggered(AEVK_F2)) return 30;
        if (AEInputCheckTriggered(AEVK_F3)) return 31;
        if (AEInputCheckTriggered(AEVK_1))  return 32;

        if (!AEInputCheckTriggered(AEVK_LBUTTON))
            return 0;

        f32 mxN{}, myN{};
        getMouseNormalized(mxN, myN);

        for (const UiButton& b : buttons)
            if (pointInRect(mxN, myN, b))
                return b.action;

        return 0;
    }

} // anonymous namespace

namespace game
{
    // ---------------------------------------------------------------
    // Tutorial 1
    // ---------------------------------------------------------------
    Tutorial1::Tutorial1()
    {
        level::loadTileMap("Assets/Levels/tutorial_1.txt", 20, 32, &tileMap[0][0]);
    }

    int (*Tutorial1::getTileMap())[32] { return tileMap; }

    int Tutorial1::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;
        if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

        if (int nav = tutorialNavButtons(1)) return nav;

        PlayerUpdate(gGame.player, dt);
        sprite::updateAnimatedTiles(dt);

        if (!camera::isTransitioning())
        {
            constexpr int gridRows = 20, gridCols = 32;
            float gx{}, gy{}, cw{}, ch{};
            gridToWorld(1, 19, gridCols, gridRows, gx, gy, cw, ch);
            float dx = gGame.player.pos.x - (gx + cw * 0.5f);
            float dy = gGame.player.pos.y - (gy + ch * 0.5f);
            if (std::sqrt(dx * dx + dy * dy) < cw * 1.5f) return 30;
        }
        return 0;
    }

    void Tutorial1::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);
        drawTutorialBackground();
        drawTilesFromMap(tileMap);
        if (gridVisible) drawGridLines(32, 20);

        // Teleporter visual: T1 col1-2, row19
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            float cw = (maxX - minX) / 32.0f;
            float ch = (maxY - minY) / 20.0f;
            for (int c : { 1, 2 })
            {
                gfx::Vec2 p{ std::round(minX + c * cw + cw * 0.5f), std::round(minY + 19 * ch + ch * 0.5f) };
                gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
            }
        }

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Tutorial 1");
        printText(-0.95f, 0.8f, 0xFFFFFFFFu, "G: grid   ESC: menu");
        PlayerDraw(gGame.player);
    }

    // ---------------------------------------------------------------
    // Tutorial 2
    // ---------------------------------------------------------------
    Tutorial2::Tutorial2()
    {
        level::loadTileMap("Assets/Levels/tutorial_2.txt", 20, 32, &tileMap[0][0]);
    }

    int (*Tutorial2::getTileMap())[32] { return tileMap; }

    int Tutorial2::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;
        if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

        if (int nav = tutorialNavButtons(2)) return nav;

        PlayerUpdate(gGame.player, dt);
        sprite::updateAnimatedTiles(dt);

        if (!camera::isTransitioning())
        {
            constexpr int gridRows = 20, gridCols = 32;
            float gx{}, gy{}, cw{}, ch{};
            gridToWorld(31, 3, gridCols, gridRows, gx, gy, cw, ch);
            float dx = gGame.player.pos.x - (gx + cw * 0.5f);
            float dy = gGame.player.pos.y - (gy + ch * 0.5f);
            if (std::sqrt(dx * dx + dy * dy) < cw * 1.5f) return 31;
        }
        return 0;
    }

    void Tutorial2::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);
        drawTutorialBackground();
        drawTilesFromMap(tileMap);
        if (gridVisible) drawGridLines(32, 20);

        // Teleporter visual: T2 col31, rows 3-4
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            float cw = (maxX - minX) / 32.0f;
            float ch = (maxY - minY) / 20.0f;
            for (int r : { 3, 4 })
            {
                gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + r * ch + ch * 0.5f) };
                gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
            }
        }

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Tutorial 2");
        PlayerDraw(gGame.player);
    }

    // ---------------------------------------------------------------
    // Tutorial 3
    // ---------------------------------------------------------------
    Tutorial3::Tutorial3()
    {
        level::loadTileMap("Assets/Levels/tutorial_3.txt", 20, 32, &tileMap[0][0]);
    }

    int (*Tutorial3::getTileMap())[32] { return tileMap; }

    int Tutorial3::update(float dt)
    {
        if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;
        if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

        if (int nav = tutorialNavButtons(3)) return nav;

        PlayerUpdate(gGame.player, dt);
        sprite::updateAnimatedTiles(dt);

        if (!camera::isTransitioning())
        {
            constexpr int gridRows = 20, gridCols = 32;
            float gx{}, gy{}, cw{}, ch{};
            gridToWorld(31, 16, gridCols, gridRows, gx, gy, cw, ch);
            float dx = gGame.player.pos.x - (gx + cw * 0.5f);
            float dy = gGame.player.pos.y - (gy + ch * 0.5f);
            if (std::sqrt(dx * dx + dy * dy) < cw * 1.5f) return 32;
        }
        return 0;
    }

    void Tutorial3::draw() const
    {
        AEGfxSetBackgroundColor(0, 0, 0);
        drawTutorialBackground();
        drawTilesFromMap(tileMap);
        if (gridVisible) drawGridLines(32, 20);

        // Teleporter visual: T3 col31, rows 16-18
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            float cw = (maxX - minX) / 32.0f;
            float ch = (maxY - minY) / 20.0f;
            for (int r : { 16, 17, 18 })
            {
                gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + r * ch + ch * 0.5f) };
                gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
            }
        }

        printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Tutorial 3");
        PlayerDraw(gGame.player);
    }

} // namespace game