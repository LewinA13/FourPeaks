#pragma once
#include "AEEngine.h"

// Simple graphics helper for Alpha Engine
// Reusable shapes with position, rotation, and scale

namespace gfx
{
    // Simple 2D vector
    struct Vec2
    {
        f32 x{};
        f32 y{};
    };

    // Call once after AEInit()
    void init();

    // Call once before program exit
    void shutdown();

    // Convert degrees to radians
    f32 degToRad(f32 degrees);

    // Create transform matrix: scale -> rotate -> translate
    AEMtx33 makeTransform(Vec2 position, f32 rotationRad, Vec2 scale);

    // Draw filled shapes
    void drawRectangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color);
    void drawTriangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color);
    void drawCircle(Vec2 position, f32 rotationRad, f32 radius, u32 color, int segments = 0);
}
