// ----------------------------------------------------------------------------
// Done By: Arun
// ----------------------------------------------------------------------------

#pragma once

// -------------------------------------------------------------------------
// graphics.hpp intentionally does NOT include AEGraphics.h directly.
// If the wrong AEGraphics.h is resolved first it defines AE_GRAPHICS_H and
// the real v3.12 header gets skipped, causing an AEGfxPrint signature mismatch.
// -------------------------------------------------------------------------

#include <AEEngine.h>
#include <AETypes.h>
#include <AEMtx33.h>

namespace gfx
{
    // -------------------------------------------------------------------------
    // Simple 2-D vector used throughout the graphics API.
    // -------------------------------------------------------------------------
    struct Vec2
    {
        f32 x{};
        f32 y{};
    };

    // -------------------------------------------------------------------------
    // Creates the shared mesh objects (rectangle, triangle) used by the draw
    // functions. Must be called once during application startup before any
    // draw function is invoked.
    // -------------------------------------------------------------------------
    void init();

    // -------------------------------------------------------------------------
    // Frees all internally allocated mesh objects and clears the sprite-mesh
    // cache. Call once during application shutdown to avoid memory leaks.
    // -------------------------------------------------------------------------
    void shutdown();

    // -------------------------------------------------------------------------
    // Converts an angle from degrees to radians.
    // -------------------------------------------------------------------------
    f32 degToRad(f32 degrees);

    // -------------------------------------------------------------------------
    // Builds a 2-D TRS (translate-rotate-scale) matrix from the given
    // world-space position, rotation (radians), and scale. Order is T * R * S.
    // -------------------------------------------------------------------------
    AEMtx33 makeTransform(Vec2 position, f32 rotationRad, Vec2 scale);

    // -------------------------------------------------------------------------
    // Draws a solid-colour axis-aligned (or rotated) rectangle at the given
    // world-space position. size is the full width and height.
    // color is packed ARGB.
    // -------------------------------------------------------------------------
    void drawRectangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color);

    // -------------------------------------------------------------------------
    // Draws a solid-colour triangle centred at position. size is the full
    // bounding-box dimensions. color is packed ARGB.
    // -------------------------------------------------------------------------
    void drawTriangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color);

    // -------------------------------------------------------------------------
    // Draws a filled circle centred at position with the given radius.
    // segments controls tessellation quality (default: 32 if <= 0).
    // The mesh is rebuilt automatically when segment count changes.
    // -------------------------------------------------------------------------
    void drawCircle(Vec2 position, f32 rotationRad, f32 radius, u32 color, int segments = 0);

    // -------------------------------------------------------------------------
    // Draws a textured sprite quad at position with the given size.
    // UV coordinates (u0, v0) are the top-left and (u1, v1) the bottom-right
    // of the source rectangle in normalised texture space [0, 1].
    // Meshes are cached by UV key so repeated calls with the same UVs reuse
    // the existing GPU buffer.
    // -------------------------------------------------------------------------
    void drawSprite(AEGfxTexture* tex, Vec2 position, f32 rotationRad, Vec2 size,
        f32 u0, f32 v0, f32 u1, f32 v1);

    // -------------------------------------------------------------------------
    // Overload of drawSprite that applies a uniform transparency (alpha [0, 1]).
    // Used for dash trail ghost effects where partial transparency is required.
    // -------------------------------------------------------------------------
    void drawSprite(AEGfxTexture* tex, Vec2 position, f32 rotationRad, Vec2 size,
        f32 u0, f32 v0, f32 u1, f32 v1, float alpha);

    // -------------------------------------------------------------------------
    // Draws a textured sprite with a packed-ARGB multiply tint applied on top.
    // The tint colour modulates each pixel's RGB channels, allowing hit-flash,
    // ice, or other surface effects without a separate texture. Resets tint to
    // white (1,1,1,1) after drawing so subsequent calls are unaffected.
    // -------------------------------------------------------------------------
    void drawSpriteTinted(AEGfxTexture* tex, Vec2 position, f32 rotationRad, Vec2 size,
        f32 u0, f32 v0, f32 u1, f32 v1, u32 multiplyColor);
}