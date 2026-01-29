#include "sprite.hpp"
#include <AEGraphics.h>

namespace sprite
{
    namespace
    {
        AEGfxTexture* tilesetTex{};
        AEGfxTexture* spikesTex{};
        AEGfxTexture* backgroundTex{};



        constexpr float texW = 224.0f;
        constexpr float texH = 320.0f;

        // seam fix (no filtering API available)
        constexpr float inset = 1.0f;

        void uvFromPixels(float px, float py, float pw, float ph,
            float& u0, float& v0, float& u1, float& v1)
        {
            float const x0 = px + inset;
            float const x1 = px + pw - inset;

            float const y0 = py + inset;        // top
            float const y1 = py + ph - inset;   // bottom

            u0 = x0 / texW;
            u1 = x1 / texW;
            v0 = y0 / texH;
            v1 = y1 / texH;
        }

    }

    void init()
    {
        backgroundTex = AEGfxTextureLoad("Assets/BG.png");
        if (!backgroundTex)
            backgroundTex = AEGfxTextureLoad("BG.png");

        if (tilesetTex) return;

        // Try common locations
        tilesetTex = AEGfxTextureLoad("Assets/winter_.png");
        if (!tilesetTex)
            tilesetTex = AEGfxTextureLoad("winter_.png");

        // NEW: spikes texture
        spikesTex = AEGfxTextureLoad("Assets/idle.png");
        if (!spikesTex)
            spikesTex = AEGfxTextureLoad("idle.png");
    }

    void shutdown()
    {
        if (tilesetTex)
        {
            AEGfxTextureUnload(tilesetTex);
            tilesetTex = nullptr;
        }

        if (spikesTex)
        {
            AEGfxTextureUnload(spikesTex);
            spikesTex = nullptr;
        }

        if (backgroundTex)
        {
            AEGfxTextureUnload(backgroundTex);
            backgroundTex = nullptr;
        }

    }

    AEGfxTexture* tileset()
    {
        return tilesetTex;
    }

    AEGfxTexture* spikes()
    {
        return spikesTex;
    }

    AEGfxTexture* background()
    {
        return backgroundTex;
    }




    bool getTileUv(int tileType, float& u0, float& v0, float& u1, float& v1)
    {
        switch (tileType)
        {
            // keep your old tile 1 if you still use it:
            // bottom-left 32x32 in this sheet is x=0,y=288
        case 1: uvFromPixels(0, 288, 32, 32, u0, v0, u1, v1); return true;

            // New tiles (leave 2 for spikes)
        // New tiles (32x32 versions from the spritesheet)
        case 3: uvFromPixels(144, 48, 32, 32, u0, v0, u1, v1); return true; // brown (32x32 version of big)
        case 4: uvFromPixels(144, 128, 32, 32, u0, v0, u1, v1); return true; // grey  (32x32 version of big)
        case 5: uvFromPixels(144, 288, 32, 32, u0, v0, u1, v1); return true; // ice   (32x32 version of big)

        case 6: uvFromPixels(0, 128, 32, 32, u0, v0, u1, v1); return true; // grey (your red 4)
        case 7: uvFromPixels(48, 128, 32, 32, u0, v0, u1, v1); return true; // grey+ice (your red 5)

        }

        return false;
    }
}
