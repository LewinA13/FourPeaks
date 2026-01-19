#include "graphics.hpp"
#include <cmath>

namespace gfx
{
    // ============================================
    // Internal data
    // ============================================

    static AEGfxVertexList* rectMesh = nullptr;
    static AEGfxVertexList* triMesh = nullptr;
    static AEGfxVertexList* circleMesh = nullptr;

    // player sprite mesh
    static AEGfxVertexList* spriteMesh = nullptr;

    static int circleSegments = 40;

    // ============================================
    // Internal helpers
    // ============================================

    // Convert ARGB color to float values
    static void colorToFloat(u32 color, f32& r, f32& g, f32& b, f32& a)
    {
        a = ((color >> 24) & 0xFF) / 255.0f;
        r = ((color >> 16) & 0xFF) / 255.0f;
        g = ((color >> 8) & 0xFF) / 255.0f;
        b = ((color >> 0) & 0xFF) / 255.0f;
    }

    // Setup render state for colored shapes
    static void setColorState(u32 color)
    {
        f32 r, g, b, a;
        colorToFloat(color, r, g, b, a);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        AEGfxSetColorToMultiply(r, g, b, a);
        AEGfxSetColorToAdd(0, 0, 0, 0);
    }

    // Draw mesh using transform (passed by value on purpose)
    static void drawMesh(AEGfxVertexList* mesh, AEMtx33 transform)
    {
        if (!mesh) return;

        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
    }

    // Build 1x1 rectangle centered at origin
    static AEGfxVertexList* buildRectangleMesh()
    {
        AEGfxMeshStart();
        const u32 white = 0xFFFFFFFF;

        AEGfxTriAdd(
            -0.5f, -0.5f, white, 0, 0,
            0.5f, -0.5f, white, 0, 0,
            -0.5f, 0.5f, white, 0, 0
        );

        AEGfxTriAdd(
            0.5f, -0.5f, white, 0, 0,
            0.5f, 0.5f, white, 0, 0,
            -0.5f, 0.5f, white, 0, 0
        );

        return AEGfxMeshEnd();
    }

    // Build simple triangle
    static AEGfxVertexList* buildTriangleMesh()
    {
        AEGfxMeshStart();
        const u32 white = 0xFFFFFFFF;

        AEGfxTriAdd(
            -0.5f, -0.5f, white, 0, 0,
            0.5f, -0.5f, white, 0, 0,
            0.0f, 0.5f, white, 0, 0
        );

        return AEGfxMeshEnd();
    }

    // Build unit circle using triangle fan
    static AEGfxVertexList* buildCircleMesh(int segments)
    {
        if (segments < 3) segments = 3;

        AEGfxMeshStart();
        const u32 white = 0xFFFFFFFF;

        const f32 radius = 0.5f;
        const f32 step = 6.2831853f / segments;

        for (int i = 0; i < segments; ++i)
        {
            f32 a0 = step * i;
            f32 a1 = step * (i + 1);

            AEGfxTriAdd(
                0.0f, 0.0f, white, 0, 0,
                cosf(a0) * radius, sinf(a0) * radius, white, 0, 0,
                cosf(a1) * radius, sinf(a1) * radius, white, 0, 0
            );
        }

        return AEGfxMeshEnd();
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

    AEMtx33 makeTransform(Vec2 position, f32 rotationRad, Vec2 scale)
    {
        AEMtx33 scaleMtx;
        AEMtx33Scale(&scaleMtx, scale.x, scale.y);

        AEMtx33 rotationMtx;
        AEMtx33Rot(&rotationMtx, rotationRad);

        AEMtx33 translationMtx;
        AEMtx33Trans(&translationMtx, position.x, position.y);

        AEMtx33 temp;
        AEMtx33Concat(&temp, &rotationMtx, &scaleMtx);

        AEMtx33 result;
        AEMtx33Concat(&result, &translationMtx, &temp);

        return result;
    }

    void drawRectangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color)
    {
        if (!rectMesh) return;

        setColorState(color);
        drawMesh(rectMesh, makeTransform(position, rotationRad, size));
    }

    void drawTriangle(Vec2 position, f32 rotationRad, Vec2 size, u32 color)
    {
        if (!triMesh) return;

        setColorState(color);
        drawMesh(triMesh, makeTransform(position, rotationRad, size));
    }

    void drawCircle(Vec2 position, f32 rotationRad, f32 radius, u32 color, int segments)
    {
        if (segments <= 0) segments = circleSegments;

        if (segments != circleSegments)
        {
            if (circleMesh)
                AEGfxMeshFree(circleMesh);

            circleSegments = segments;
            circleMesh = buildCircleMesh(circleSegments);
        }

        setColorState(color);

        Vec2 scale{ radius * 2.0f, radius * 2.0f };
        drawMesh(circleMesh, makeTransform(position, rotationRad, scale));
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
