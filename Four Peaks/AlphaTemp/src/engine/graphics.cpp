// -------------------------------------------------------------------------
// Only the .cpp includes AEGraphics.h
// -------------------------------------------------------------------------
#include "../engine/graphics.hpp"
#include "AEEngine.h"
#include <cmath>
#include <cstdint>
#include <unordered_map>

#ifndef AE_PI
#define AE_PI 3.14159265358979323846f
#endif

// -------------------------------------------------------------------------
// Cache of sprite meshes keyed by a 64-bit hash of their UV coordinates.
// Avoids rebuilding identical GPU buffers every frame.
// -------------------------------------------------------------------------
static std::unordered_map<uint64_t, AEGfxVertexList*> spriteMeshCache;

static int circleSegments = 40;

namespace gfx
{
    namespace
    {
        // -------------------------------------------------------------------------
        // Shared meshes for the three primitive shapes.
        // -------------------------------------------------------------------------
        AEGfxVertexList* rectMesh{};
        AEGfxVertexList* triMesh{};
        AEGfxVertexList* circleMesh{};

        // -------------------------------------------------------------------------
        // Tracks the segment count used to build circleMesh so it is only rebuilt
        // when the requested count changes.
        // -------------------------------------------------------------------------
        int circleSegmentsBuilt{};
    }

    namespace
    {
        // -------------------------------------------------------------------------
        // Unpacks a packed ARGB colour and applies it as the blend (tint) colour.
        // -------------------------------------------------------------------------
        void setTintColor(u32 color)
        {
            u8 a = static_cast<u8>((color >> 24) & 0xFF);
            u8 r = static_cast<u8>((color >> 16) & 0xFF);
            u8 g = static_cast<u8>((color >> 8) & 0xFF);
            u8 b = static_cast<u8>((color >> 0) & 0xFF);
            AEGfxSetBlendColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        }
    }

    // -------------------------------------------------------------------------
    // Builds a TRS (translate * rotate * scale) matrix from the given components.
    // -------------------------------------------------------------------------
    AEMtx33 makeTransform(Vec2 position, f32 rotationRad, Vec2 scale)
    {
        AEMtx33 scaleMtx{};
        AEMtx33 rotMtx{};
        AEMtx33 transMtx{};
        AEMtx33 result{};

        AEMtx33Scale(&scaleMtx, scale.x, scale.y);
        AEMtx33Rot(&rotMtx, rotationRad);
        AEMtx33Trans(&transMtx, position.x, position.y);

        AEMtx33Concat(&result, &rotMtx, &scaleMtx);
        AEMtx33Concat(&result, &transMtx, &result);

        return result;
    }

    // -------------------------------------------------------------------------
    // Builds a unit quad mesh with the given UV rectangle. The quad spans
    // [-0.5, 0.5] in both axes so scaling via the transform matrix controls size.
    // -------------------------------------------------------------------------
    static AEGfxVertexList* buildSpriteMesh(f32 u0, f32 v0, f32 u1, f32 v1)
    {
        AEGfxMeshStart();
        const u32 white = 0xFFFFFFFF;

        AEGfxTriAdd(-0.5f, -0.5f, white, u0, v1,
            0.5f, -0.5f, white, u1, v1,
            -0.5f, 0.5f, white, u0, v0);

        AEGfxTriAdd(0.5f, -0.5f, white, u1, v1,
            0.5f, 0.5f, white, u1, v0,
            -0.5f, 0.5f, white, u0, v0);

        return AEGfxMeshEnd();
    }

    // -------------------------------------------------------------------------
    // Encodes four normalised UV floats into a single 64-bit integer key for
    // the sprite mesh cache. Each component is quantised to a 16-bit value.
    // -------------------------------------------------------------------------
    static uint64_t makeUVKey(f32 u0, f32 v0, f32 u1, f32 v1)
    {
        uint16_t iu0 = (uint16_t)(u0 * 65535.0f);
        uint16_t iv0 = (uint16_t)(v0 * 65535.0f);
        uint16_t iu1 = (uint16_t)(u1 * 65535.0f);
        uint16_t iv1 = (uint16_t)(v1 * 65535.0f);
        return ((uint64_t)iu0 << 48) | ((uint64_t)iv0 << 32) |
            ((uint64_t)iu1 << 16) | (uint64_t)iv1;
    }

    // -------------------------------------------------------------------------
    // Builds the rectangle and triangle shared meshes. The circle mesh is
    // created lazily on the first drawCircle call to defer the cost until needed.
    // -------------------------------------------------------------------------
    void init()
    {
        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        rectMesh = AEGfxMeshEnd();

        AEGfxMeshStart();
        AEGfxTriAdd(0.0f, 0.5f, 0xFFFFFFFF, 0.5f, 1.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
        triMesh = AEGfxMeshEnd();

        circleMesh = nullptr;
        circleSegmentsBuilt = 0;
    }

    // -------------------------------------------------------------------------
    // Releases all shared meshes and the cached sprite meshes.
    // -------------------------------------------------------------------------
    void shutdown()
    {
        if (rectMesh) { AEGfxMeshFree(rectMesh);   rectMesh = nullptr; }
        if (triMesh) { AEGfxMeshFree(triMesh);     triMesh = nullptr; }
        if (circleMesh) { AEGfxMeshFree(circleMesh);  circleMesh = nullptr; }

        for (auto& pair : spriteMeshCache) {
            if (pair.second) AEGfxMeshFree(pair.second);
        }
        spriteMeshCache.clear();
    }

    // -------------------------------------------------------------------------
    // Converts degrees to radians.
    // -------------------------------------------------------------------------
    f32 degToRad(f32 degrees)
    {
        constexpr f32 pi = 3.14159265358979323846f;
        return degrees * (pi / 180.0f);
    }

    // -------------------------------------------------------------------------
    // Draws a solid-colour rectangle. Resets multiply/add colour modifiers to
    // neutral values first to avoid bleeding from previous draw calls.
    // -------------------------------------------------------------------------
    void drawRectangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        u8 a = static_cast<u8>((color >> 24) & 0xFF);
        u8 r = static_cast<u8>((color >> 16) & 0xFF);
        u8 g = static_cast<u8>((color >> 8) & 0xFF);
        u8 b = static_cast<u8>((color >> 0) & 0xFF);

        AEGfxSetBlendColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        AEGfxSetTransparency(a / 255.0f);

        AEMtx33 m = makeTransform(position, rotationRad, size);
        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
    }

    // -------------------------------------------------------------------------
    // Draws a solid-colour triangle using the shared triangle mesh.
    // -------------------------------------------------------------------------
    void drawTriangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        AEMtx33 m = makeTransform(position, rotationRad, size);
        AEGfxSetTransform(m.m);

        setTintColor(color);
        AEGfxMeshDraw(triMesh, AE_GFX_MDM_TRIANGLES);
    }

    // -------------------------------------------------------------------------
    // Draws a filled circle as a triangle fan. Rebuilds the circle mesh only
    // when the requested segment count differs from the previously built mesh.
    // -------------------------------------------------------------------------
    void drawCircle(Vec2 position, f32 rotationRad, f32 radius, u32 color, int segments)
    {
        if (segments <= 0) segments = 32;

        if (!circleMesh || circleSegmentsBuilt != segments)
        {
            if (circleMesh) AEGfxMeshFree(circleMesh);

            AEGfxMeshStart();
            const float step = 2.0f * static_cast<float>(AE_PI) / segments;

            for (int i = 0; i < segments; ++i)
            {
                float a0 = i * step;
                float a1 = (i + 1) * step;

                // -------------------------------------------------------------------------
                // Unit-radius circle; scaled by radius via the transform matrix.
                // -------------------------------------------------------------------------
                float x0 = std::cos(a0) * 0.5f;
                float y0 = std::sin(a0) * 0.5f;
                float x1 = std::cos(a1) * 0.5f;
                float y1 = std::sin(a1) * 0.5f;

                AEGfxTriAdd(0.0f, 0.0f, 0xFFFFFFFF, 0.5f, 0.5f,
                    x0, y0, 0xFFFFFFFF, 0.5f + x0, 0.5f + y0,
                    x1, y1, 0xFFFFFFFF, 0.5f + x1, 0.5f + y1);
            }

            circleMesh = AEGfxMeshEnd();
            circleSegmentsBuilt = segments;
        }

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);

        Vec2 scale{ radius * 2.0f, radius * 2.0f };
        AEMtx33 m = makeTransform(position, rotationRad, scale);
        AEGfxSetTransform(m.m);

        setTintColor(color);
        AEGfxMeshDraw(circleMesh, AE_GFX_MDM_TRIANGLES);
    }

    // -------------------------------------------------------------------------
    // Draws a textured sprite quad. Looks up the UV-keyed mesh from the cache
    // (building and storing it on first use) to avoid redundant GPU allocations.
    // -------------------------------------------------------------------------
    void drawSprite(AEGfxTexture* tex, Vec2 position, f32 rotationRad, Vec2 size,
        f32 u0, f32 v0, f32 u1, f32 v1)
    {
        if (!tex) return;

        uint64_t key = makeUVKey(u0, v0, u1, v1);
        auto it = spriteMeshCache.find(key);
        AEGfxVertexList* mesh;

        if (it != spriteMeshCache.end()) {
            mesh = it->second;
        }
        else {
            mesh = buildSpriteMesh(u0, v0, u1, v1);
            spriteMeshCache[key] = mesh;
        }

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1, 1, 1, 1);
        AEGfxSetColorToAdd(0, 0, 0, 0);

        AEGfxTextureSet(tex, 0, 0);

        AEMtx33 m = makeTransform(position, rotationRad, size);
        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
    }

    // -------------------------------------------------------------------------
    // Overload of drawSprite with a uniform alpha value in [0, 1].
    // Used for player dash trail ghosts and other fade effects.
    // -------------------------------------------------------------------------
    void drawSprite(AEGfxTexture* tex, Vec2 position, f32 rotationRad, Vec2 size,
        f32 u0, f32 v0, f32 u1, f32 v1, float alpha)
    {
        if (!tex) return;

        uint64_t key = makeUVKey(u0, v0, u1, v1);
        auto it = spriteMeshCache.find(key);
        AEGfxVertexList* mesh;
        if (it != spriteMeshCache.end()) mesh = it->second;
        else { mesh = buildSpriteMesh(u0, v0, u1, v1); spriteMeshCache[key] = mesh; }

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(alpha);
        AEGfxSetColorToMultiply(1, 1, 1, 1);
        AEGfxSetColorToAdd(0, 0, 0, 0);
        AEGfxTextureSet(tex, 0, 0);
        AEMtx33 m = makeTransform(position, rotationRad, size);
        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
    }

    // -------------------------------------------------------------------------
    // Draws a textured sprite with a packed-ARGB multiply tint. The tint colour
    // is applied as a per-channel multiply, allowing hit-flash or environmental
    // colour effects. Tint is reset to white after drawing so subsequent calls
    // are not accidentally affected.
    // -------------------------------------------------------------------------
    void drawSpriteTinted(AEGfxTexture* tex, Vec2 position, f32 rotationRad, Vec2 size,
        f32 u0, f32 v0, f32 u1, f32 v1, u32 multiplyColor)
    {
        if (!tex) return;

        uint64_t key = makeUVKey(u0, v0, u1, v1);
        auto it = spriteMeshCache.find(key);
        AEGfxVertexList* mesh;

        if (it != spriteMeshCache.end())
            mesh = it->second;
        else {
            mesh = buildSpriteMesh(u0, v0, u1, v1);
            spriteMeshCache[key] = mesh;
        }

        // -------------------------------------------------------------------------
        // Unpack the ARGB tint colour into normalised float components.
        // -------------------------------------------------------------------------
        const float a = ((multiplyColor >> 24) & 0xFF) / 255.0f;
        const float r = ((multiplyColor >> 16) & 0xFF) / 255.0f;
        const float g = ((multiplyColor >> 8) & 0xFF) / 255.0f;
        const float b = ((multiplyColor >> 0) & 0xFF) / 255.0f;

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(a);
        AEGfxSetColorToMultiply(r, g, b, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        AEGfxTextureSet(tex, 0, 0);

        AEMtx33 m = makeTransform(position, rotationRad, size);
        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);

        // -------------------------------------------------------------------------
        // Restore neutral multiply so subsequent draw calls are unaffected.
        // -------------------------------------------------------------------------
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    }
}