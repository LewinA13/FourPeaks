#include "scenes/summer.hpp"
#include "AEEngine.h"
#include "engine/graphics.hpp"
#include "gameplay/player.hpp"
#include "core/gamestate.hpp"
#include "engine/sprite.hpp"
#include "engine/camera.hpp"
#include "core/level_loader.hpp"
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include "engine/collision.hpp"

typedef uint32_t u32;
extern s8 gFontId;

namespace game {

    // -------------------------------------------------------------------------
    // Draws special tile types 16-19 that are shared across all summer stages.
    // Returns true if the tile was handled so the caller can skip further checks.
    // -------------------------------------------------------------------------
    static bool drawSpecialTile(int tileType, gfx::Vec2& pos, gfx::Vec2& size)
    {
        // Tile ID mapping:
        // 16 = Tile_02, 17 = Tile_12, 18 = bottle, 19 = sign
        if (tileType == 16) {
            AEGfxTexture* tex = sprite::tile02();
            if (tex) gfx::drawSprite(tex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
            else     gfx::drawRectangle(pos, 0.0f, size, 0xFFFF8800u);
            return true;
        }
        if (tileType == 17) {
            AEGfxTexture* tex = sprite::tile12();
            if (tex) gfx::drawSprite(tex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
            else     gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA44u);
            return true;
        }
        if (tileType == 18) {
            AEGfxTexture* tex = sprite::bottle();
            if (tex) gfx::drawSprite(tex, pos, 0.0f, size, 0.0f, 0.0f, 1.0f, 1.0f);
            else     gfx::drawRectangle(pos, 0.0f, size, 0xFF004488u);
            return true;
        }
        if (tileType == 19) {
            AEGfxTexture* tex = sprite::sign();
            // Draw sign 2 tiles tall, anchored to bottom of tile
            gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.05f };
            gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
            if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
            else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
            return true;
        }
        return false;
    }

    // -------------------------------------------------------------------------
    // Decays the player's heat by a fixed rate each frame.
    // Kills the player once heat reaches zero.
    // -------------------------------------------------------------------------
    void HeatUpdate(float dt)
    {
        gGame.player.heat -= 0.04f * dt;
        if (gGame.player.heat < 0.0f) gGame.player.heat = 0.0f;
        if (gGame.player.heat <= 0.0f && gGame.player.alive)
            PlayerKill(gGame.player);
    }

    // -------------------------------------------------------------------------
    // Renders the heat bar sprite in screen space using the current heat value
    // to select the correct UV frame from the heat bar sprite sheet.
    // Temporarily resets the camera to screen space for correct HUD placement.
    // -------------------------------------------------------------------------
    void HeatDraw()
    {
        float oldCamX, oldCamY;
        AEGfxGetCamPosition(&oldCamX, &oldCamY);
        AEGfxSetCamPosition(0.0f, 0.0f);

        AEGfxTexture* hbTex = sprite::heatbar();
        if (!hbTex) return;

        int frame = (int)((1.0f - gGame.player.heat) * 5.0f);
        if (frame < 0) frame = 0;
        if (frame > 5) frame = 5;

        float u0, v0, u1, v1;
        sprite::getHeatBarUv(frame, u0, v0, u1, v1);

        const float hudCenterX = -700.0f;
        const float hudBottom = 280.0f - 42.0f / 2.0f;
        const float gap = 6.0f;
        gfx::Vec2 hbSize{ 110.0f, 110.0f };
        gfx::Vec2 hbPos{ hudCenterX, hudBottom - gap - hbSize.y / 2.0f };

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        gfx::drawSprite(hbTex, hbPos, 0.0f, hbSize, u0, v0, u1, v1);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        AEGfxSetCamPosition(oldCamX, oldCamY);
    }


    // =========================================================================
    // SummerS1
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 SummerS1::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the stage tile map from disk. Clears the map to zero if loading fails.
    // -------------------------------------------------------------------------
    SummerS1::SummerS1() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s1.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
    }
    SummerS1::~SummerS1() = default;

    // -------------------------------------------------------------------------
    // Reloads the tile map from disk and re-removes any already-collected melons.
    // Called on player death before re-entering the stage.
    // -------------------------------------------------------------------------
    void SummerS1::reset() {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s1.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        if (!loaded)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;

        ApplyCollectedMelonsToTileMap("SummerS1", gridRows, tileMap);
    }

    // -------------------------------------------------------------------------
    // Runs one frame of stage logic: handles respawn reset, player input,
    // heat decay, animated tile updates, and teleport zone detection.
    // Returns 20 to advance to the next stage, or 0 to remain here.
    // -------------------------------------------------------------------------
    int SummerS1::update(float dt) {
        if (gGame.player.justRespawned) {
            reset();
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);
        if (!camera::isTransitioning()) HeatUpdate(dt);

        // Teleport zone: matches visual indicator (col 30-31, row 19)
        {
            float gx, gy, cw, ch;
            gridToWorld(30, 19, gx, gy, cw, ch);
            float zoneLeft = gx;
            float zoneRight = gx + cw * 2.0f;
            float zoneBot = gy;
            float zoneTop = gy + ch;
            if (gGame.player.pos.x >= zoneLeft && gGame.player.pos.x <= zoneRight &&
                gGame.player.pos.y >= zoneBot && gGame.player.pos.y <= zoneTop)
                return 20;
        }
        sprite::updateAnimatedTiles(dt);
        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the summer background, all tiles, the debug grid if enabled,
    // the teleport zone indicator rectangles, the heat bar HUD, and the player.
    // -------------------------------------------------------------------------
    void SummerS1::draw() const {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
        AEGfxTexture* bg = sprite::summerBackground();
        if (bg) {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::drawSprite(bg, { (minX + maxX) * 0.5f,(minY + maxY) * 0.5f }, 0.0f, { maxX - minX,maxY - minY }, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible) drawGrid();

        if (!camera::isTransitioning()) {
            for (int c = 0; c < 2; c++) {
                int col = 30 + c;
                if (col < gridCols) {
                    float gx, gy, cw, ch;
                    gridToWorld(col, 19, gx, gy, cw, ch);
                    gfx::Vec2 pp{ std::round(gx + cw * 0.5f), std::round(gy + ch * 0.5f) };
                    gfx::drawRectangle(pp, 0.0f, { cw,ch }, 0xAAFFFFFFu);
                }
            }
        }
        HeatDraw();
        PlayerDraw(gGame.player, gridVisible);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space centre position and cell
    // dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void SummerS1::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Spike tiles are offset and scaled to avoid visible gaps at their bases.
    // Side-facing spikes (21/22) are drawn with swapped width/height.
    // Border lines are drawn on the exposed edges of solid tiles.
    // -------------------------------------------------------------------------
    void SummerS1::drawTiles() const {
        auto isSolid = [&](int r, int c) -> bool {
            if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
            int t = tileMap[r][c];
            return (t == 1 || t == 3 || t == 5 || t == 7 || t == 16 || t == 17 || t == 23);
            };

        for (int row = 0; row < gridRows; ++row) {
            for (int col = 0; col < gridCols; ++col) {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;
                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);
                gfx::Vec2 pos{ std::round(xWorld + cellW * 0.5f), std::round(yWorld + cellH * 0.5f) };
                gfx::Vec2 size{ cellW + 1.0f, cellH + 1.0f };

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32 borderColor = 0xFF000000;

                if (sprite::drawAnimatedTile(tileType, pos, size)) continue;

                if (tileType == 2) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        gfx::Vec2 ss{ size.x, size.y * 1.5f };
                        gfx::Vec2 sp = pos; sp.y += (ss.y - size.y) * 0.5f;
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, 0.0f, 0.0f, 1.0f, 1.0f);
                    }
                    else gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    continue;
                }

                if (tileType == 9) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        gfx::Vec2 ss{ size.x, size.y * 1.5f };
                        gfx::Vec2 sp = pos; sp.y -= (ss.y - size.y) * 0.5f;
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, 0.0f, 1.0f, 1.0f, 0.0f);
                    }
                    else gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    continue;
                }

                if (tileType == 26 || tileType == 27) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) {
                            sp.x += (ss.x - size.x) * 0.5f;
                        }
                        else {
                            sp.x -= (ss.x - size.x) * 0.5f;
                            u0 = 1.0f;
                            u1 = 0.0f;
                        }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                // Cell-fit left spike (21) and right spike (22): drawn sideways
                // by swapping width/height so the portrait spike lies on its side
                if (tileType == 21 || tileType == 22)
                {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex)
                    {
                        gfx::Vec2 ss{ size.y, size.x };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 22) {
                            u0 = 1.0f;
                            u1 = 0.0f;
                        }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (tileType == 23) {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                else if (drawSpecialTile(tileType, pos, size)) { /* handled */ }
                else if (tileType == 1) {
                    AEGfxTexture* t = sprite::spring1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 3) {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 5) {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 7) {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 4) {
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                }
                else if (tileType == 6) {
                    AEGfxTexture* t = sprite::winterT();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                }
                else {
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

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as a series of thin rectangles aligned to
    // the tile cell boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void SummerS1::drawGrid() const {
        const u32 gc = 0x80FFFFFFu;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols), ch = (maxY - minY) / static_cast<float>(gridRows);
        float t = (cw < ch ? cw : ch) * 0.04f;
        for (int c = 0; c <= gridCols; ++c) gfx::drawRectangle({ minX + c * cw,(minY + maxY) * 0.5f }, 0.0f, { t,maxY - minY }, gc);
        for (int r = 0; r <= gridRows; ++r) gfx::drawRectangle({ (minX + maxX) * 0.5f,minY + r * ch }, 0.0f, { maxX - minX,t }, gc);
    }


    // =========================================================================
    // SummerS2
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 SummerS2::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the stage tile map from disk. Clears the map to zero if loading fails.
    // -------------------------------------------------------------------------
    SummerS2::SummerS2() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s2.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r) for (int c = 0; c < gridCols; ++c) tileMap[r][c] = 0;
    }
    SummerS2::~SummerS2() = default;

    // -------------------------------------------------------------------------
    // Reloads the tile map from disk and re-removes any already-collected melons.
    // Called on player death before re-entering the stage.
    // -------------------------------------------------------------------------
    void SummerS2::reset()
    {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s2.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        if (!loaded)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;

        ApplyCollectedMelonsToTileMap("SummerS2", gridRows, tileMap);
    }

    // -------------------------------------------------------------------------
    // Runs one frame of stage logic: handles respawn reset, player input,
    // heat decay, animated tile updates, and teleport zone detection.
    // Returns 21 to advance to the next stage, or 0 to remain here.
    // -------------------------------------------------------------------------
    int SummerS2::update(float dt) {
        if (gGame.player.justRespawned) {
            reset();
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);
        if (!camera::isTransitioning()) HeatUpdate(dt);

        // Teleport zone: matches visual indicator (col 30-31, row 0)
        {
            float gx, gy, cw, ch;
            gridToWorld(30, 0, gx, gy, cw, ch);
            float zoneLeft = gx;
            float zoneRight = gx + cw * 2.0f;
            float zoneBot = gy;
            float zoneTop = gy + ch;
            if (gGame.player.pos.x >= zoneLeft && gGame.player.pos.x <= zoneRight &&
                gGame.player.pos.y >= zoneBot && gGame.player.pos.y <= zoneTop)
                return 21;
        }
        sprite::updateAnimatedTiles(dt);
        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the summer background, all tiles, the debug grid if enabled,
    // the teleport zone indicator rectangles, the heat bar HUD, and the player.
    // -------------------------------------------------------------------------
    void SummerS2::draw() const {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
        AEGfxTexture* bg = sprite::summerBackground();
        if (bg) {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::drawSprite(bg, { (minX + maxX) * 0.5f,(minY + maxY) * 0.5f }, 0.0f, { maxX - minX,maxY - minY }, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible) drawGrid();

        if (!camera::isTransitioning()) {
            for (int c = 0; c < 2; c++) {
                int col = 30 + c;
                if (col < gridCols) {
                    float gx, gy, cw, ch;
                    gridToWorld(col, 0, gx, gy, cw, ch);
                    gfx::Vec2 pp{ std::round(gx + cw * 0.5f), std::round(gy + ch * 0.5f) };
                    gfx::drawRectangle(pp, 0.0f, { cw,ch }, 0xAAFFFFFFu);
                }
            }
        }
        HeatDraw();
        PlayerDraw(gGame.player, gridVisible);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space centre position and cell
    // dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void SummerS2::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW; yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Vertical spikes (2/9) are sunk ~12% into the tile to close the base gap;
    // V coordinates are mirrored for downward-facing spikes.
    // Border lines are drawn on the exposed edges of solid tiles.
    // -------------------------------------------------------------------------
    void SummerS2::drawTiles() const {
        auto isSolid = [&](int r, int c) -> bool {
            if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
            int t = tileMap[r][c];
            return (t == 1 || t == 3 || t == 5 || t == 7 || t == 16 || t == 17 || t == 23);
            };

        for (int row = 0; row < gridRows; ++row) {
            for (int col = 0; col < gridCols; ++col) {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;
                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);
                gfx::Vec2 pos{ std::round(xWorld + cellW * 0.5f), std::round(yWorld + cellH * 0.5f) };
                gfx::Vec2 size{ cellW, cellH };

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32 borderColor = 0xFF000000;

                if (sprite::drawAnimatedTile(tileType, pos, size)) continue;

                // Vertical spikes: type 2 points up, type 9 points down.
                // Sink the sprite ~12% into the tile to avoid a visible gap
                // at the base. Mirroring V coordinates flips the downward spike.
                if (tileType == 2 || tileType == 9) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        float heightScale = 1.5f;
                        gfx::Vec2 ss{ size.x, size.y * heightScale };
                        gfx::Vec2 sp = pos;
                        if (tileType == 2) {
                            sp.y += (ss.y - size.y) * 0.5f;
                            sp.y -= size.y * 0.12f;
                        }
                        else {
                            sp.y -= (ss.y - size.y) * 0.5f;
                            sp.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) { v0 = 1.0f; v1 = 0.0f; }
                        sp.x = std::round(sp.x);
                        sp.y = std::round(sp.y);
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    else {
                        gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                    }
                    continue;
                }

                if (tileType == 26 || tileType == 27) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (tileType == 23) {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                else if (drawSpecialTile(tileType, pos, size)) { /* handled */ }
                else if (tileType == 1) {
                    AEGfxTexture* t = sprite::spring1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 3) {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 5) {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 7) {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 4) {
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                }
                else if (tileType == 6) {
                    AEGfxTexture* t = sprite::winterT();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                }
                else {
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

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as a series of thin rectangles aligned to
    // the tile cell boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void SummerS2::drawGrid() const {
        const u32 gc = 0x80FFFFFFu;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float sw = maxX - minX, sh = maxY - minY;
        float cw = sw / static_cast<float>(gridCols), ch = sh / static_cast<float>(gridRows);
        float t = (cw < ch ? cw : ch) * 0.04f;
        for (int c = 0; c <= gridCols; ++c) gfx::drawRectangle({ minX + c * cw,(minY + maxY) * 0.5f }, 0.0f, { t,sh }, gc);
        for (int r = 0; r <= gridRows; ++r) gfx::drawRectangle({ (minX + maxX) * 0.5f,minY + r * ch }, 0.0f, { sw,t }, gc);
    }


    // =========================================================================
    // SummerS3
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 SummerS3::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the tile map from disk, then scans it to build the initial iceTiles
    // list for every type-1 (breakable ice) cell found.
    // -------------------------------------------------------------------------
    SummerS3::SummerS3() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s3.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r) for (int c = 0; c < gridCols; ++c) tileMap[r][c] = 0;
        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                if (tileMap[row][col] == 1) { IceTileState ice; ice.row = row; ice.col = col; iceTiles.push_back(ice); }
    }
    SummerS3::~SummerS3() = default;

    // -------------------------------------------------------------------------
    // Reloads the tile map, re-removes collected melons, then clears and
    // rebuilds the iceTiles list from the freshly loaded map.
    // Called on player death before re-entering the stage.
    // -------------------------------------------------------------------------
    void SummerS3::reset()
    {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s3.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        if (!loaded)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;

        ApplyCollectedMelonsToTileMap("SummerS3", gridRows, tileMap);

        iceTiles.clear();

        for (int r = 0; r < gridRows; ++r)
            for (int c = 0; c < gridCols; ++c)
                if (tileMap[r][c] == 1)
                {
                    IceTileState ice;
                    ice.row = r;
                    ice.col = c;
                    iceTiles.push_back(ice);
                }
    }

    // -------------------------------------------------------------------------
    // Runs one frame of stage logic: handles respawn reset, player input, heat
    // decay, teleport zone detection, ice tile crack timers, and tile removal
    // once the final crack frame is reached.
    // Returns 22 to advance to the next stage, or 0 to remain here.
    // -------------------------------------------------------------------------
    int SummerS3::update(float dt) {
        if (gGame.player.justRespawned) {
            reset();
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);
        if (!camera::isTransitioning()) HeatUpdate(dt);

        // Teleport zone: matches visual indicator (col 29-30, row 19)
        {
            float gx, gy, cw, ch;
            gridToWorld(29, 19, gx, gy, cw, ch);
            float zoneLeft = gx;
            float zoneRight = gx + cw * 2.0f;
            float zoneBot = gy;
            float zoneTop = gy + ch;
            if (gGame.player.pos.x >= zoneLeft && gGame.player.pos.x <= zoneRight &&
                gGame.player.pos.y >= zoneBot && gGame.player.pos.y <= zoneTop)
                return 22;
        }

        // Consume the collision system's trigger list and mark matching ice tiles
        for (auto& trigger : g_triggeredIceTiles)
            for (auto& ice : iceTiles)
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered) ice.triggered = true;
        g_triggeredIceTiles.clear();

        // Advance crack animation; destroy tile once the final frame is reached
        for (auto& ice : iceTiles) {
            if (ice.triggered && !ice.destroyed) {
                ice.timer += dt;
                int nf = static_cast<int>(ice.timer / sprite::crackFrameTime);
                if (nf >= sprite::crackFrameCount - 1) { tileMap[ice.row][ice.col] = 0; ice.destroyed = true; }
                else ice.crackFrame = nf;
            }
        }
        sprite::updateAnimatedTiles(dt);
        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the summer background, all tiles (including cracking ice frames),
    // the debug grid if enabled, the teleport zone indicator rectangles,
    // the heat bar HUD, and the player.
    // -------------------------------------------------------------------------
    void SummerS3::draw() const {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
        AEGfxTexture* bg = sprite::summerBackground();
        if (bg) {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::drawSprite(bg, { (minX + maxX) * 0.5f,(minY + maxY) * 0.5f }, 0.0f, { maxX - minX,maxY - minY }, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible) drawGrid();

        if (!camera::isTransitioning()) {
            for (int c = 0; c < 2; c++) {
                int col = 29 + c;
                if (col < gridCols) {
                    float gx, gy, cw, ch;
                    gridToWorld(col, 19, gx, gy, cw, ch);
                    gfx::Vec2 pp{ std::round(gx + cw * 0.5f),std::round(gy + ch * 0.5f) };
                    gfx::drawRectangle(pp, 0.0f, { cw,ch }, 0xAAFFFFFFu);
                }
            }
        }
        HeatDraw();
        PlayerDraw(gGame.player, gridVisible);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space centre position and cell
    // dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void SummerS3::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW; yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Ice tiles (type 1) look up their per-tile crack frame from iceTiles and
    // select the matching UV region from the crack sprite sheet, then skip
    // the border pass since cracking ice is not treated as solid.
    // Vertical spikes are sunk into the tile to close the base gap.
    // Border lines are drawn on the exposed edges of remaining solid tiles.
    // -------------------------------------------------------------------------
    void SummerS3::drawTiles() const {
        auto isSolid = [&](int r, int c) -> bool {
            if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
            int t = tileMap[r][c];
            // type 1 is cracking ice — handled with continue, never reaches borders
            return (t == 3 || t == 5 || t == 7 || t == 16 || t == 17 || t == 23);
            };

        for (int row = 0; row < gridRows; ++row) {
            for (int col = 0; col < gridCols; ++col) {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;
                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);
                gfx::Vec2 pos{ std::round(xWorld + cellW * 0.5f), std::round(yWorld + cellH * 0.5f) };
                gfx::Vec2 size{ cellW, cellH };

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32 borderColor = 0xFF000000;

                // Vertical spikes: type 2 points up, type 9 points down.
                // Sink slightly into the tile to close the gap at the base.
                if (tileType == 2 || tileType == 9) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        float heightScale = 1.5f;
                        gfx::Vec2 ss{ size.x, size.y * heightScale };
                        gfx::Vec2 sp = pos;
                        if (tileType == 2) {
                            sp.y += (ss.y - size.y) * 0.5f;
                            sp.y -= size.y * 0.12f;
                        }
                        else {
                            sp.y -= (ss.y - size.y) * 0.5f;
                            sp.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) { v0 = 1.0f; v1 = 0.0f; }
                        sp.x = std::round(sp.x);
                        sp.y = std::round(sp.y);
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (tileType == 26 || tileType == 27) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (tileType == 23) {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                else if (tileType == 1) {
                    // Cracking ice: look up the per-tile crack frame and select
                    // the matching UV region from the crack sprite sheet
                    AEGfxTexture* crackTex = sprite::crack();
                    if (crackTex) {
                        int frameToUse = 0;
                        for (const auto& ice : iceTiles)
                            if (ice.row == row && ice.col == col) { frameToUse = ice.crackFrame; break; }
                        float u0{}, v0{}, u1{}, v1{};
                        sprite::getCrackUv(frameToUse, u0, v0, u1, v1);
                        gfx::drawSprite(crackTex, pos, 0.0f, size, u0, v0, u1, v1);
                    }
                    continue; // not in isSolid, skip borders
                }
                else if (sprite::drawAnimatedTile(tileType, pos, size)) { continue; }
                else if (drawSpecialTile(tileType, pos, size)) { /* handled */ }
                else if (tileType == 3) {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 5) {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 7) {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 4) {
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                }
                else if (tileType == 6) {
                    AEGfxTexture* t = sprite::winterT();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                }
                else {
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

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as a series of thin rectangles aligned to
    // the tile cell boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void SummerS3::drawGrid() const {
        const u32 gc = 0x80FFFFFFu;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float sw = maxX - minX, sh = maxY - minY;
        float cw = sw / static_cast<float>(gridCols), ch = sh / static_cast<float>(gridRows);
        float t = (cw < ch ? cw : ch) * 0.04f;
        for (int c = 0; c <= gridCols; ++c) gfx::drawRectangle({ minX + c * cw,(minY + maxY) * 0.5f }, 0.0f, { t,sh }, gc);
        for (int r = 0; r <= gridRows; ++r) gfx::drawRectangle({ (minX + maxX) * 0.5f,minY + r * ch }, 0.0f, { sw,t }, gc);
    }


    // =========================================================================
    // SummerS4
    // =========================================================================

    // -------------------------------------------------------------------------
    // Returns the debug-draw fallback colour for a given tile type ID.
    // -------------------------------------------------------------------------
    u32 SummerS4::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    // -------------------------------------------------------------------------
    // Loads the tile map from disk, then scans it to build the initial iceTiles
    // list for every type-1 (breakable ice) cell found.
    // -------------------------------------------------------------------------
    SummerS4::SummerS4() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s4.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r) for (int c = 0; c < gridCols; ++c) tileMap[r][c] = 0;
        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                if (tileMap[row][col] == 1) { IceTileState ice; ice.row = row; ice.col = col; iceTiles.push_back(ice); }
    }
    SummerS4::~SummerS4() = default;

    // -------------------------------------------------------------------------
    // Reloads the tile map, re-removes collected melons, then clears and
    // rebuilds the iceTiles list from the freshly loaded map.
    // Called on player death before re-entering the stage.
    // -------------------------------------------------------------------------
    void SummerS4::reset()
    {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s4.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        if (!loaded)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;

        ApplyCollectedMelonsToTileMap("SummerS4", gridRows, tileMap);

        iceTiles.clear();

        for (int r = 0; r < gridRows; ++r)
            for (int c = 0; c < gridCols; ++c)
                if (tileMap[r][c] == 1)
                {
                    IceTileState ice;
                    ice.row = r;
                    ice.col = c;
                    iceTiles.push_back(ice);
                }
    }

    // -------------------------------------------------------------------------
    // Runs one frame of stage logic: handles respawn reset, player input, heat
    // decay, teleport zone detection, animated tile updates, ice tile crack
    // timers, and tile removal once the final crack frame is reached.
    // Returns 25 to transition to SpringS1, or 0 to remain here.
    // -------------------------------------------------------------------------
    int SummerS4::update(float dt) {
        if (gGame.player.justRespawned)
        {
            reset();
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (!camera::isTransitioning()) {
            PlayerUpdate(gGame.player, dt);
            HeatUpdate(dt);
        }

        // Teleport zone: matches visual indicator (col 31, rows 18-19)
        {
            float gx, gy, cw, ch;
            gridToWorld(31, 18, gx, gy, cw, ch);
            float zoneLeft = gx;
            float zoneRight = gx + cw;
            float zoneBot = gy;
            float zoneTop = gy + ch * 2.0f;
            if (gGame.player.pos.x >= zoneLeft && gGame.player.pos.x <= zoneRight &&
                gGame.player.pos.y >= zoneBot && gGame.player.pos.y <= zoneTop)
                return 25; // -> SpringS1
        }
        sprite::updateAnimatedTiles(dt);

        // Consume the collision system's trigger list and mark matching ice tiles
        for (auto& trigger : g_triggeredIceTiles)
            for (auto& ice : iceTiles)
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered) ice.triggered = true;
        g_triggeredIceTiles.clear();

        // Advance crack animation; destroy tile once the final frame is reached
        for (auto& ice : iceTiles) {
            if (ice.triggered && !ice.destroyed) {
                ice.timer += dt;
                int nf = static_cast<int>(ice.timer / sprite::crackFrameTime);
                if (nf >= sprite::crackFrameCount - 1) { tileMap[ice.row][ice.col] = 0; ice.destroyed = true; }
                else ice.crackFrame = nf;
            }
        }
        return 0;
    }

    // -------------------------------------------------------------------------
    // Renders the summer background, all tiles (including cracking ice frames),
    // the debug grid if enabled, the teleport zone indicator rectangles,
    // the heat bar HUD, and the player.
    // -------------------------------------------------------------------------
    void SummerS4::draw() const {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
        AEGfxTexture* bg = sprite::summerBackground();
        if (bg) {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            gfx::drawSprite(bg, { (minX + maxX) * 0.5f,(minY + maxY) * 0.5f }, 0.0f, { maxX - minX,maxY - minY }, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
        drawTiles();
        if (gridVisible) drawGrid();

        if (!camera::isTransitioning()) {
            // Teleporter visual: column 32, rows 19-20 (zero-based: col 31, rows 18-19)
            for (int r = 18; r <= 19; ++r) {
                float gx, gy, cw, ch;
                gridToWorld(31, r, gx, gy, cw, ch);
                gfx::Vec2 pp{ std::round(gx + cw * 0.5f), std::round(gy + ch * 0.5f) };
                gfx::drawRectangle(pp, 0.0f, { cw, ch }, 0xAAFFFFFFu);
            }
        }
        HeatDraw();
        PlayerDraw(gGame.player, gridVisible);
    }

    // -------------------------------------------------------------------------
    // Converts a tile (col, row) index to world-space centre position and cell
    // dimensions, accounting for the current viewport bounds.
    // -------------------------------------------------------------------------
    void SummerS4::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW; yWorld = minY + row * cellH;
    }

    // -------------------------------------------------------------------------
    // Iterates every tile in the map and draws the appropriate sprite or colour.
    // Ice tiles (type 1) look up their per-tile crack frame from iceTiles and
    // select the matching UV region from the crack sprite sheet, then skip
    // the border pass since cracking ice is not treated as solid.
    // The summer artifact (type 31) bobs vertically using a sine wave offset.
    // Border lines are drawn on the exposed edges of remaining solid tiles.
    // -------------------------------------------------------------------------
    void SummerS4::drawTiles() const {
        auto isSolid = [&](int r, int c) -> bool {
            if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
            int t = tileMap[r][c];
            // type 1 is cracking ice — handled with continue, never reaches borders
            return (t == 3 || t == 5 || t == 7 || t == 16 || t == 17 || t == 23);
            };

        for (int row = 0; row < gridRows; ++row) {
            for (int col = 0; col < gridCols; ++col) {
                int tileType = tileMap[row][col];
                if (tileType <= 0) continue;
                float xWorld{}, yWorld{}, cellW{}, cellH{};
                gridToWorld(col, row, xWorld, yWorld, cellW, cellH);
                gfx::Vec2 pos{ std::round(xWorld + cellW * 0.5f), std::round(yWorld + cellH * 0.5f) };
                gfx::Vec2 size{ cellW, cellH };

                float border = (cellW < cellH ? cellW : cellH) * 0.05f;
                u32 borderColor = 0xFF000000;

                if (tileType == 2 || tileType == 9) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        gfx::Vec2 ss{ size.x, size.y * 1.5f };
                        gfx::Vec2 sp = pos;
                        sp.y += (tileType == 2) ? (ss.y - size.y) * 0.5f : -(ss.y - size.y) * 0.5f;
                        float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
                        if (tileType == 9) { v0 = 1.0f; v1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (tileType == 26 || tileType == 27) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        gfx::Vec2 ss{ size.x * 1.5f, size.y };
                        gfx::Vec2 sp = pos;
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }
                        else { sp.x -= (ss.x - size.x) * 0.5f; u0 = 1.0f; u1 = 0.0f; }
                        gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                    }
                    continue;
                }

                if (tileType == 23) {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                }
                else if (tileType == 1) {
                    // Cracking ice: look up the per-tile crack frame and select
                    // the matching UV region from the crack sprite sheet
                    AEGfxTexture* crackTex = sprite::crack();
                    if (crackTex) {
                        int frameToUse = 0;
                        for (const auto& ice : iceTiles)
                            if (ice.row == row && ice.col == col) { frameToUse = ice.crackFrame; break; }
                        float u0{}, v0{}, u1{}, v1{};
                        sprite::getCrackUv(frameToUse, u0, v0, u1, v1);
                        gfx::drawSprite(crackTex, pos, 0.0f, size, u0, v0, u1, v1);
                    }
                    continue; // not in isSolid, skip borders
                }
                else if (sprite::drawAnimatedTile(tileType, pos, size)) { continue; }
                else if (drawSpecialTile(tileType, pos, size)) { /* handled */ }
                else if (tileType == 31) {
                    // Summer artifact: bob up and down using a sine wave offset
                    AEGfxTexture* summerArtifactsTex = sprite::summerArtifacts();
                    float bobOffset = sinf((float)AEGetTime(nullptr) * 2.0f) * (size.y * 0.08f);
                    gfx::Vec2 artifactsPos{ pos.x, pos.y + bobOffset };
                    gfx::Vec2 artifactsSize{ size.x * 0.9f, size.y * 0.9f };
                    if (summerArtifactsTex) gfx::drawSprite(summerArtifactsTex, artifactsPos, 0.0f, artifactsSize, 0, 0, 1, 1);
                    else   gfx::drawRectangle(artifactsPos, 0.0f, size, 0xFF00AA00u);
                }
                else if (tileType == 3) {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 5) {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 7) {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColor(tileType));
                }
                else if (tileType == 4) {
                    AEGfxTexture* t = sprite::winterC();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF808080u);
                }
                else if (tileType == 6) {
                    AEGfxTexture* t = sprite::winterT();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF555555u);
                }
                else {
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

    // -------------------------------------------------------------------------
    // Draws the debug grid overlay as a series of thin rectangles aligned to
    // the tile cell boundaries across the full viewport.
    // -------------------------------------------------------------------------
    void SummerS4::drawGrid() const {
        const u32 gc = 0x80FFFFFFu;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float sw = maxX - minX, sh = maxY - minY;
        float cw = sw / static_cast<float>(gridCols), ch = sh / static_cast<float>(gridRows);
        float t = (cw < ch ? cw : ch) * 0.04f;
        for (int c = 0; c <= gridCols; ++c) gfx::drawRectangle({ minX + c * cw,(minY + maxY) * 0.5f }, 0.0f, { t,sh }, gc);
        for (int r = 0; r <= gridRows; ++r) gfx::drawRectangle({ (minX + maxX) * 0.5f,minY + r * ch }, 0.0f, { sw,t }, gc);
    }

} // namespace game