// ---------------------------------------------------------------------------
// summer.cpp - Combined Summer Stages (Stage 1..4)
// ---------------------------------------------------------------------------
#include "summer.hpp"
#include "AEEngine.h"
#include "graphics.hpp"
#include "player.hpp"
#include "gamestate.hpp"
#include "sprite.hpp"
#include "camera.hpp"
#include "level_loader.hpp"
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include "collision.hpp"

typedef uint32_t u32;
extern s8 gFontId;

namespace game {

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
    // HELPER: Draw tiles 16-19 (shared logic, inlined per stage)
    // ===================================================================
    static bool drawSpecialTile(int tileType, gfx::Vec2& pos, gfx::Vec2& size)
    {
        // Tile ID mapping (as per your request):
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


    // -------------------------------------------------------------------
    // Heat for summer
    // -------------------------------------------------------------------
    void HeatUpdate(float dt)
    {
        gGame.player.heat -= 0.04f * dt;
        if (gGame.player.heat < 0.0f) gGame.player.heat = 0.0f;
        if (gGame.player.heat <= 0.0f && gGame.player.alive)
            PlayerKill(gGame.player);
    }

    void HeatDraw()
    {
        AEGfxTexture* hbTex = sprite::heatbar();
        if (!hbTex) return;

        int frame = (int)((1.0f - gGame.player.heat) * 5.0f);  // was 11.0f
        if (frame < 0) frame = 0;
        if (frame > 5) frame = 5;

        float u0, v0, u1, v1;
        sprite::getHeatBarUv(frame, u0, v0, u1, v1);

        float maxX = AEGfxGetWinMaxX();
        float maxY = AEGfxGetWinMaxY();
        gfx::Vec2 hbPos{ maxX - 55.0f, maxY - 55.0f };
        gfx::Vec2 hbSize{ 90.0f, 90.0f };

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        gfx::drawSprite(hbTex, hbPos, 0.0f, hbSize, u0, v0, u1, v1);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
    }


    // ===================================================================
    // SUMMER STAGE 1
    // ===================================================================
    u32 SummerS1::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    SummerS1::SummerS1() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s1.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r)
                for (int c = 0; c < gridCols; ++c)
                    tileMap[r][c] = 0;
    }
    SummerS1::~SummerS1() = default;

    void SummerS1::reset() {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s1.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        // Reload the original map; if loading fails, clear it safely
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
            {
                for (int c = 0; c < gridCols; ++c)
                {
                    tileMap[r][c] = 0;
                }
            }
        }

        // Re-remove melons that were already collected and saved
        ApplyCollectedMelonsToTileMap("SummerS1", gridRows, tileMap);
    }

    int SummerS1::update(float dt) {
        if (gGame.player.justRespawned) {
            // basically reset the bottles
            reset();
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (AEInputCheckTriggered(AEVK_UP)) {
            if (!camera::isTransitioning()) {
                camera::startTransitionY(0.0f, camera::screenHeight(), 0.3f);
                return 20;
            }
        }
        if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);
        if (!camera::isTransitioning()) {
            float gx, gy, cw, ch;
            gridToWorld(30, 19, gx, gy, cw, ch);
            float dx = gGame.player.pos.x - (gx + cw * 0.5f);
            float dy = gGame.player.pos.y - (gy + ch * 0.5f);
            if (sqrt(dx * dx + dy * dy) < cw * 1.5f) return 20;

            // Back-teleport to Winter Stage 4 (at col 1-2, row 2)
            float bx, by, bcw, bch;
            gridToWorld(30, 19, bx, by, bcw, bch);
            float bdx = gGame.player.pos.x - (bx + bcw * 0.5f);
            float bdy = gGame.player.pos.y - (by + bch * 0.5f);
            if (sqrt(bdx * bdx + bdy * bdy) < bcw * 1.5f) return 24; // back to WinterS4

            HeatUpdate(dt);
        }
        sprite::updateAnimatedTiles(dt);
        return 0;
    }

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

        //printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 1 - 32x20 Grid");
        //printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        //printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        if (!camera::isTransitioning()) {
            for (int c = 0; c < 2; c++) {
                int col = 30 + c;
                if (col < gridCols) {
                    float gx, gy, cw, ch;
                    gridToWorld(col, 19, gx, gy, cw, ch);
                    gfx::Vec2 pp{ std::round(gx + cw * 0.5f), std::round(gy + ch * 0.5f) };
                    gfx::drawRectangle(pp, 0.0f, { cw,ch }, 0xAA00FFFFu);
                }
            }
            // Draw back-teleport to WinterS4 indicator (col 1-2, row 2)
            for (int c = 0; c < 2; c++) {
                float gx, gy, cw, ch;
                gridToWorld(30 + c, 19, gx, gy, cw, ch);
                gfx::Vec2 pp{ std::round(gx + cw * 0.5f), std::round(gy + ch * 0.5f) };
                gfx::drawRectangle(pp, 0.0f, { cw,ch }, 0xAA4488FFu); // Blue = leads to Winter
            }
        }
        HeatDraw();
        PlayerDraw(gGame.player);
    }

    void SummerS1::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW;
        yWorld = minY + row * cellH;
    }

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
                        if (tileType == 26) { sp.x += (ss.x - size.x) * 0.5f; }

                        // Cell-fit left spike (21) and right spike (22)
                        else if (tileType == 21 || tileType == 22)
                        {
                            AEGfxTexture* spikeTex = sprite::spikes();
                            if (spikeTex)
                            {
                                // Draw sideways: swap width/height so portrait spike lies on side
                                gfx::Vec2 ss{ size.y, size.x };
                                gfx::Vec2 sp = pos;
                                float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                                if (tileType == 21) { v0 = 0.0f; v1 = 1.0f; }       // left: spike points right
                                else { u0 = 1.0f; u1 = 0.0f; }                       // right: mirrored, points left
                                gfx::drawSprite(spikeTex, sp, 0.0f, ss, u0, v0, u1, v1);
                            }
                        }
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
                else if (drawSpecialTile(tileType, pos, size)) { /* drawn */ }
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

                if (!isSolid(row + 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row - 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row, col - 1)) {
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
                if (!isSolid(row, col + 1)) {
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
            }
        }
    }

    void SummerS1::drawGrid() const {
        const u32 gc = 0x80FFFFFFu;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols), ch = (maxY - minY) / static_cast<float>(gridRows);
        float t = (cw < ch ? cw : ch) * 0.04f;
        for (int c = 0; c <= gridCols; ++c) gfx::drawRectangle({ minX + c * cw,(minY + maxY) * 0.5f }, 0.0f, { t,maxY - minY }, gc);
        for (int r = 0; r <= gridRows; ++r) gfx::drawRectangle({ (minX + maxX) * 0.5f,minY + r * ch }, 0.0f, { maxX - minX,t }, gc);
    }

    // ===================================================================
    // SUMMER STAGE 2
    // ===================================================================
    u32 SummerS2::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    SummerS2::SummerS2() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s2.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r) for (int c = 0; c < gridCols; ++c) tileMap[r][c] = 0;
    }
    SummerS2::~SummerS2() = default;

    void SummerS2::reset()
    {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s2.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        // Reload the original map. if loading fails, clear it safely
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
            {
                for (int c = 0; c < gridCols; ++c)
                {
                    tileMap[r][c] = 0;
                }
            }
        }

        // Re-remove melons that were already collected and saved.
        ApplyCollectedMelonsToTileMap("SummerS2", gridRows, tileMap);
    }

    int SummerS2::update(float dt) {
        if (gGame.player.justRespawned) {
            reset();
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);
        if (!camera::isTransitioning()) {
            float gx, gy, cw, ch;
            gridToWorld(30, 0, gx, gy, cw, ch);
            float dx = gGame.player.pos.x - (gx + cw * 0.5f);
            float dy = gGame.player.pos.y - (gy + ch * 1.0f);
            if (sqrt(dx * dx + dy * dy) < cw * 1.5f) return 21;
            HeatUpdate(dt);
        }
        sprite::updateAnimatedTiles(dt);
        return 0;
    }

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

        //printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 2 - 32x20 Grid");
        //printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        //printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        if (!camera::isTransitioning()) {
            for (int c = 0; c < 2; c++) {
                int col = 30 + c;
                if (col < gridCols) {
                    float gx, gy, cw, ch;
                    gridToWorld(col, 0, gx, gy, cw, ch);
                    gfx::Vec2 pp{ std::round(gx + cw * 0.5f), std::round(gy + ch * 0.5f) };
                    gfx::drawRectangle(pp, 0.0f, { cw,ch }, 0xAA00FFFFu);
                }
            }
        }
        HeatDraw();
        PlayerDraw(gGame.player);
    }

    void SummerS2::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW; yWorld = minY + row * cellH;
    }

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

                // Vertical spikes. Type 2 points up; type 9 points down. Use a
                // slight offset to sink the sprite into the tile by ~12% of
                // its height. Without this offset the spike base would float
                // slightly above the tile, leaving a visible gap. Mirroring
                // the V coordinates flips the sprite for downward spikes.
                if (tileType == 2 || tileType == 9) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        float heightScale = 1.5f;
                        gfx::Vec2 ss{ size.x, size.y * heightScale };
                        gfx::Vec2 sp = pos;
                        if (tileType == 2) {
                            // Up-facing: anchor base to cell bottom then sink
                            sp.y += (ss.y - size.y) * 0.5f;
                            sp.y -= size.y * 0.12f;
                        }
                        else {
                            // Down-facing: anchor base to cell top then sink
                            sp.y -= (ss.y - size.y) * 0.5f;
                            sp.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) {
                            v0 = 1.0f; v1 = 0.0f;
                        }
                        // Round to avoid sub-pixel gaps
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
                else if (drawSpecialTile(tileType, pos, size)) { /* drawn */ }
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

                if (!isSolid(row + 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row - 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row, col - 1)) {
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
                if (!isSolid(row, col + 1)) {
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
            }
        }
    }

    void SummerS2::drawGrid() const {
        const u32 gc = 0x80FFFFFFu;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float sw = maxX - minX, sh = maxY - minY;
        float cw = sw / static_cast<float>(gridCols), ch = sh / static_cast<float>(gridRows);
        float t = (cw < ch ? cw : ch) * 0.04f;
        for (int c = 0; c <= gridCols; ++c) gfx::drawRectangle({ minX + c * cw,(minY + maxY) * 0.5f }, 0.0f, { t,sh }, gc);
        for (int r = 0; r <= gridRows; ++r) gfx::drawRectangle({ (minX + maxX) * 0.5f,minY + r * ch }, 0.0f, { sw,t }, gc);
    }

    // ===================================================================
    // SUMMER STAGE 3
    // ===================================================================
    u32 SummerS3::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    SummerS3::SummerS3() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s3.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r) for (int c = 0; c < gridCols; ++c) tileMap[r][c] = 0;
        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                if (tileMap[row][col] == 1) { IceTileState ice; ice.row = row; ice.col = col; iceTiles.push_back(ice); }
    }
    SummerS3::~SummerS3() = default;

    void SummerS3::reset()
    {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s3.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        // Reload the original map; if loading fails, clear it safely.
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
            {
                for (int c = 0; c < gridCols; ++c)
                {
                    tileMap[r][c] = 0;
                }
            }
        }

        // Re-remove melons that were already collected and saved.
        ApplyCollectedMelonsToTileMap("SummerS3", gridRows, tileMap);

        iceTiles.clear();

        // Rebuild breakable ice data from the updated tile map.
        for (int r = 0; r < gridRows; ++r)
        {
            for (int c = 0; c < gridCols; ++c)
            {
                if (tileMap[r][c] == 1)
                {
                    IceTileState ice;
                    ice.row = r;
                    ice.col = c;
                    iceTiles.push_back(ice);
                }
            }
        }
    }

    int SummerS3::update(float dt) {
        if (gGame.player.justRespawned) {
            reset(); // reset the bottle
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);
        if (!camera::isTransitioning()) {
            float gx, gy, cw, ch;
            gridToWorld(29, 19, gx, gy, cw, ch);
            float dx = gGame.player.pos.x - (gx + cw * 1.0f);
            float dy = gGame.player.pos.y - (gy + ch * 0.5f);
            if (sqrt(dx * dx + dy * dy) < cw * 1.5f) return 22;
            HeatUpdate(dt);
        }
        for (auto& trigger : g_triggeredIceTiles)
            for (auto& ice : iceTiles)
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered) ice.triggered = true;
        g_triggeredIceTiles.clear();
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

        //printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 3 - 32x20 Grid");
        //printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        //printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        if (!camera::isTransitioning()) {
            for (int c = 0; c < 2; c++) {
                int col = 29 + c;
                if (col < gridCols) {
                    float gx, gy, cw, ch;
                    gridToWorld(col, 19, gx, gy, cw, ch);
                    gfx::Vec2 pp{ std::round(gx + cw * 0.5f),std::round(gy + ch * 0.5f) };
                    gfx::drawRectangle(pp, 0.0f, { cw,ch }, 0xAA00FFFFu);
                }
            }
        }
        HeatDraw();
        PlayerDraw(gGame.player);
    }

    void SummerS3::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW; yWorld = minY + row * cellH;
    }

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

                // Vertical spikes. Type 2 points up; type 9 points down. Sink
                // them slightly into the tile so there is no visible gap.
                if (tileType == 2 || tileType == 9) {
                    AEGfxTexture* spikeTex = sprite::spikes();
                    if (spikeTex) {
                        float heightScale = 1.5f;
                        gfx::Vec2 ss{ size.x, size.y * heightScale };
                        gfx::Vec2 sp = pos;
                        if (tileType == 2) {
                            // Up-facing: anchor base then sink by ~12%
                            sp.y += (ss.y - size.y) * 0.5f;
                            sp.y -= size.y * 0.12f;
                        }
                        else {
                            // Down-facing: anchor base then sink by ~12%
                            sp.y -= (ss.y - size.y) * 0.5f;
                            sp.y += size.y * 0.12f;
                        }
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        if (tileType == 9) {
                            v0 = 1.0f; v1 = 0.0f;
                        }
                        // Round to integral pixels to avoid sub-pixel gaps
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
                    // falls through to border pass (grass IS solid)
                }
                else if (tileType == 1) {
                    // Cracking ice — per-tile frame
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
                else if (drawSpecialTile(tileType, pos, size)) { /* drawn */ }
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

                if (!isSolid(row + 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row - 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row, col - 1)) {
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
                if (!isSolid(row, col + 1)) {
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
            }
        }
    }
    // SummerS4::drawTiles() is identical to S3 above — same body, different class name

    void SummerS3::drawGrid() const {
        const u32 gc = 0x80FFFFFFu;
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float sw = maxX - minX, sh = maxY - minY;
        float cw = sw / static_cast<float>(gridCols), ch = sh / static_cast<float>(gridRows);
        float t = (cw < ch ? cw : ch) * 0.04f;
        for (int c = 0; c <= gridCols; ++c) gfx::drawRectangle({ minX + c * cw,(minY + maxY) * 0.5f }, 0.0f, { t,sh }, gc);
        for (int r = 0; r <= gridRows; ++r) gfx::drawRectangle({ (minX + maxX) * 0.5f,minY + r * ch }, 0.0f, { sw,t }, gc);
    }

    // ===================================================================
    // SUMMER STAGE 4
    // ===================================================================
    u32 SummerS4::getTileColor(int tileType) const {
        switch (tileType) {
        case 1:  return 0xFF224B94u;
        case 2:  return 0xFFA3B013u;
        case 3:  return 0xFF808080u;
        default: return 0x00000000u;
        }
    }

    SummerS4::SummerS4() : gridVisible(false), tileMap{} {
        const bool loaded = level::loadTileMap("Assets/Levels/summer_s4.txt", gridRows, gridCols, &tileMap[0][0]);
        if (!loaded)
            for (int r = 0; r < gridRows; ++r) for (int c = 0; c < gridCols; ++c) tileMap[r][c] = 0;
        for (int row = 0; row < gridRows; ++row)
            for (int col = 0; col < gridCols; ++col)
                if (tileMap[row][col] == 1) { IceTileState ice; ice.row = row; ice.col = col; iceTiles.push_back(ice); }
    }
    SummerS4::~SummerS4() = default;

    void SummerS4::reset()
    {
        const bool loaded =
            level::loadTileMap("Assets/Levels/summer_s4.txt",
                gridRows, gridCols,
                &tileMap[0][0]);

        // Reload the original map; if loading fails, clear it safely
        if (!loaded)
        {
            for (int r = 0; r < gridRows; ++r)
            {
                for (int c = 0; c < gridCols; ++c)
                {
                    tileMap[r][c] = 0;
                }
            }
        }

        // Re-remove melons that were already collected and saved
        ApplyCollectedMelonsToTileMap("SummerS4", gridRows, tileMap);

        iceTiles.clear();

        // Rebuild breakable ice data from the updated tile map
        for (int r = 0; r < gridRows; ++r)
        {
            for (int c = 0; c < gridCols; ++c)
            {
                if (tileMap[r][c] == 1)
                {
                    IceTileState ice;
                    ice.row = r;
                    ice.col = c;
                    iceTiles.push_back(ice);
                }
            }
        }
    }


    int SummerS4::update(float dt) {
        if (gGame.player.justRespawned)
        {
            reset(); // reset the spawning of bottle
            gGame.player.justRespawned = false;
        }
        if (AEInputCheckTriggered(AEVK_G)) gridVisible = !gridVisible;
        if (!camera::isTransitioning()) {
            PlayerUpdate(gGame.player, dt);
            HeatUpdate(dt);
        }
        // Teleporter to SpringS1 at col31 row18-19
        if (!camera::isTransitioning())
        {
            float gx{}, gy{}, cw{}, ch{};
            gridToWorld(31, 18, gx, gy, cw, ch);
            float dx = gGame.player.pos.x - (gx + cw * 0.5f);
            float dy = gGame.player.pos.y - (gy + ch * 0.5f);
            if (std::sqrt(dx * dx + dy * dy) < cw * 1.5f) return 25; // -> SpringS1
        }
        sprite::updateAnimatedTiles(dt);
        for (auto& trigger : g_triggeredIceTiles)
            for (auto& ice : iceTiles)
                if (ice.row == trigger.row && ice.col == trigger.col && !ice.triggered) ice.triggered = true;
        g_triggeredIceTiles.clear();
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

        //printText(-0.95f, 0.9f, 0xFFFFFFFFu, "Summer Stage 4 - 32x20 Grid");
        //printText(-0.95f, 0.7f, 0xFFFFFFFFu, "Press G to toggle grid");
        //printText(-0.95f, 0.5f, 0xFFFFFFFFu, "Press ESC to return to menu");

        HeatDraw();
        // Teleporter visual: S4: col31 row18-19
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            float cw = (maxX - minX) / static_cast<float>(gridCols);
            float ch = (maxY - minY) / static_cast<float>(gridRows);
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

    void SummerS4::gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX(), minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        cellW = (maxX - minX) / static_cast<float>(gridCols);
        cellH = (maxY - minY) / static_cast<float>(gridRows);
        xWorld = minX + col * cellW; yWorld = minY + row * cellH;
    }

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
                    // falls through to border pass (grass IS solid)
                }
                else if (tileType == 1) {
                    // Cracking ice — per-tile frame
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
                else if (drawSpecialTile(tileType, pos, size)) { /* drawn */ }
                else if (tileType == 31) {
                    AEGfxTexture* summerArtifactsTex = sprite::summerArtifacts();
                    gfx::Vec2 artifactsSize{ size.x * 0.9f, size.y * 0.9f };
                    if (summerArtifactsTex) gfx::drawSprite(summerArtifactsTex, pos, 0.0f, artifactsSize, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
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

                if (!isSolid(row + 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row - 1, col)) {
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                }
                if (!isSolid(row, col - 1)) {
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
                if (!isSolid(row, col + 1)) {
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                }
            }
        }
    }
    // SummerS4::drawTiles() is identical to S3 above — same body, different class name

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