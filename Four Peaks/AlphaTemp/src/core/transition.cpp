// ---------------------------------------------------------------------------
// transition.cpp  -  Simple fade-to-black screen transition
// ---------------------------------------------------------------------------
// Replaces the old PokemonWipe / Dissolve system.
// This version draws exactly ONE full-screen quad per frame using a single
// pre-built mesh, so it has near-zero performance cost and works on every
// stage in every season.
//
// Phase timeline (all durations divided by speed multiplier):
//   FadeIn  - alpha 0 -> 1   (0.30 s at speed 2.5)
//   Hold    - alpha 1        (0.10 s at speed 2.5)  <- scene switch fires here
//   FadeOut - alpha 1 -> 0   (0.30 s at speed 2.5)
// ---------------------------------------------------------------------------

#define NOMINMAX
#include "core/transition.hpp"
#include "engine/camera.hpp"
#include "engine/graphics.hpp"
#include "AEEngine.h"
#include <cstdint>
#include <algorithm>

typedef std::uint32_t u32;

// Phase durations in seconds (before applying speed multiplier)
static constexpr float FADE_IN_DURATION = 0.75f;
static constexpr float HOLD_DURATION = 0.25f;
static constexpr float FADE_OUT_DURATION = 0.75f;

// ===================================================================
// Transition::drawBlackOverlay
// Draws a single full-screen black quad at the given alpha (0=clear, 1=black).
// Uses one MeshStart/MeshEnd/MeshDraw call - fastest possible approach.
// ===================================================================
void Transition::drawBlackOverlay(float alpha)
{
    if (alpha <= 0.0f) return;
    alpha = std::min(alpha, 1.0f);

    float minX = AEGfxGetWinMinX();
    float maxX = AEGfxGetWinMaxX();
    float minY = AEGfxGetWinMinY();
    float maxY = AEGfxGetWinMaxY();

    u32 a8 = static_cast<u32>(alpha * 255.0f);
    u32 col = (a8 << 24); // ARGB: alpha, R=0, G=0, B=0

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // Two triangles covering the full screen - one draw call total
    AEGfxMeshStart();
    AEGfxTriAdd(minX, maxY, col, 0.0f, 0.0f,
        maxX, maxY, col, 0.0f, 0.0f,
        minX, minY, col, 0.0f, 0.0f);
    AEGfxTriAdd(maxX, maxY, col, 0.0f, 0.0f,
        maxX, minY, col, 0.0f, 0.0f,
        minX, minY, col, 0.0f, 0.0f);
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

// ===================================================================
// Transition::start
// Kick off a new transition. Resets state so it is safe to call mid-flight.
// ===================================================================
void Transition::start()
{
    phase = Phase::FadeIn;
    t = 0.0f;
    switchPending = false;
    switchDone = false;
}

// ===================================================================
// Transition::update
// Advance the transition timer. Call once per frame with the frame delta.
// ===================================================================
void Transition::update(float dt)
{
    if (phase == Phase::Idle) return;

    float duration = 0.0f;
    switch (phase)
    {
    case Phase::FadeIn:  duration = FADE_IN_DURATION / speed; break;
    case Phase::Hold:    duration = HOLD_DURATION / speed; break;
    case Phase::FadeOut: duration = FADE_OUT_DURATION / speed; break;
    default: break;
    }

    t += dt / duration;

    if (t >= 1.0f)
    {
        t = 0.0f;
        switch (phase)
        {
        case Phase::FadeIn:
            phase = Phase::Hold;
            switchPending = true;   // caller should switch scene NOW
            break;
        case Phase::Hold:
            phase = Phase::FadeOut;
            break;
        case Phase::FadeOut:
            phase = Phase::Idle;
            switchDone = true;
            break;
        default: break;
        }
    }
}

// ===================================================================
// Transition::draw
// Draw the black overlay on top of everything. Call LAST in your render loop.
// ===================================================================
void Transition::draw() const
{
    if (phase == Phase::Idle) return;

    float alpha = 0.0f;
    switch (phase)
    {
    case Phase::FadeIn:
        alpha = t * t * (3.0f - 2.0f * t);
        break;
    case Phase::Hold:
        alpha = 1.0f;
        break;
    case Phase::FadeOut:
        alpha = 1.0f - (t * t * (3.0f - 2.0f * t));
        break;
    default:
        return;
    }

    float camX = 0.0f;
    float camY = 0.0f;
    AEGfxGetCamPosition(&camX, &camY);

    const u32 a8 = static_cast<u32>(alpha * 255.0f);
    const u32 color = (a8 << 24);

    // draw transition using the real gameplay camera
    gfx::drawRectangle(
        { camX, camY },
        0.0f,
        { camera::screenWidth(), camera::screenHeight() },
        color
    );
}