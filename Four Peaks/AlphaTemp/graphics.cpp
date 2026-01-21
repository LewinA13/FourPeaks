

// Only the .cpp includes AEGraphics.h
#include "graphics.hpp"
#include "AEEngine.h"   
#include <cmath>
#include <cstdint>



#ifndef AE_PI
#define AE_PI 3.14159265358979323846f
#endif
    // player sprite mesh
    static AEGfxVertexList* spriteMesh = nullptr;

    static int circleSegments = 40;

namespace gfx
{
    namespace
    {
        AEGfxVertexList* rectMesh{};
        AEGfxVertexList* triMesh{};
        AEGfxVertexList* circleMesh{};
        int circleSegmentsBuilt{};
    }

    namespace
    {
        void setTintColor(u32 color)
        {
            u8 a = static_cast<u8>((color >> 24) & 0xFF);  // Alpha
            u8 r = static_cast<u8>((color >> 16) & 0xFF);  // Red
            u8 g = static_cast<u8>((color >> 8) & 0xFF);   // Green
            u8 b = static_cast<u8>((color >> 0) & 0xFF);   // Blue
            AEGfxSetBlendColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        }
    }

    f32 degToRad(f32 degrees)
    {
        return degrees * (static_cast<f32>(AE_PI) / 180.0f);
    }

    AEMtx33 makeTransform(Vec2 position, f32 rotationRad, Vec2 scale)
    {
        AEMtx33 scaleMtx{};
        AEMtx33 rotMtx{};
        AEMtx33 transMtx{};
        AEMtx33 result{};

        AEMtx33Scale(&scaleMtx, scale.x, scale.y);
        AEMtx33Rot(&rotMtx, rotationRad);
        AEMtx33Trans(&transMtx, position.x, position.y);

        // result = trans * rot * scale
        AEMtx33Concat(&result, &rotMtx, &scaleMtx);
        AEMtx33Concat(&result, &transMtx, &result);

        return result;
    }

    static u8 getA(u32 c) { return static_cast<u8>((c >> 24) & 0xFF); }
    static u8 getR(u32 c) { return static_cast<u8>((c >> 16) & 0xFF); }
    static u8 getG(u32 c) { return static_cast<u8>((c >> 8) & 0xFF); }
    static u8 getB(u32 c) { return static_cast<u8>((c >> 0) & 0xFF); }

    static u32 toAERGBA(u32 argb)
    {
        return (static_cast<u32>(getR(argb)) << 24) |
            (static_cast<u32>(getG(argb)) << 16) |
            (static_cast<u32>(getB(argb)) << 8) |
            (static_cast<u32>(getA(argb)) << 0);
    }

    void init()
    {
        // Rectangle mesh (unit square centered at origin)
        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 1.0f);
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        rectMesh = AEGfxMeshEnd();

        // Triangle mesh
        AEGfxMeshStart();
        AEGfxTriAdd(0.0f, 0.5f, 0xFFFFFFFF, 0.5f, 1.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
        triMesh = AEGfxMeshEnd();

        // Circle mesh built lazily (first drawCircle call)
        circleMesh = nullptr;
        circleSegmentsBuilt = 0;
    }

    void shutdown()
    {
        if (rectMesh) AEGfxMeshFree(rectMesh);
        if (triMesh) AEGfxMeshFree(triMesh);
        if (circleMesh) AEGfxMeshFree(circleMesh);

        rectMesh = nullptr;
        triMesh = nullptr;
        circleMesh = nullptr;
        circleSegmentsBuilt = 0;
    }

    static AEGfxVertexList* buildSpriteMesh(f32 u0, f32 v0, f32 u1, f32 v1)
    {
        AEGfxMeshStart();
        const u32 white = 0xFFFFFFFF;

        // 2 triangles, unit quad centered at origin
        AEGfxTriAdd(
            -0.5f, -0.5f, white, u0, v1,
            0.5f, -0.5f, white, u1, v1,
            -0.5f, 0.5f, white, u0, v0
        );

        AEGfxTriAdd(
            0.5f, -0.5f, white, u1, v1,
            0.5f, 0.5f, white, u1, v0,
            -0.5f, 0.5f, white, u0, v0
        );

        return AEGfxMeshEnd();
    }

    // ============================================
    // Public API
    // ============================================

    void init()
    {
        if (rectMesh) return;

        rectMesh = buildRectangleMesh();
        triMesh = buildTriangleMesh();
        circleMesh = buildCircleMesh(circleSegments);
    }

    void shutdown()
    {
        if (rectMesh) { AEGfxMeshFree(rectMesh); rectMesh = nullptr; }
        if (triMesh) { AEGfxMeshFree(triMesh); triMesh = nullptr; }
        if (circleMesh) { AEGfxMeshFree(circleMesh); circleMesh = nullptr; }
        if (spriteMesh) { AEGfxMeshFree(spriteMesh); spriteMesh = nullptr; }
    }

    f32 degToRad(f32 degrees)
    {
        constexpr f32 pi = 3.14159265358979323846f;
        return degrees * (pi / 180.0f);
    }

    void drawRectangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);

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


    void drawTriangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);

        AEMtx33 m = makeTransform(position, rotationRad, size);
        AEGfxSetTransform(m.m);

        setTintColor(color);
        AEGfxMeshDraw(triMesh, AE_GFX_MDM_TRIANGLES);
    }

    void drawCircle(Vec2 position, f32 rotationRad, f32 radius, u32 color, int segments)
    {
        if (segments <= 0) segments = 32;

        // Rebuild mesh only if segment count changes
        if (!circleMesh || circleSegmentsBuilt != segments)
        {
            if (circleMesh) AEGfxMeshFree(circleMesh);

            AEGfxMeshStart();
            const float step = 2.0f * static_cast<float>(AE_PI) / segments;

            for (int i = 0; i < segments; ++i)
            {
                float a0 = i * step;
                float a1 = (i + 1) * step;

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

    void drawSprite(AEGfxTexture* tex, Vec2 position, f32 rotationRad, Vec2 size,
        f32 u0, f32 v0, f32 u1, f32 v1)
    {
        if (!tex) return;

        // rebuild mesh for these UVs (simple + clear for now)
        if (spriteMesh) { AEGfxMeshFree(spriteMesh); spriteMesh = nullptr; }
        spriteMesh = buildSpriteMesh(u0, v0, u1, v1);

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        // White = no tint
        AEGfxSetColorToMultiply(1, 1, 1, 1);
        AEGfxSetColorToAdd(0, 0, 0, 0);

        AEGfxTextureSet(tex, 0, 0);

        drawMesh(spriteMesh, makeTransform(position, rotationRad, size));
    }
}
