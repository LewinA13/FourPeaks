// ---------------------------------------------------------------------------
// transition.cpp  ?  Pokemon-style screen-wipe transition effect
// ---------------------------------------------------------------------------
// Self-contained: no changes to existing game files needed.
// See transition.hpp for integration instructions.
//
// PokemonWipe:
//   Phase 1 (WipeIn)  - Black diamond/circle pattern closes in from corners
//                        until the whole screen is black (t: 0?1, ~0.35 s)
//   Phase 2 (Hold)    - Screen stays black for a brief moment (~0.1 s)
//                        ? this is when you should load the new scene
//   Phase 3 (WipeOut) - Black circle opens outward revealing the new scene
//                        (t: 1?0, ~0.35 s)
//
// SweepLeft:
//   A solid black panel slides in from the right, pauses, then slides out.
//
// Dissolve:
//   Random 4?4 pixel blocks fade to black then fade back in.
// ---------------------------------------------------------------------------

#define NOMINMAX           // prevent Windows.h from defining min/max macros
#include "transition.hpp"
#include "graphics.hpp"    // gfx::drawRectangle
#include <AEEngine.h>
#include <cmath>
#include <cstdint>
#include <array>
#include <cstdlib>   // rand / srand -- remove?
#include <cstring>   // memset

typedef std::uint32_t u32;

// Timing constants 
static constexpr float WIPE_IN_DURATION = 0.75f;
static constexpr float HOLD_DURATION = 0.25f;
static constexpr float WIPE_OUT_DURATION = 0.75f;

// Dissolve block grid 
static constexpr int DISSOLVE_COLS = 24;
static constexpr int DISSOLVE_ROWS = 16;
static constexpr int DISSOLVE_TOTAL = DISSOLVE_COLS * DISSOLVE_ROWS;

static std::array<float, DISSOLVE_TOTAL> gDissolveOrder{};
static bool gDissolveReady = false;

static void buildDissolveOrder()
{
    for (int i = 0; i < DISSOLVE_TOTAL; ++i)
        gDissolveOrder[i] = static_cast<float>(i) / DISSOLVE_TOTAL;
    // Fisher-Yates shuffle
    for (int i = DISSOLVE_TOTAL - 1; i > 0; --i)
    {
        int j = rand() % (i + 1); // debug remove 
        std::swap(gDissolveOrder[i], gDissolveOrder[j]); 
    }
    gDissolveReady = true;
}

// Solid rectangle helper (colour draw)
void Transition::drawSolidRect(float x, float y, float w, float h, u32 argb)
{
    // Convert ARGB to floats
    float a = ((argb >> 24) & 0xFF) / 255.0f;
    float r = ((argb >> 16) & 0xFF) / 255.0f;
    float g = ((argb >> 8) & 0xFF) / 255.0f;
    float b = ((argb >> 0) & 0xFF) / 255.0f;

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxMeshStart();
    AEGfxTriAdd( //simplify 
        x, y + h, (static_cast<u32>(a * 255) << 24) | (static_cast<u32>(r * 255) << 16) | (static_cast<u32>(g * 255) << 8) | static_cast<u32>(b * 255), 0, 0,
        x + w, y + h, (static_cast<u32>(a * 255) << 24) | (static_cast<u32>(r * 255) << 16) | (static_cast<u32>(g * 255) << 8) | static_cast<u32>(b * 255), 0, 0,
        x, y, (static_cast<u32>(a * 255) << 24) | (static_cast<u32>(r * 255) << 16) | (static_cast<u32>(g * 255) << 8) | static_cast<u32>(b * 255), 0, 0);
    AEGfxTriAdd(
        x + w, y + h, (static_cast<u32>(a * 255) << 24) | (static_cast<u32>(r * 255) << 16) | (static_cast<u32>(g * 255) << 8) | static_cast<u32>(b * 255), 0, 0,
        x + w, y, (static_cast<u32>(a * 255) << 24) | (static_cast<u32>(r * 255) << 16) | (static_cast<u32>(g * 255) << 8) | static_cast<u32>(b * 255), 0, 0,
        x, y, (static_cast<u32>(a * 255) << 24) | (static_cast<u32>(r * 255) << 16) | (static_cast<u32>(g * 255) << 8) | static_cast<u32>(b * 255), 0, 0);
    AEGfxVertexList* mesh = AEGfxMeshEnd();
    if (mesh)
    {
        AEMtx33 identity;
        AEMtx33Identity(&identity);
        AEGfxSetTransform(identity.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(mesh);
    }
}

// Control API 
void Transition::start()
{
    if (phase != Phase::Idle) return;   // already running

    phase = Phase::WipeIn;
    t = 0.0f;
    switchPending = false;
    switchDone = false;

    if (style == TransitionStyle::Dissolve && !gDissolveReady)
        buildDissolveOrder();
}

void Transition::update(float dt)
{
    if (phase == Phase::Idle) return;

    float duration = 0.0f;
    switch (phase)
    {
    case Phase::WipeIn:  duration = WIPE_IN_DURATION / speed; break;
    case Phase::Hold:    duration = HOLD_DURATION / speed; break;
    case Phase::WipeOut: duration = WIPE_OUT_DURATION / speed; break;
    default: break;
    }

    t += dt / duration;
    if (t >= 1.0f)
    {
        t = 0.0f;
        switch (phase)
        {
        case Phase::WipeIn:
            phase = Phase::Hold;
            switchPending = true;  // signal caller to swap scene NOW
            break;
        case Phase::Hold:
            phase = Phase::WipeOut;
            break;
        case Phase::WipeOut:
            phase = Phase::Idle;
            switchDone = true;
            break;
        default: break;
        }
    }
}

// ??? Draw dispatch ???????????????????????????????????????????????????????????
void Transition::draw() const
{
    if (phase == Phase::Idle) return;

    // Compute a 0..1 coverage value:
    //   WipeIn:  0=transparent ? 1=fully black
    //   Hold:    1=fully black
    //   WipeOut: 1=fully black ? 0=transparent
    float coverage = 0.0f;
    bool wipeIn = true;
    switch (phase)
    {
    case Phase::WipeIn:  coverage = t;       wipeIn = true;  break;
    case Phase::Hold:    coverage = 1.0f;    wipeIn = true;  break;
    case Phase::WipeOut: coverage = 1.0f - t; wipeIn = false; break;
    default: break;
    }

    // Smooth step for nicer curve
    float s = coverage * coverage * (3.0f - 2.0f * coverage);

    switch (style)
    {
    case TransitionStyle::PokemonWipe:
        drawPokemonWipe(s, wipeIn || phase == Phase::Hold);
        break;
    case TransitionStyle::SweepLeft:
        drawSweepLeft(s, wipeIn || phase == Phase::Hold);
        break;
    case TransitionStyle::Dissolve:
        drawDissolve(s);
        break;
    }
}

// ??? Pokemon diamond wipe ?????????????????????????????????????????????????????
// Classic: black diamonds/circles close in from all four corners to meet in the
// middle (wipe-in), then open outward from centre (wipe-out).
// Implemented as a grid of black squares whose size grows/shrinks with progress.
void Transition::drawPokemonWipe(float progress, bool /*coverScreen*/) const
{
    float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
    float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
    float scW = maxX - minX;
    float scH = maxY - minY;

    constexpr int GX = 12;
    constexpr int GY = 8;
    float tileW = scW / GX;
    float tileH = scH / GY;

    float maxSize = std::sqrt(tileW * tileW + tileH * tileH) * 1.1f;
    float tileSize = maxSize * progress;
    float h = tileSize * 0.5f;

    u32 col = 0xFF000000u;  // opaque black

    // Batch ALL diamond rects into a single mesh to avoid per-cell draw call overhead.
    // Each diamond = 2 overlapping rects (horizontal + vertical) = 4 tris = 12 verts per cell.
    // 12 x 8 = 96 cells => 96 x 4 = 384 tris total, one mesh, one draw call.
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxMeshStart();
    for (int gy = 0; gy < GY; ++gy)
    {
        for (int gx = 0; gx < GX; ++gx)
        {
            float cx = minX + (gx + 0.5f) * tileW;
            float cy = minY + (gy + 0.5f) * tileH;

            // Horizontal rect
            float x1 = cx - h, y1 = cy - h * 0.45f;
            float w1 = tileSize, h1 = tileSize * 0.45f;
            AEGfxTriAdd(x1, y1 + h1, col, 0, 0,
                x1 + w1, y1 + h1, col, 0, 0,
                x1, y1, col, 0, 0);
            AEGfxTriAdd(x1 + w1, y1 + h1, col, 0, 0,
                x1 + w1, y1, col, 0, 0,
                x1, y1, col, 0, 0);

            // Vertical rect
            float x2 = cx - h * 0.45f, y2 = cy - h;
            float w2 = tileSize * 0.45f, h2 = tileSize;
            AEGfxTriAdd(x2, y2 + h2, col, 0, 0,
                x2 + w2, y2 + h2, col, 0, 0,
                x2, y2, col, 0, 0);
            AEGfxTriAdd(x2 + w2, y2 + h2, col, 0, 0,
                x2 + w2, y2, col, 0, 0,
                x2, y2, col, 0, 0);
        }
    }
    AEGfxVertexList* mesh = AEGfxMeshEnd();
    if (mesh)
    {
        AEMtx33 identity;
        AEMtx33Identity(&identity);
        AEGfxSetTransform(identity.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(mesh);
    }

    // At high progress (>0.7) fill the screen completely to close any remaining gaps
    if (progress > 0.7f)
    {
        float gap = (progress - 0.7f) / 0.3f;
        u32 fillCol = (static_cast<u32>(gap * 255) << 24) | 0x00000000u;
        drawSolidRect(minX, minY, scW, scH, fillCol | 0xFF000000u);
    }
}

// ??? Horizontal sweep ?????????????????????????????????????????????????????????
void Transition::drawSweepLeft(float progress, bool /*coverScreen*/) const
{
    float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
    float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
    float scW = maxX - minX;
    float scH = maxY - minY;

    // Panel sweeps from right (progress=0 ? panel off-screen right)
    // to left (progress=1 ? panel covers whole screen)
    float panelRight = maxX;
    float panelLeft = maxX - progress * scW * 1.05f;  // slight overshoot

    if (panelLeft < panelRight)
        drawSolidRect(panelLeft, minY, panelRight - panelLeft, scH, 0xFF000000u);
}

// ??? Dissolve ?????????????????????????????????????????????????????????????????
// Random-order block reveal/hide.  Each block fades independently based on
// its randomised threshold in gDissolveOrder[].
void Transition::drawDissolve(float progress) const
{
    if (!gDissolveReady) return;

    float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
    float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
    float scW = maxX - minX;
    float scH = maxY - minY;

    float bW = scW / DISSOLVE_COLS;
    float bH = scH / DISSOLVE_ROWS;

    for (int i = 0; i < DISSOLVE_TOTAL; ++i)
    {
        float threshold = gDissolveOrder[i];
        // Blocks whose threshold < progress are black
        float blockAlpha = 0.0f;
        if (progress >= threshold)
        {
            float local = (progress - threshold) / (1.0f / DISSOLVE_TOTAL + 0.05f);
            blockAlpha = (local < 1.0f) ? local : 1.0f;
        }

        if (blockAlpha <= 0.01f) continue;

        int col = i % DISSOLVE_COLS;
        int row = i / DISSOLVE_COLS;
        float bx = minX + col * bW;
        float by = minY + row * bH;

        u32 argb = (static_cast<u32>(blockAlpha * 255) << 24);
        drawSolidRect(bx, by, bW + 1.0f, bH + 1.0f, argb | 0x00000000u);
    }
}