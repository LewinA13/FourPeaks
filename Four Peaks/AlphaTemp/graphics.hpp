#pragma once

// graphics.hpp should NOT include AEGraphics.h.
// Reason: if a wrong AEGraphics.h is picked up first, it defines AE_GRAPHICS_H,
// and then the real v3.12 AEGraphics.h gets skipped, causing AEGfxPrint mismatch.

#include <AEEngine.h>

#include <AETypes.h>   // f32, u32
#include <AEMtx33.h>   // AEMtx33

namespace gfx
{
    struct Vec2
    {
        f32 x{};
        f32 y{};
    };

    // These functions are implemented in graphics.cpp
    void init();
    void shutdown();

    f32 degToRad(f32 degrees);
    AEMtx33 makeTransform(Vec2 position, f32 rotationRad, Vec2 scale);

    void drawRectangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color);
    void drawTriangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color);
    void drawCircle(Vec2 position, f32 rotationRad, f32 radius, u32 color, int segments = 0);
}
