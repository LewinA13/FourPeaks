#include "autumn.hpp"

#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include "gamestate.hpp"
#include "sprite.hpp"
#include "level_loader.hpp"
#include "camera.hpp"

#include <cstdint>
#include <cmath>

namespace
{
    using u32 = std::uint32_t;

    static void drawBackground()
    {
        // Re-use an existing background if you don't have a dedicated Autumn BG yet.
        // Change this later to sprite::springBackground() when you add it.
        AEGfxTexture* bg = sprite::autumnBackground();
        if (!bg) bg = sprite::background();

        if (bg)
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::drawSprite(bg, { (minX + maxX) * 0.5f, (minY + maxY) * 0.5f }, 0.0f,
                { (maxX - minX), (maxY - minY) }, 0, 0, 1, 1);
        }
    }

    static void gridToWorldCommon(int col, int row, int gridCols, int gridRows,
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
            gfx::drawRectangle({ x, (minY + maxY) * 0.5f }, 0.0f, { thickness, maxY - minY }, gridColor);
        }

        for (int row = 0; row <= gridRows; ++row)
        {
            float y = minY + row * cellH;
            gfx::drawRectangle({ (minX + maxX) * 0.5f, y }, 0.0f, { maxX - minX, thickness }, gridColor);
        }
    }

    static u32 getTileColorCommon(int tileType)
    {
        switch (tileType)
        {
        case 6: return 0xFF555555u; // underground
        case 7: return 0xFF888888u; // top ground / platform
        case 1: return 0xFF66AAFFu; // legacy
        case 2: return 0xFFFF3333u; // spikes
        case 8: return 0xFFFFFF00u; // pickup (coin/bottle depending on your build)
        case 10: return 0xFF00FF00u; // checkpoint
        default: return 0x00000000u;
        }
    }

    static void drawTilesCommon(int gridRows, int gridCols, int tileMap[][32], float stageBaseY = 0.0f)
    {
        auto isSolid = [&](int r, int c) -> bool {
            if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
            int t = tileMap[r][c];
            return (t == 1 || t == 3 || t == 5 || t == 6 || t == 7);
            };

        for (int r = 0; r < gridRows; ++r)
        {
            for (int c = 0; c < gridCols; ++c)
            {
                int tileType = tileMap[r][c];
                if (tileType == 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorldCommon(c, r, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
                yWorld += stageBaseY;

                gfx::Vec2 pos{ std::round(xWorld + cellW * 0.5f), std::round(yWorld + cellH * 0.5f) };
                gfx::Vec2 size{ cellW, cellH };

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32 borderColor = 0xFF000000;

                if (sprite::drawAnimatedTile(tileType, pos, size)) continue;

                if (tileType == 23) {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                    // falls through to border pass
                }
                else if (tileType == 2 || tileType == 9 || tileType == 26 || tileType == 27)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        float heightScale = 1.5f;
                        if (tileType == 26 || tileType == 27)
                        {
                            gfx::Vec2 spikeSize{ size.x * heightScale, size.y };
                            gfx::Vec2 spikePos = pos;
                            float u0, v0, u1, v1;
                            if (tileType == 27) { u0 = 1.0f; v0 = 0.0f; u1 = 0.0f; v1 = 1.0f; spikePos.x -= (spikeSize.x - size.x) * 0.5f; }
                            else { u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f; spikePos.x += (spikeSize.x - size.x) * 0.5f; }
                            gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                        }
                        else
                        {
                            gfx::Vec2 spikeSize{ size.x, size.y * heightScale };
                            gfx::Vec2 spikePos = pos;
                            float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                            if (tileType == 2) { spikePos.y += (spikeSize.y - size.y) * 0.5f; }
                            else { spikePos.y -= (spikeSize.y - size.y) * 0.5f; v0 = 1.0f; v1 = 0.0f; }
                            gfx::drawSprite(spikeTex, spikePos, 0.0f, spikeSize, u0, v0, u1, v1);
                        }
                    }
                    continue; // spikes are never solid
                }
                else if (tileType == 1) {
                    AEGfxTexture* t = sprite::spring1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else if (tileType == 3) {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else if (tileType == 5) {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else if (tileType == 7) {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else {
                    float u0{}, v0{}, u1{}, v1{};
                    if (sprite::getTileUv(tileType, u0, v0, u1, v1))
                        gfx::drawSprite(sprite::tileset(), pos, 0.0f, size, u0, v0, u1, v1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }

                // ---- Borders (solid tiles only) ----
                if (!isSolid(r, c)) continue;

                if (!isSolid(r + 1, c)) {
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(r - 1, c)) {
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(r, c - 1)) {
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
                if (!isSolid(r, c + 1)) {
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
            }
        }
    }
}

// -------------------------------------------------------------------
// AutumnS1
// action codes:
// 60 -> AutumnS2
// 2  -> MainMenu
// -------------------------------------------------------------------
game::AutumnS1::AutumnS1()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s1.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::AutumnS1::~AutumnS1() = default;

int game::AutumnS1::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
    if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

    // Transition trigger (you can change this logic later):
    // UP key or reaching a small trigger near (30,19)
    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(0.0f, camera::screenHeight(), 0.3f);
            return 60;
        }
    }

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    if (!camera::isTransitioning())
    {
        float gx{}, gy{}, cw{}, ch{};
        gridToWorld(30, 19, gx, gy, cw, ch);
        float dx = gGame.player.pos.x - (gx + cw * 0.5f);
        float dy = gGame.player.pos.y - (gy + ch * 0.5f);
        if (sqrt(dx * dx + dy * dy) < cw * 1.5f) return 60;
    }

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::AutumnS1::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();
    PlayerDraw(gGame.player);
}

u32 game::AutumnS1::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::AutumnS1::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::AutumnS1::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::AutumnS1::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }

// -------------------------------------------------------------------
// AutumnS2
// 61 -> AutumnS3
// -------------------------------------------------------------------
game::AutumnS2::AutumnS2()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s2.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::AutumnS2::~AutumnS2() = default;

int game::AutumnS2::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
    if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(camera::screenHeight(), camera::screenHeight() * 2.0f, 0.3f);
            return 61;
        }
    }

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::AutumnS2::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();
    PlayerDraw(gGame.player);
}

u32 game::AutumnS2::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::AutumnS2::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::AutumnS2::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::AutumnS2::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }

// -------------------------------------------------------------------
// AutumnS3
// 62 -> AutumnS4
// -------------------------------------------------------------------
game::AutumnS3::AutumnS3()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s3.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::AutumnS3::~AutumnS3() = default;

int game::AutumnS3::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
    if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(camera::screenHeight() * 2.0f, camera::screenHeight() * 3.0f, 0.3f);
            return 62;
        }
    }

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::AutumnS3::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();
    PlayerDraw(gGame.player);
}

u32 game::AutumnS3::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::AutumnS3::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::AutumnS3::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::AutumnS3::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }

// -------------------------------------------------------------------
// AutumnS4 (end of Autumn - returns 2 to go back to menu by default)
// -------------------------------------------------------------------
game::AutumnS4::AutumnS4()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s4.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::AutumnS4::~AutumnS4() = default;

int game::AutumnS4::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
    if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::AutumnS4::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();
    PlayerDraw(gGame.player);
}

u32 game::AutumnS4::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::AutumnS4::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::AutumnS4::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::AutumnS4::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }
