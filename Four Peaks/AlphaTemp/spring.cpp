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
#include <vector>
#include <cstdlib>

namespace
{
    using u32 = std::uint32_t;

    // ===================================================================
    // BACKGROUND
    // ===================================================================
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
    static void drawTilesCommon(int gridRows, int gridCols, int tileMap[][32], float stageBaseY = 0.0f)
    {
        auto isSolid = [&](int r, int c) -> bool {
            if (r < 0 || r >= gridRows || c < 0 || c >= gridCols) return false;
            int t = tileMap[r][c];
            return (t == 30 || t == 3 || t == 4 || t == 5 || t == 6 || t == 7 || t == 23);
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
                if (tileType == 23)
                {
                    AEGfxTexture* t = sprite::grass();
                    if (t) gfx::drawSprite(t, pos, 0.0f, size, 0, 0, 1, 1);
                    else   gfx::drawRectangle(pos, 0.0f, size, 0xFF00AA00u);
                    // falls through to border pass
                }
                else if (tileType == 19) {
                    AEGfxTexture* tex = sprite::sign();
                    // Draw sign 2 tiles tall, anchored to bottom of tile
                    gfx::Vec2 signSize{ size.x * 0.9f, size.y * 1.05f };
                    gfx::Vec2 signPos{ pos.x, pos.y + (signSize.y - size.y) * 0.5f };
                    if (tex) gfx::drawSprite(tex, signPos, 0.0f, signSize, 0.0f, 0.0f, 1.0f, 1.0f);
                    else     gfx::drawRectangle(signPos, 0.0f, signSize, 0xFF88FF88u);
                }
                else if (tileType == 32) {
                    AEGfxTexture* springArtifactsTex = sprite::springArtifacts();
                    float bobOffset = sinf((float)AEGetTime(nullptr) * 2.0f) * (cellH * 0.08f);
                    gfx::Vec2 artifactsPos{ pos.x, pos.y + bobOffset };
                    gfx::Vec2 artifactsSize{ size.x * 0.9f, size.y * 0.9f };
                    if (springArtifactsTex) gfx::drawSprite(springArtifactsTex, artifactsPos, 0.0f, artifactsSize, 0, 0, 1, 1);
                    else   gfx::drawRectangle(artifactsPos, 0.0f, size, 0xFF00AA00u);
                }
                else if (tileType == 30)
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
    // WIND SYSTEM
    // Pushes the player left or right on a fixed interval. Visualised
    // with streaking petal/dust particles that move with the wind.
    // ===================================================================

    // Direction: +1 = right, -1 = left
    enum class WindDir { Right = 1, Left = -1 };

    struct WindParticle
    {
        float x, y;
        float velX, velY;   // pixels per second
        float alpha;
        float lifetime;
        float maxLifetime;
        float length;       // streak length
        float thickness;
        u32   color;
        bool  active;
    };

    struct WindSystem
    {
        // ---- Tuning constants ----
        static constexpr float WIND_INTERVAL = 8.0f;  // seconds between wind shifts
        static constexpr float GUST_BUILDUP = 1.2f;  // ramp-up time (seconds)
        static constexpr float GUST_HOLD = 3.5f;  // full-strength hold time
        static constexpr float GUST_FADEOUT = 1.5f;  // fade-out time
        static constexpr float GUST_TOTAL = GUST_BUILDUP + GUST_HOLD + GUST_FADEOUT;
        static constexpr float WIND_FORCE = 180.0f; // peak push force on player (units/s)
        static constexpr int   MAX_PARTICLES = 200;
        static constexpr float PARTICLE_RATE = 0.018f; // seconds between particle spawns

        std::vector<WindParticle> particles;

        WindDir dir = WindDir::Right;
        float   cycleTimer = 0.0f;  // counts to WIND_INTERVAL, then fires a gust
        float   gustTimer = 0.0f;  // counts within the active gust
        bool    gustActive = false;
        float   currentForce = 0.0f;  // current force actually applied this frame
        float   particleTimer = 0.0f;

        bool initialised = false;

        // Spring petal colours — soft pastels (ARGB)
        static constexpr u32 COLORS[] = {
            0xFFFFB6C1u,  // light pink
            0xFFFFD1DCu,  // pastel pink
            0xFFFFECF0u,  // blush white
            0xFFFFC0CBu,  // pink
            0xFFFFE4E1u,  // misty rose
            0xFFFFFFFFu,  // white (petal edge)
            0xFFE8F5E9u,  // pale green (leaf bits)
            0xFFF8BBD0u,  // deeper pink
        };
        static constexpr int COLOR_COUNT = 8;

        static float randF(float lo, float hi)
        {
            return lo + (hi - lo) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
        }

        void init()
        {
            if (initialised) return;
            particles.reserve(MAX_PARTICLES);
            initialised = true;
        }

        // Returns 0..1 strength of the current gust
        float gustStrength() const
        {
            if (!gustActive) return 0.0f;
            if (gustTimer < GUST_BUILDUP)
                return gustTimer / GUST_BUILDUP;
            if (gustTimer < GUST_BUILDUP + GUST_HOLD)
                return 1.0f;
            float fade = gustTimer - GUST_BUILDUP - GUST_HOLD;
            return 1.0f - (fade / GUST_FADEOUT);
        }

        void spawnParticle()
        {
            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();

            float strength = gustStrength();
            float speed = 300.0f + strength * 500.0f; // faster when wind is stronger
            float dirSign = (dir == WindDir::Right) ? 1.0f : -1.0f;

            WindParticle p{};
            // Spawn from the upwind edge of the screen
            p.x = (dirSign > 0.0f) ? minX - 10.0f : maxX + 10.0f;
            p.y = randF(minY, maxY);

            p.velX = dirSign * speed;
            p.velY = randF(-30.0f, 30.0f);
            p.alpha = randF(0.3f, 0.7f) * strength;
            p.maxLifetime = randF(0.4f, 1.2f);
            p.lifetime = 0.0f;
            p.length = randF(8.0f, 30.0f) * strength;
            p.thickness = randF(1.0f, 3.5f);
            p.color = COLORS[rand() % COLOR_COUNT];
            p.active = true;

            for (auto& slot : particles)
                if (!slot.active) { slot = p; return; }
            if (static_cast<int>(particles.size()) < MAX_PARTICLES)
                particles.push_back(p);
        }

        // Call this every frame from each stage's update().
        // Returns the horizontal force to apply to the player this frame.
        float update(float dt)
        {
            init();

            // Cycle timer — fires gust and alternates direction
            if (!gustActive)
            {
                cycleTimer += dt;
                if (cycleTimer >= WIND_INTERVAL)
                {
                    cycleTimer = 0.0f;
                    gustTimer = 0.0f;
                    gustActive = true;
                    // Flip direction each gust
                    dir = (dir == WindDir::Right) ? WindDir::Left : WindDir::Right;
                }
            }
            else
            {
                gustTimer += dt;
                if (gustTimer >= GUST_TOTAL)
                {
                    gustActive = false;
                    gustTimer = 0.0f;
                }
            }

            float strength = gustStrength();
            currentForce = strength * WIND_FORCE * static_cast<float>(dir == WindDir::Right ? 1 : -1);

            // Spawn particles only while wind is blowing
            if (gustActive && strength > 0.05f)
            {
                particleTimer += dt;
                if (particleTimer >= PARTICLE_RATE)
                {
                    particleTimer -= PARTICLE_RATE;
                    // Spawn several per tick so the screen fills nicely
                    int burst = 1 + static_cast<int>(strength * 4.0f);
                    for (int i = 0; i < burst; ++i)
                        spawnParticle();
                }
            }

            float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
            float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();

            for (auto& p : particles)
            {
                if (!p.active) continue;

                p.lifetime += dt;
                p.x += p.velX * dt;
                p.y += p.velY * dt;

                // Fade out in last 30% of lifetime
                float lifeRatio = p.lifetime / p.maxLifetime;
                if (lifeRatio > 0.7f)
                    p.alpha *= (1.0f - (lifeRatio - 0.7f) / 0.3f);

                if (p.lifetime >= p.maxLifetime ||
                    p.x < minX - 40.0f || p.x > maxX + 40.0f ||
                    p.y < minY - 10.0f || p.y > maxY + 10.0f)
                    p.active = false;
            }

            return currentForce;
        }

        // Draw streaking petal particles and a subtle wind-direction arrow UI
        void draw() const
        {
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);

            float strength = gustStrength();

            for (const auto& p : particles)
            {
                if (!p.active) continue;

                float a = p.alpha;
                float r = ((p.color >> 16) & 0xFF) / 255.0f;
                float g = ((p.color >> 8) & 0xFF) / 255.0f;
                float b = ((p.color >> 0) & 0xFF) / 255.0f;

                u32 col = (static_cast<u32>(a * 255) << 24)
                    | (static_cast<u32>(r * 255) << 16)
                    | (static_cast<u32>(g * 255) << 8)
                    | static_cast<u32>(b * 255);

                // Streak: a thin elongated rectangle aligned to velocity direction
                float angle = std::atan2(p.velY, p.velX);
                gfx::Vec2 pos{ p.x, p.y };
                gfx::Vec2 streak{ p.length, p.thickness };
                gfx::drawRectangle(pos, angle, streak, col);
            }

            // HUD: small wind-direction indicator in the top-centre of the screen
            if (gustActive && strength > 0.05f)
            {
                float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
                float maxY = AEGfxGetWinMaxY();

                float cx = (minX + maxX) * 0.5f;
                float cy = maxY - 28.0f;
                u32   arrowA = static_cast<u32>(strength * 200.0f);
                u32   hudCol = (arrowA < 24u ? arrowA : 24u) | 0x00AAFFAAu; // soft green tint

                float dirSign = (dir == WindDir::Right) ? 1.0f : -1.0f;

                // Arrow shaft
                gfx::drawRectangle({ cx, cy }, 0.0f, { 60.0f * strength, 5.0f }, hudCol);
                // Arrow head
                float tipX = cx + dirSign * 30.0f * strength;
                gfx::drawRectangle({ tipX, cy }, 0.785f, { 14.0f, 4.0f }, hudCol);
                gfx::drawRectangle({ tipX, cy }, -0.785f, { 14.0f, 4.0f }, hudCol);
            }

            AEGfxSetBlendMode(AE_GFX_BM_NONE);
        }

        void reset()
        {
            particles.clear();
            cycleTimer = 0.0f;
            gustTimer = 0.0f;
            gustActive = false;
            currentForce = 0.0f;
            particleTimer = 0.0f;
            initialised = false;
        }
    };

    // Single instance shared across all Spring stages
    static WindSystem g_windSystem;

} // anonymous namespace


// ===================================================================
// SpringS1  —  action codes: 40 -> SpringS2 | 2 -> MainMenu
// ===================================================================
game::SpringS1::SpringS1()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s1.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS1::~SpringS1() = default;

int game::SpringS1::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;

    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(0.0f, camera::screenHeight(), 0.3f);
            return 40;
        }
    }

    // Apply wind force to player before movement update
    float windForce = g_windSystem.update(dt);
    if (!camera::isTransitioning() && !gGame.player.respawning)
    {
        gGame.player.pos.x += windForce * dt;
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

    // Teleporter visual: col 0, rows 16-18
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 16, 17, 18 })
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
    }

    PlayerDraw(gGame.player);
    g_windSystem.draw();
}

u32  game::SpringS1::getTileColor(int t) const { return getTileColorCommon(t); }
void game::SpringS1::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::SpringS1::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::SpringS1::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }


// ===================================================================
// SpringS2  —  action codes: 41 -> SpringS3
// ===================================================================
game::SpringS2::SpringS2()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s2.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS2::~SpringS2() = default;

int game::SpringS2::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;

    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(camera::screenHeight(), camera::screenHeight() * 2.0f, 0.3f);
            return 41;
        }
    }

    float windForce = g_windSystem.update(dt);
    if (!camera::isTransitioning() && !gGame.player.respawning)
    {
        gGame.player.pos.x += windForce * dt;
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

    // Teleporter visual: col 31, rows 17-19
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 17, 18, 19 })
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
    }

    PlayerDraw(gGame.player);
    g_windSystem.draw();
}

u32  game::SpringS2::getTileColor(int t) const { return getTileColorCommon(t); }
void game::SpringS2::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::SpringS2::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::SpringS2::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }


// ===================================================================
// SpringS3  —  action codes: 42 -> SpringS4
// ===================================================================
game::SpringS3::SpringS3()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s3.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS3::~SpringS3() = default;

int game::SpringS3::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;

    if (AEInputCheckTriggered(AEVK_UP))
    {
        if (!camera::isTransitioning())
        {
            camera::startTransitionY(camera::screenHeight() * 2.0f, camera::screenHeight() * 3.0f, 0.3f);
            return 42;
        }
    }

    float windForce = g_windSystem.update(dt);
    if (!camera::isTransitioning() && !gGame.player.respawning)
    {
        gGame.player.pos.x += windForce * dt;
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

    // Teleporter visual: col 0, rows 17-19
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 17, 18, 19 })
        {
            gfx::Vec2 p{ std::round(minX + 0 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
    }

    PlayerDraw(gGame.player);
    g_windSystem.draw();
}

u32  game::SpringS3::getTileColor(int t) const { return getTileColorCommon(t); }
void game::SpringS3::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::SpringS3::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::SpringS3::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }


// ===================================================================
// SpringS4  —  action codes: 43 -> AutumnS1
// ===================================================================
game::SpringS4::SpringS4()
{
    gridVisible = false;
    level::loadTileMap("Assets/Levels/spring_s4.txt", gridRows, gridCols, &tileMap[0][0]);
}
game::SpringS4::~SpringS4() = default;

int game::SpringS4::update(float dt)
{
    if (AEInputCheckTriggered(AEVK_G))      gridVisible = !gridVisible;

    float windForce = g_windSystem.update(dt);
    if (!camera::isTransitioning() && !gGame.player.respawning)
    {
        gGame.player.pos.x += windForce * dt;
    }

    if (!camera::isTransitioning()) PlayerUpdate(gGame.player, dt);

    // Teleporter to AutumnS1
    if (!camera::isTransitioning())
    {
        int   teleportCol = 31;
        int   teleportRow2 = 18;

        float gridWorldX{}, gridWorldY{}, cellW{}, cellH{};
        gridToWorld(teleportCol, teleportRow2, gridWorldX, gridWorldY, cellW, cellH);

        float teleportCenterX = gridWorldX + cellW * 0.5f;
        float teleportCenterY = gridWorldY + cellH * 1.0f;

        float dx = gGame.player.pos.x - teleportCenterX;
        float dy = gGame.player.pos.y - teleportCenterY;

        if (std::sqrt(dx * dx + dy * dy) < cellW * 1.5f)
        {
            g_windSystem.reset();
            return 43;
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

    // Teleporter visual: col 31, rows 18-19
    if (!camera::isTransitioning())
    {
        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float cw = (maxX - minX) / static_cast<float>(gridCols);
        float ch = (maxY - minY) / static_cast<float>(gridRows);
        for (int row : { 18, 19 })
        {
            gfx::Vec2 p{ std::round(minX + 31 * cw + cw * 0.5f), std::round(minY + row * ch + ch * 0.5f) };
            gfx::drawRectangle(p, 0.0f, { cw, ch }, 0xAA00FFFFu);
        }
    }

    PlayerDraw(gGame.player);
    g_windSystem.draw();
}

u32  game::SpringS4::getTileColor(int t) const { return getTileColorCommon(t); }
void game::SpringS4::gridToWorld(int col, int row, float& xW, float& yW, float& cW, float& cH) const
{
    gridToWorldCommon(col, row, gridCols, gridRows, xW, yW, cW, cH);
}
void game::SpringS4::drawGrid()  const { drawGridLines(gridCols, gridRows); }
void game::SpringS4::drawTiles() const { drawTilesCommon(gridRows, gridCols, (int(*)[32])tileMap, 0.0f); }