#include "spring.hpp"

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
        AEGfxTexture* bg = sprite::springBackground();
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
        for (int r = 0; r < gridRows; ++r)
        {
            for (int c = 0; c < gridCols; ++c)
            {
                int tileType = tileMap[r][c];
                if (tileType == 0) continue;

                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorldCommon(c, r, gridCols, gridRows, xWorld, yWorld, cellW, cellH);

                // If you ever stack stages vertically, you can pass stageBaseY in.
                yWorld += stageBaseY;

                gfx::Vec2 pos{ xWorld + cellW * 0.5f, yWorld + cellH * 0.5f };
                gfx::Vec2 size{ cellW, cellH };

                // Animated tiles first (coin/checkpoint/fire/saw).
                if (sprite::drawAnimatedTile(tileType, pos, size))
                    continue;

                // Spikes - taller than cell, anchored correctly
                if (tileType == 2 || tileType == 9)
                {
                    AEGfxTexture* s = sprite::spikes();
                    if (s)
                    {
                        gfx::Vec2 ss{ size.x, size.y * 1.5f };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 2) { sp.y += (ss.y - size.y) * 0.5f; }
                        else { sp.y -= (ss.y - size.y) * 0.5f; v0 = 1.0f; v1 = 0.0f; }
                        gfx::drawSprite(s, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    else gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                    continue;
                }

                // Left-facing spike (26) and right-facing spike (27)
                if (tileType == 26 || tileType == 27)
                {
                    AEGfxTexture* s = sprite::spikes();
                    if (s)
                    {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(s, sp, 0.0f, ss, u0, v0, u1, v1);
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
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                    continue;
                }
                if (tileType == 3)
                {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                    continue;
                }
                if (tileType == 5)
                {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                    continue;
                }
                if (tileType == 7)
                {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                    continue;
                }

                // WinterC (ID 4) and WinterT (ID 6) standalone textures
                if (tileType == 4)
                {
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                    continue;
                }
                if (tileType == 6)
                {
                    AEGfxTexture* t = sprite::winterT();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                    continue;
                }

                // Regular tiles from tileset.
                float u0{}, v0{}, u1{}, v1{};
                if (sprite::getTileUv(tileType, u0, v0, u1, v1))
                {
                    gfx::drawSprite(sprite::tileset(), pos, 0.0f, size, u0, v0, u1, v1);
                }
                else
                {
                    gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
            }
        }
    }
}

// -------------------------------------------------------------------
// SpringS1
// action codes:
// 40 -> SpringS2
// 2  -> MainMenu
// -------------------------------------------------------------------
game::SpringS1::SpringS1()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s1.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS1::~SpringS1() = default;

int game::SpringS1::update(float dt)
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
            return 40;
        }
    }

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    if (!camera::isTransitioning())
    {
        float gx{}, gy{}, cw{}, ch{};
        gridToWorld(0, 16, gx, gy, cw, ch);
        float dx = gGame.player.pos.x - (gx + cw * 0.5f);
        float dy = gGame.player.pos.y - (gy + ch * 0.5f);
        if (std::sqrt(dx * dx + dy * dy) < cw * 1.5f) return 40;
    }

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::SpringS1::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    // Teleporter visual: S1: col0 row16-18
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + 16 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + 17 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + 18 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
    }

    PlayerDraw(gGame.player);
}

u32 game::SpringS1::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::SpringS1::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::SpringS1::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::SpringS1::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }

// -------------------------------------------------------------------
// SpringS2
// 41 -> SpringS3
// -------------------------------------------------------------------
game::SpringS2::SpringS2()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s2.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS2::~SpringS2() = default;

int game::SpringS2::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
    if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(camera::screenHeight(), camera::screenHeight() * 2.0f, 0.3f);
            return 41;
        }
    }

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    if (!camera::isTransitioning())
    {
        float gx{}, gy{}, cw{}, ch{};
        gridToWorld(31, 17, gx, gy, cw, ch);
        float dx = gGame.player.pos.x - (gx + cw * 0.5f);
        float dy = gGame.player.pos.y - (gy + ch * 0.5f);
        if (std::sqrt(dx * dx + dy * dy) < cw * 1.5f) return 41;
    }

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::SpringS2::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    // Teleporter visual: S2: col31 row17-19
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + 17 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + 18 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + 19 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
    }

    PlayerDraw(gGame.player);
}

u32 game::SpringS2::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::SpringS2::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::SpringS2::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::SpringS2::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }

// -------------------------------------------------------------------
// SpringS3
// 42 -> SpringS4
// -------------------------------------------------------------------
game::SpringS3::SpringS3()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s3.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS3::~SpringS3() = default;

int game::SpringS3::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
    if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(camera::screenHeight() * 2.0f, camera::screenHeight() * 3.0f, 0.3f);
            return 42;
        }
    }

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    if (!camera::isTransitioning())
    {
        float gx{}, gy{}, cw{}, ch{};
        gridToWorld(0, 17, gx, gy, cw, ch);
        float dx = gGame.player.pos.x - (gx + cw * 0.5f);
        float dy = gGame.player.pos.y - (gy + ch * 0.5f);
        if (std::sqrt(dx * dx + dy * dy) < cw * 1.5f) return 42;
    }

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::SpringS3::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    // Teleporter visual: S3: col0 row17-19
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + 17 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + 18 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + 19 * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
    }

    PlayerDraw(gGame.player);
}

u32 game::SpringS3::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::SpringS3::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::SpringS3::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::SpringS3::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }

// -------------------------------------------------------------------
// SpringS4 (end of Spring - returns 2 to go back to menu by default)
// -------------------------------------------------------------------
game::SpringS4::SpringS4()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s4.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS4::~SpringS4() = default;

int game::SpringS4::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
    if (AEInputCheckTriggered(AEVK_ESCAPE)) return 2;

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    // Teleporter to Autumn Stage 1 (last level of Spring)
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
			return 43; // autumn stage 1
        }
    }

    sprite::updateAnimatedTiles(dt);
    return 0;
}

void game::SpringS4::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    // Draw teleporter indicator (2x1 cells, cyan)
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

u32 game::SpringS4::getTileColor(int tileType) const { return getTileColorCommon(tileType); }
void game::SpringS4::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xWorld, yWorld, cellW, cellH);
}
void game::SpringS4::drawGrid() const { drawGridLines(gridCols, gridRows); }
void game::SpringS4::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }