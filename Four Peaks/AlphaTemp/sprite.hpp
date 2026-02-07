#pragma once

#include <AEEngine.h> // AEGfxTexture
#include <AETypes.h>  // f32
#include "graphics.hpp" // gfx::Vec2, drawSprite

namespace sprite
{
    void init();
    void shutdown();

    // tileset texture (winter_.png)
    AEGfxTexture* tileset();

    // NEW: spikes texture (idle.png)
    AEGfxTexture* spikes();

	// background texture (BG.png)
    AEGfxTexture* background();

    // coin spritesheet (coin_.png)
    AEGfxTexture* coin();

	// ice texture (ice1.jpg)
    AEGfxTexture* ice();

    // checkpoint spritesheet (Checkpoint.png)
    AEGfxTexture* checkpoint();

    // white wall pieces (wall ice, 4x for WINTER)
    AEGfxTexture* wallTop();
    AEGfxTexture* wallLeft();
    AEGfxTexture* wallRight();
    AEGfxTexture* wallBottom();

    // UVs for checkpoint animation frames
    bool getCheckpointUv(int frame, float& u0, float& v0, float& u1, float& v1);

    // UVs for coin animation frames 
    bool getCoinUv(int frame, float& u0, float& v0, float& u1, float& v1);

    // UVs from tileset
    bool getTileUv(int tileType, float& u0, float& v0, float& u1, float& v1);

    // Update shared animated tiles (coin/checkpoint) once per frame
    void updateAnimatedTiles(f32 dt);

    // Draw animated tiles by tileType. Returns true if it drew something.
    // Use this inside any stage tile loop.
    bool drawAnimatedTile(int tileType, gfx::Vec2 pos, gfx::Vec2 size);

}
