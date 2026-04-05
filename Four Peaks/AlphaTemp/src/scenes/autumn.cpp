// ----------------------------------------------------------------------------
// Done By: Hong Yang, Skyler, Arun, Justin
// ----------------------------------------------------------------------------

#include "scenes/autumn.hpp"

#include "AEEngine.h"
#include "engine/graphics.hpp"
#include "gameplay/player.hpp"
#include "core/gamestate.hpp"
#include "engine/sprite.hpp"
#include "core/level_loader.hpp"
#include "engine/camera.hpp"

#include <cstdint>
#include <cmath>
#include <vector>
#include <cstdlib>

#include "engine/collision.hpp"

namespace
{
    using u32 = std::uint32_t;

    // ===================================================================
    // BACKGROUND
    // ===================================================================
    static void drawBackground()
    {
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

    // ===================================================================
    // GRID HELPERS
    // ===================================================================
    static void gridToWorldCommon(int col, int row, int gridCols, int gridRows,
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

    // ===================================================================
    // TILE COLOR FALLBACKS
    // ===================================================================
    static u32 getTileColorCommon(int tileType)
    {
        switch (tileType)
        {
        case 6:  return 0xFF555555u;
        case 7:  return 0xFF888888u;
        case 1:  return 0xFF66AAFFu;
        case 2:  return 0xFFFF3333u;
        case 8:  return 0xFFFFFF00u;
        case 10: return 0xFF00FF00u;
        default: return 0x00000000u;
        }
    }

    // ===================================================================
    // TILE DRAWING
    // ===================================================================
    static void drawTilesCommon(int gridRows, int gridCols, int tileMap[][32],
        float stageBaseY = 0.0f,
        const std::vector<game::BreakableTileState>* breakableTiles = nullptr)
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
                u32   borderColor = 0xFF000000;

                if (sprite::drawAnimatedTile(tileType, pos, size)) continue;

                if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
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
                    continue;
                }
                else if (tileType == 1)
                {
                    AEGfxTexture* t = sprite::spring1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else if (tileType == 3)
                {
                    AEGfxTexture* t = sprite::spring2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else if (tileType == 5)
                {
                    AEGfxTexture* t = sprite::autumn1();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else if (tileType == 7)
                {
                    AEGfxTexture* t = sprite::autumn2();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }
                else if (tileType == 15)
                {
                    AEGfxTexture* breakabletileTex = sprite::breakabletile();
                    if (breakabletileTex && breakableTiles)
                    {
                        int thisCrackFrame = 0;
                        for (const auto& brkTile : *breakableTiles)
                        {
                            if (brkTile.row == r && brkTile.col == c)
                            {
                                thisCrackFrame = brkTile.crackFrame;
                                break;
                            }
                        }
                        float u0{}, v0{}, u1{}, v1{};
                        sprite::getCrackUv(thisCrackFrame, u0, v0, u1, v1);
                        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                        gfx::drawSprite(breakabletileTex, pos, 0.0f, size, u0, v0, u1, v1);
                        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                        AEGfxSetBlendMode(AE_GFX_BM_NONE);
                    }
                    continue;
                }
                else if (tileType == 19) {
                    AEGfxTexture* tex = sprite::sign();
                    // Draw sign 2 tiles tall, anchored to bottom of tile
                    gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.05f };
                    gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
                    if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
                }
                else if (tileType == 33) {
                    AEGfxTexture* autumnArtifactsTex = sprite::autumnArtifacts();
                    float bobOffset = sinf((float)AEGetTime(nullptr) * 2.0f) * (cellH * 0.08f);
                    gfx::Vec2 artifactsPos{ pos.x, pos.y + bobOffset };
                    gfx::Vec2 artifactsSize{ size.x * 0.9f, size.y * 0.9f };
                    if (autumnArtifactsTex) gfx::drawSprite(autumnArtifactsTex, artifactsPos, 0.0f, artifactsSize, 0, 0, 1, 1);
                    else   gfx::drawRectangle(artifactsPos, 0.0f, size, 0xFF00AA00u);
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
                    float u0{}, v0{}, u1{}, v1{};
                    if (sprite::getTileUv(tileType, u0, v0, u1, v1))
                        gfx::drawSprite(sprite::tileset(), pos, 0.0f, size, u0, v0, u1, v1);
                    else
                        gfx::drawRectangle(pos, 0.0f, size, getTileColorCommon(tileType));
                }

                // ---- Borders (solid tiles only) ----
                if (!isSolid(r, c)) continue;

                if (!isSolid(r + 1, c))
                    gfx::drawRectangle({ pos.x, pos.y + size.y * 0.5f - border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(r - 1, c))
                    gfx::drawRectangle({ pos.x, pos.y - size.y * 0.5f + border * 0.5f }, 0.0f, { size.x, border }, borderColor);
                if (!isSolid(r, c - 1))
                    gfx::drawRectangle({ pos.x - size.x * 0.5f + border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
                if (!isSolid(r, c + 1))
                    gfx::drawRectangle({ pos.x + size.x * 0.5f - border * 0.5f, pos.y }, 0.0f, { border, size.y }, borderColor);
            }
        }
    }

    // ===================================================================
    // LEAF SHAPE HELPER
    //
    // Draws one broad leaf at (cx, cy) using 5 overlapping rectangles:
    //   - Tall narrow spine down the centre
    //   - Two wider lobes angled ~22° outward on each side
    //   - Two shallower outer rects to puff the mid-section
    // All pieces share the same base rotation so the whole shape
    // rotates freely.
    // ===================================================================
    static void drawLeaf(float cx, float cy,
        float size,
        float rotation,
        u32   baseColor,
        float alpha)
    {
        auto withAlpha = [](u32 c, float a) -> u32 {
            return (static_cast<u32>(a * 255.0f) << 24) | (c & 0x00FFFFFFu);
            };

        float s = size;
        gfx::Vec2 centre{ cx, cy };

        // Centre spine (vein)
        gfx::drawRectangle(centre, rotation, { s * 0.22f, s * 2.0f }, withAlpha(baseColor, alpha * 0.95f));

        // Left and right lobes
        gfx::drawRectangle(centre, rotation + 0.38f, { s * 0.55f, s * 1.55f }, withAlpha(baseColor, alpha * 0.90f));
        gfx::drawRectangle(centre, rotation - 0.38f, { s * 0.55f, s * 1.55f }, withAlpha(baseColor, alpha * 0.90f));

        // Outer fill to puff the middle
        gfx::drawRectangle(centre, rotation + 0.18f, { s * 0.70f, s * 1.20f }, withAlpha(baseColor, alpha * 0.70f));
        gfx::drawRectangle(centre, rotation - 0.18f, { s * 0.70f, s * 1.20f }, withAlpha(baseColor, alpha * 0.70f));
    }

    // ===================================================================
    // LEAF PARTICLE SYSTEM
    //
    // Leaves appear at RANDOM positions on screen at RANDOM times.
    // They do NOT rain from the top — they simply pop into existence,
    // drift very slowly, then fade out.
    //
    // A COVER EVENT fires every COVER_INTERVAL seconds: a dense cluster
    // of large leaves suddenly fills a random region of the screen.
    // ===================================================================
    struct LeafParticle
    {
        float x, y;
        float driftX, driftY;
        float rotation;
        float rotSpeed;
        float size;
        u32   color;
        float alpha;
        float fadeInTime;
        float holdTime;
        float fadeOutTime;
        float timer;
        bool  active;
    };

    struct LeafSystem
    {
        static constexpr int   MAX_LEAVES = 200;
        static constexpr float AMBIENT_MIN_GAP = 0.3f;
        static constexpr float AMBIENT_MAX_GAP = 2.5f;
        static constexpr int   AMBIENT_BURST = 3;
        static constexpr float COVER_INTERVAL = 10.0f;
        static constexpr int   COVER_COUNT = 60;

        std::vector<LeafParticle> leaves;

        float ambientTimer = 0.0f;
        float nextAmbientTime = 0.0f;
        float coverTimer = 0.0f;
        bool  initialised = false;

        // -------------------------------------------------------
        // Pure orange palette — every shade is unambiguously orange
        // (R=255, G=100..180, B=0).  No reds, no browns.
        // -------------------------------------------------------
        static constexpr u32 COLORS[] = {
            0xFFFFA500u,  // pure orange
            0xFFFF8C00u,  // dark orange
            0xFFFF6600u,  // vivid orange
            0xFFFF7F00u,  // blaze orange
            0xFFFFB300u,  // golden orange
            0xFFFF9000u,  // bright orange
            0xFFFFAA00u,  // light orange
            0xFFE67700u,  // deep orange
        };
        static constexpr int COLOR_COUNT = 8;

        static float randF(float lo, float hi)
        {
            return lo + (hi - lo) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
        }

        void init()
        {
            if (initialised) return;
            leaves.reserve(MAX_LEAVES);
            nextAmbientTime = randF(AMBIENT_MIN_GAP, AMBIENT_MAX_GAP);
            initialised = true;
        }

        void spawnAt(float x, float y, float minSize, float maxSize)
        {
            LeafParticle lp{};
            lp.x = x;
            lp.y = y;
            lp.driftX = randF(-12.0f, 12.0f);
            lp.driftY = randF(-8.0f, 8.0f);
            lp.rotation = randF(0.0f, 6.28f);
            lp.rotSpeed = randF(-0.4f, 0.4f);
            lp.size = randF(minSize, maxSize);
            lp.color = COLORS[rand() % COLOR_COUNT];
            lp.alpha = 0.0f;
            lp.fadeInTime = randF(0.3f, 0.7f);
            lp.holdTime = randF(1.5f, 4.0f);
            lp.fadeOutTime = randF(0.5f, 1.2f);
            lp.timer = 0.0f;
            lp.active = true;

            for (auto& slot : leaves)
                if (!slot.active) { slot = lp; return; }
            if (static_cast<int>(leaves.size()) < MAX_LEAVES)
                leaves.push_back(lp);
        }

        void spawnCluster(int count, float minSize, float maxSize)
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
            float screenW = maxX - minX;
            float screenH = maxY - minY;

            float regionCX = randF(minX + screenW * 0.15f, maxX - screenW * 0.15f);
            float regionCY = randF(minY + screenH * 0.15f, maxY - screenH * 0.15f);
            float spreadX = screenW * randF(0.25f, 0.55f);
            float spreadY = screenH * randF(0.25f, 0.55f);

            for (int i = 0; i < count; ++i)
            {
                float lx = regionCX + randF(-spreadX, spreadX);
                float ly = regionCY + randF(-spreadY, spreadY);
                lx = std::fmaxf(minX, std::fminf(maxX, lx));
                ly = std::fmaxf(minY, std::fminf(maxY, ly));
                spawnAt(lx, ly, minSize, maxSize);
            }
        }

        void update(float dt)
        {
            init();

            // Ambient appearances: a few leaves pop in at random spots
            ambientTimer += dt;
            if (ambientTimer >= nextAmbientTime)
            {
                ambientTimer = 0.0f;
                nextAmbientTime = randF(AMBIENT_MIN_GAP, AMBIENT_MAX_GAP);

                float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
                float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
                for (int i = 0; i < AMBIENT_BURST; ++i)
                    spawnAt(randF(minX, maxX), randF(minY, maxY), 8.0f, 20.0f);
            }

            // Cover event: dense cluster of big leaves in a random region
            coverTimer += dt;
            if (coverTimer >= COVER_INTERVAL)
            {
                coverTimer = 0.0f;
                spawnCluster(COVER_COUNT, 22.0f, 55.0f);
            }

            for (auto& lp : leaves)
            {
                if (!lp.active) continue;

                lp.timer += dt;
                lp.rotation += lp.rotSpeed * dt;
                lp.x += lp.driftX * dt;
                lp.y += lp.driftY * dt;

                float totalLife = lp.fadeInTime + lp.holdTime + lp.fadeOutTime;

                if (lp.timer < lp.fadeInTime)
                    lp.alpha = lp.timer / lp.fadeInTime;
                else if (lp.timer < lp.fadeInTime + lp.holdTime)
                    lp.alpha = 1.0f;
                else if (lp.timer < totalLife)
                    lp.alpha = 1.0f - (lp.timer - lp.fadeInTime - lp.holdTime) / lp.fadeOutTime;
                else
                    lp.active = false;
            }
        }

        void draw() const
        {
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);

            for (const auto& lp : leaves)
            {
                if (!lp.active || lp.alpha <= 0.0f) continue;
                drawLeaf(lp.x, lp.y, lp.size, lp.rotation, lp.color, lp.alpha);
            }

            AEGfxSetBlendMode(AE_GFX_BM_NONE);
        }

        void reset()
        {
            leaves.clear();
            ambientTimer = 0.0f;
            nextAmbientTime = 0.0f;
            coverTimer = 0.0f;
            initialised = false;
        }
    };

    static LeafSystem g_leafSystem;

} // anonymous namespace


// ===================================================================
// AutumnS1  —  60 -> AutumnS2 | 2 -> MainMenu
// ===================================================================
game::AutumnS1::AutumnS1()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s1.txt", gridRows, gridCols, &tileMap[0][0]);

    for (int row = 0; row < gridRows; ++row)
        for (int col = 0; col < gridCols; ++col) {
            if (tileMap[row][col] == 15) {
                BreakableTileState brkTile;
                brkTile.row = row;
                brkTile.col = col;
                breakableTiles.push_back(brkTile);
            }
        }
}

void game::AutumnS1::resetBreakableTile()
{
    for (auto& brktile : breakableTiles) {
        tileMap[brktile.row][brktile.col] = 15;
        brktile.triggered = false;
        brktile.timer = 0.0f;
        brktile.crackFrame = 0;
        brktile.destroyed = false;
    }
}

game::AutumnS1::~AutumnS1() = default;

int game::AutumnS1::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;


    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    // Teleport zone: matches visual indicator (col 31, rows 7-9)
    {
        float gx, gy, cw, ch;
        gridToWorld(31, 7, gx, gy, cw, ch);
        float zoneLeft  = gx;
        float zoneRight = gx + cw;
        float zoneBot   = gy;
        float zoneTop   = gy + ch * 3.0f;
        if (gGame.player.pos.x >= zoneLeft  && gGame.player.pos.x <= zoneRight &&
            gGame.player.pos.y >= zoneBot   && gGame.player.pos.y <= zoneTop)
            return 60;
    }

    sprite::updateAnimatedTiles(dt);
    g_leafSystem.update(dt);

    for (auto& brktile : breakableTiles)
        brktile.triggered = false;

    for (auto& trigger : g_triggeredbrkTiles)
        for (auto& brktile : breakableTiles)
            if (brktile.row == trigger.row && brktile.col == trigger.col && !brktile.triggered)
                brktile.triggered = true;

    g_triggeredbrkTiles.clear();

    for (auto& brktile : breakableTiles)
    {
        if (brktile.triggered && !brktile.destroyed)
        {
            brktile.timer += (dt * 2.0f);
            int nf = static_cast<int>(brktile.timer / sprite::crackFrameTime);
            if (nf >= sprite::crackFrameCount - 1)
            {
                tileMap[brktile.row][brktile.col] = 0;
                brktile.destroyed = true;
            }
            else brktile.crackFrame = nf;
        }
    }
    
    if (gGame.player.respawning) {
        for (auto& brktile : breakableTiles) {
            tileMap[brktile.row][brktile.col] = 15;
            brktile.triggered = false;
            brktile.timer = 0.0f;
            brktile.crackFrame = 0;
            brktile.destroyed = false;
        }
    }

    return 0;
}

void game::AutumnS1::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 7, 8, 9 })
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAAFFFFFFu);
        }
    }

    PlayerDraw(gGame.player, gridVisible);
    g_leafSystem.draw();
}

u32  game::AutumnS1::getTileColor(int t) const { return getTileColorCommon(t); }
void game::AutumnS1::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::AutumnS1::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::AutumnS1::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f, &breakableTiles); }


// ===================================================================
// AutumnS2  —  61 -> AutumnS3
// ===================================================================
game::AutumnS2::AutumnS2()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s2.txt", gridRows, gridCols, &tileMap[0][0]);

    for (int row = 0; row < gridRows; ++row)
        for (int col = 0; col < gridCols; ++col) {
            if (tileMap[row][col] == 15) {
                BreakableTileState brkTile;
                brkTile.row = row;
                brkTile.col = col;
                breakableTiles.push_back(brkTile);
            }
        }
}

void game::AutumnS2::resetBreakableTile()
{
    for (auto& brktile : breakableTiles) {
        tileMap[brktile.row][brktile.col] = 15;
        brktile.triggered = false;
        brktile.timer = 0.0f;
        brktile.crackFrame = 0;
        brktile.destroyed = false;
    }
}

game::AutumnS2::~AutumnS2() = default;

int game::AutumnS2::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;


    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    // Teleport zone: matches visual indicator (col 31, rows 17-19)
    {
        float gx, gy, cw, ch;
        gridToWorld(31, 17, gx, gy, cw, ch);
        float zoneLeft  = gx;
        float zoneRight = gx + cw;
        float zoneBot   = gy;
        float zoneTop   = gy + ch * 3.0f;
        if (gGame.player.pos.x >= zoneLeft  && gGame.player.pos.x <= zoneRight &&
            gGame.player.pos.y >= zoneBot   && gGame.player.pos.y <= zoneTop)
            return 61;
    }

    sprite::updateAnimatedTiles(dt);

    for (auto& brktile : breakableTiles)
        brktile.triggered = false;

    for (auto& trigger : g_triggeredbrkTiles)
        for (auto& brktile : breakableTiles)
            if (brktile.row == trigger.row && brktile.col == trigger.col && !brktile.triggered) brktile.triggered = true;

    g_triggeredbrkTiles.clear();

    for (auto& brktile : breakableTiles) {
        if (brktile.triggered && !brktile.destroyed) {
            brktile.timer += (dt * 2.0f);
            int nf = static_cast<int>(brktile.timer / sprite::crackFrameTime);
            if (nf >= sprite::crackFrameCount - 1) { tileMap[brktile.row][brktile.col] = 0; brktile.destroyed = true; }
            else brktile.crackFrame = nf;
        }
    }

    if (gGame.player.respawning) {
        for (auto& brktile : breakableTiles) {
            tileMap[brktile.row][brktile.col] = 15;
            brktile.triggered = false;
            brktile.timer = 0.0f;
            brktile.crackFrame = 0;
            brktile.destroyed = false;
        }
    }

    return 0;
}

void game::AutumnS2::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 17, 18, 19 })
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAAFFFFFFu);
        }
    }

    PlayerDraw(gGame.player, gridVisible);
    g_leafSystem.draw();
}

u32  game::AutumnS2::getTileColor(int t) const { return getTileColorCommon(t); }
void game::AutumnS2::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::AutumnS2::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::AutumnS2::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f, &breakableTiles); }


// ===================================================================
// AutumnS3  —  62 -> AutumnS4
// ===================================================================
game::AutumnS3::AutumnS3()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s3.txt", gridRows, gridCols, &tileMap[0][0]);

    for (int row = 0; row < gridRows; ++row)
        for (int col = 0; col < gridCols; ++col) {
            if (tileMap[row][col] == 15) {
                BreakableTileState brkTile;
                brkTile.row = row;
                brkTile.col = col;
                breakableTiles.push_back(brkTile);
            }
        }
}

void game::AutumnS3::resetBreakableTile()
{
    for (auto& brktile : breakableTiles) {
        tileMap[brktile.row][brktile.col] = 15;
        brktile.triggered = false;
        brktile.timer = 0.0f;
        brktile.crackFrame = 0;
        brktile.destroyed = false;
    }
}

game::AutumnS3::~AutumnS3() = default;

int game::AutumnS3::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;


    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    // Teleport zone: matches visual indicator (col 31, rows 1-3)
    {
        float gx, gy, cw, ch;
        gridToWorld(31, 1, gx, gy, cw, ch);
        float zoneLeft  = gx;
        float zoneRight = gx + cw;
        float zoneBot   = gy;
        float zoneTop   = gy + ch * 3.0f;
        if (gGame.player.pos.x >= zoneLeft  && gGame.player.pos.x <= zoneRight &&
            gGame.player.pos.y >= zoneBot   && gGame.player.pos.y <= zoneTop)
            return 62;
    }

    sprite::updateAnimatedTiles(dt);

    for (auto& brktile : breakableTiles)
        brktile.triggered = false;

    for (auto& trigger : g_triggeredbrkTiles)
        for (auto& brktile : breakableTiles)
            if (brktile.row == trigger.row && brktile.col == trigger.col && !brktile.triggered) brktile.triggered = true;

    g_triggeredbrkTiles.clear();

    for (auto& brktile : breakableTiles) {
        if (brktile.triggered && !brktile.destroyed) {
            brktile.timer += (dt * 2.0f);
            int nf = static_cast<int>(brktile.timer / sprite::crackFrameTime);
            if (nf >= sprite::crackFrameCount - 1) { tileMap[brktile.row][brktile.col] = 0; brktile.destroyed = true; }
            else brktile.crackFrame = nf;
        }
    }

    if (gGame.player.respawning) {
        for (auto& brktile : breakableTiles) {
            tileMap[brktile.row][brktile.col] = 15;
            brktile.triggered = false;
            brktile.timer = 0.0f;
            brktile.crackFrame = 0;
            brktile.destroyed = false;
        }
    }
    return 0;
}

void game::AutumnS3::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 1, 2, 3 })
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAAFFFFFFu);
        }
    }

    PlayerDraw(gGame.player, gridVisible);
    g_leafSystem.draw();
}

u32  game::AutumnS3::getTileColor(int t) const { return getTileColorCommon(t); }
void game::AutumnS3::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::AutumnS3::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::AutumnS3::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f, &breakableTiles); }


// ===================================================================
// AutumnS4  —  63 -> Tutorial1
// ===================================================================
game::AutumnS4::AutumnS4()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/autumn_s4.txt", gridRows, gridCols, &tileMap[0][0]);

    for (int row = 0; row < gridRows; ++row)
        for (int col = 0; col < gridCols; ++col) {
            if (tileMap[row][col] == 15) {
                BreakableTileState brkTile;
                brkTile.row = row;
                brkTile.col = col;
                breakableTiles.push_back(brkTile);
            }
        }
}


void game::AutumnS4::resetBreakableTile()
{
    for (auto& brktile : breakableTiles) {
        tileMap[brktile.row][brktile.col] = 15;
        brktile.triggered = false;
        brktile.timer = 0.0f;
        brktile.crackFrame = 0;
        brktile.destroyed = false;
    }
}

game::AutumnS4::~AutumnS4() = default;

int game::AutumnS4::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    // Teleport zone: matches visual indicator (col 31, rows 18-19)
    {
        float gx, gy, cw, ch;
        gridToWorld(31, 18, gx, gy, cw, ch);
        float zoneLeft  = gx;
        float zoneRight = gx + cw;
        float zoneBot   = gy;
        float zoneTop   = gy + ch * 2.0f;
        if (gGame.player.pos.x >= zoneLeft  && gGame.player.pos.x <= zoneRight &&
            gGame.player.pos.y >= zoneBot   && gGame.player.pos.y <= zoneTop)
        {
            g_leafSystem.reset();
            return 63;
        }
    }

    sprite::updateAnimatedTiles(dt);

    for (auto& brktile : breakableTiles)
        brktile.triggered = false;

    for (auto& trigger : g_triggeredbrkTiles)
        for (auto& brktile : breakableTiles)
            if (brktile.row == trigger.row && brktile.col == trigger.col && !brktile.triggered) brktile.triggered = true;

    g_triggeredbrkTiles.clear();

    for (auto& brktile : breakableTiles) {
        if (brktile.triggered && !brktile.destroyed) {
            brktile.timer += (dt * 2.0f);
            int nf = static_cast<int>(brktile.timer / sprite::crackFrameTime);
            if (nf >= sprite::crackFrameCount - 1) { tileMap[brktile.row][brktile.col] = 0; brktile.destroyed = true; }
            else brktile.crackFrame = nf;
        }
    }

    if (gGame.player.respawning) {
        for (auto& brktile : breakableTiles) {
            tileMap[brktile.row][brktile.col] = 15;
            brktile.triggered = false;
            brktile.timer = 0.0f;
            brktile.crackFrame = 0;
            brktile.destroyed = false;
        }
    }
    return 0;
}

void game::AutumnS4::draw() const
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    drawBackground();
    drawTiles();
    if (gridVisible) drawGrid();

    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 18, 19 })
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAAFFFFFFu);
        }
    }

    PlayerDraw(gGame.player, gridVisible);
    g_leafSystem.draw();
}

u32  game::AutumnS4::getTileColor(int t) const { return getTileColorCommon(t); }
void game::AutumnS4::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::AutumnS4::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::AutumnS4::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f, &breakableTiles); }