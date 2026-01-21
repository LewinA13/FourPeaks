// Only the .cpp includes AEGraphics.h
#include "graphics.hpp"
#include "AEEngine.h"
#include <cmath>
#include <cstdint>

#ifndef AE_PI
#define AE_PI 3.14159265358979323846f
#endif

namespace gfx
{
    namespace
    {
        AEGfxVertexList* rectMesh{};
        AEGfxVertexList* triMesh{};
        AEGfxVertexList* circleMesh{};
        int circleSegmentsBuilt{};

        // Sprite mesh (unit quad centered at origin). We rebuild only when UVs change.
        AEGfxVertexList* spriteMesh{};
        f32 spriteU0{}, spriteV0{}, spriteU1{}, spriteV1{};
        bool spriteUvValid{};
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

        // Sprite mesh built lazily (first drawSprite call)
        spriteMesh = nullptr;
        spriteUvValid = false;
    }

    void shutdown()
    {
        if (rectMesh)   AEGfxMeshFree(rectMesh);
        if (triMesh)    AEGfxMeshFree(triMesh);
        if (circleMesh) AEGfxMeshFree(circleMesh);
        if (spriteMesh) AEGfxMeshFree(spriteMesh);

        rectMesh = nullptr;
        triMesh = nullptr;
        circleMesh = nullptr;
        circleSegmentsBuilt = 0;

        spriteMesh = nullptr;
        spriteUvValid = false;
    }

    namespace
    {
        void setTintColor(u32 color)
        {
            u8 a = static_cast<u8>((color >> 24) & 0xFF);
            u8 r = static_cast<u8>((color >> 16) & 0xFF);
            u8 g = static_cast<u8>((color >> 8) & 0xFF);
            u8 b = static_cast<u8>((color >> 0) & 0xFF);

            AEGfxSetBlendColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        }

        // Build a unit sprite quad mesh centered at origin using given UVs.
        AEGfxVertexList* buildSpriteMesh(f32 u0, f32 v0, f32 u1, f32 v1)
        {
            AEGfxMeshStart();
            const u32 white = 0xFFFFFFFF;

            // 2 triangles, unit quad centered at origin
            // Note: v0 is top, v1 is bottom.
            AEGfxTriAdd(
                -0.5f, -0.5f, white, u0, v1,
                0.5f, -0.5f, white, u1, v1,
                -0.5f, 0.5f, white, u0, v0);

            AEGfxTriAdd(
                0.5f, -0.5f, white, u1, v1,
                0.5f, 0.5f, white, u1, v0,
                -0.5f, 0.5f, white, u0, v0);

            return AEGfxMeshEnd();
        }
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

        // Rebuild mesh only if UVs changed
        if (!spriteUvValid || u0 != spriteU0 || v0 != spriteV0 || u1 != spriteU1 || v1 != spriteV1)
        {
            if (spriteMesh) AEGfxMeshFree(spriteMesh);
            spriteMesh = buildSpriteMesh(u0, v0, u1, v1);

            spriteU0 = u0; spriteV0 = v0; spriteU1 = u1; spriteV1 = v1;
            spriteUvValid = true;
        }

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        // White = no tint
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        // Bind texture
        AEGfxTextureSet(tex, 0, 0);

        // Transform and draw
        AEMtx33 m = makeTransform(position, rotationRad, size);
        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(spriteMesh, AE_GFX_MDM_TRIANGLES);
    }
}
