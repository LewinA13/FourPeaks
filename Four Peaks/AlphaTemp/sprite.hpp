#pragma once

#include <AEEngine.h> // AEGfxTexture

namespace sprite
{
    void init();
    void shutdown();

    // tileset texture (winter_.png)
    AEGfxTexture* tileset();

    // NEW: spikes texture (idle.png)
    AEGfxTexture* spikes();

    AEGfxTexture* background();

    // UVs from tileset
    bool getTileUv(int tileType, float& u0, float& v0, float& u1, float& v1);
}
