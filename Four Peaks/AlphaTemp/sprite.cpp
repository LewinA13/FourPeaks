#include "sprite.hpp"
#include <AEGraphics.h>
#include "graphics.hpp"




namespace sprite
{


    namespace
    {
        AEGfxTexture* tilesetTex{};
        AEGfxTexture* spikesTex{};
        AEGfxTexture* backgroundTex{};
        AEGfxTexture* tutorialBgTex{};
        AEGfxTexture* coinTex{};
        AEGfxTexture* bgStripTex{};
        AEGfxTexture* iceTex{};
        AEGfxTexture* checkpointTex{};
        AEGfxTexture* crackTex{};
        AEGfxTexture* heatbarTex{};

        constexpr float texW = 224.0f;
        constexpr float texH = 320.0f;

        // seam fix (no filtering API available)
        constexpr float inset = 1.0f;

        // Helper: get UVs from pixel coordinates in the tileset
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

        // ---------------------------------------------------------
        // Shared animation state for tiles (used across all stages)
        // ---------------------------------------------------------
        int coinFrame = 0;
        float coinTimer = 0.0f;
        // Melon.png is 544x32, 17 frames, each 32x32 (horizontal strip)
        constexpr int coinFrameCount = 17;
        constexpr float coinFrameTime = 0.08f; // tweak if you want faster/slower


        int checkpointFrame = 0;
        float checkpointTimer = 0.0f;
        constexpr int checkpointFrameCount = 10;
        constexpr float checkpointFrameTime = 0.09f;

        // ice crack sprite sheet is 224x32, 7 frames, each 32x32 (horizontal strip)
        int crackFrame = 0;
        float crackTimer = 0.0f;
        
      

    }

    void init()
    {
        backgroundTex = AEGfxTextureLoad("Assets/BG.png");
        if (!backgroundTex)
            backgroundTex = AEGfxTextureLoad("BG.png");

        // Tutorial background (3 tutorial stages)
        tutorialBgTex = AEGfxTextureLoad("Assets/bg_tutorial.png");
        if (!tutorialBgTex)
            tutorialBgTex = AEGfxTextureLoad("bg_tutorial.png");

        if (tilesetTex) return;

        // Winter spritesheet
        tilesetTex = AEGfxTextureLoad("Assets/winter_.png");
        if (!tilesetTex)
            tilesetTex = AEGfxTextureLoad("winter_.png");

        // spikes texture
        spikesTex = AEGfxTextureLoad("Assets/idle.png");
        if (!spikesTex)
            spikesTex = AEGfxTextureLoad("idle.png");

        // coin spritesheet texture
        coinTex = AEGfxTextureLoad("Assets/Melon.png");
        if (!coinTex)
            coinTex = AEGfxTextureLoad("Melon.png");

        // ice texture
        iceTex = AEGfxTextureLoad("Assets/IceBox.png");
        if (!iceTex)
            iceTex = AEGfxTextureLoad("IceBox.png");

        // checkpoint spritesheet texture
        checkpointTex = AEGfxTextureLoad("Assets/Checkpoint.png");
        if (!checkpointTex)
            checkpointTex = AEGfxTextureLoad("Checkpoint.png");

        // ice crack spritesheet texture
        crackTex = AEGfxTextureLoad("Assets/b_ice.jpg");
        if (!crackTex)
            crackTex = AEGfxTextureLoad("b_ice.jpg");

        heatbarTex = AEGfxTextureLoad("Assets/player/heatbar.png");
        if (!heatbarTex)
            heatbarTex = AEGfxTextureLoad("heatbar.png");

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

        if (tutorialBgTex)
        {
            AEGfxTextureUnload(tutorialBgTex);
            tutorialBgTex = nullptr;
        }

        if (coinTex)
        {
            AEGfxTextureUnload(coinTex);
            coinTex = nullptr;
        }

        if (iceTex)
        {
            AEGfxTextureUnload(iceTex);
            iceTex = nullptr;
        }

        if (checkpointTex)
        {
            AEGfxTextureUnload(checkpointTex);
            checkpointTex = nullptr;
        }

        if (crackTex)
        {
            AEGfxTextureUnload(crackTex);
            crackTex = nullptr;
        }
        if (heatbarTex)
        {
            AEGfxTextureUnload(heatbarTex);
            heatbarTex = nullptr;
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

    AEGfxTexture* tutorialBackground()
    {
        return tutorialBgTex;
    }

    AEGfxTexture* coin()
    {
        return coinTex;
    }

    AEGfxTexture* ice()
    {
        return iceTex;
    }

    AEGfxTexture* checkpoint()
    {
        return checkpointTex;
    }

    AEGfxTexture* crack()
    {
        return crackTex;
    }


    bool getCoinUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        // Melon.png = 544x32
        // 17 frames, each 32x32, laid horizontally
        constexpr int frameCount = 17;
        constexpr float sheetW = 544.0f;
        constexpr float sheetH = 32.0f;
        constexpr float frameW = 32.0f;
        constexpr float frameH = 32.0f;

        if (frame < 0) frame = 0;
        frame %= frameCount;

        // small inset to reduce bleeding between frames
        constexpr float insetPx = 0.5f;

        float px = frame * frameW;
        float py = 0.0f;

        float x0 = px + insetPx;
        float x1 = px + frameW - insetPx;
        float y0 = py + insetPx;
        float y1 = py + frameH - insetPx;

        u0 = x0 / sheetW;
        u1 = x1 / sheetW;
        v0 = y0 / sheetH;
        v1 = y1 / sheetH;

        return true;
    }


    // UVs for checkpoint animation frames
    bool getCheckpointUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        // Checkpoint.png = 320x32 (10 frames, each 32x32 laid horizontally)
        constexpr int frameCount = 10;
        constexpr float sheetW = 320.0f;
        constexpr float sheetH = 32.0f;
        constexpr float frameW = 32.0f;
        constexpr float frameH = 32.0f;

        if (frame < 0) frame = 0;
        frame %= frameCount;

        // Small inset to reduce texture bleeding
        constexpr float insetPx = 0.5f;

        float px = frame * frameW;
        float py = 0.0f;

        float x0 = px + insetPx;
        float x1 = px + frameW - insetPx;
        float y0 = py + insetPx;
        float y1 = py + frameH - insetPx;

        u0 = x0 / sheetW;
        u1 = x1 / sheetW;
        v0 = y0 / sheetH;
        v1 = y1 / sheetH;

        return true;
    }

    // UVs for crack animation frames
    bool getCrackUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        // cf985dfb-...jpg = 224x32 (7 frames, each 32x32 laid horizontally)
        constexpr int frameCount = 7;
        constexpr float sheetW = 160.0f;
        constexpr float sheetH = 32.0f;
        constexpr float frameW = 32.0f;
        constexpr float frameH = 32.0f;

        if (frame < 0) frame = 0;
        frame %= frameCount;

        // Small inset to reduce bleeding
        constexpr float insetPx = 0.5f;

        float px = frame * frameW;
        float py = 0.0f;

        float x0 = px + insetPx;
        float x1 = px + frameW - insetPx;
        float y0 = py + insetPx;
        float y1 = py + frameH - insetPx;

        u0 = x0 / sheetW;
        u1 = x1 / sheetW;
        v0 = y0 / sheetH;
        v1 = y1 / sheetH;

        return true;
    }


    void updateAnimatedTiles(f32 dt)
    {
        // coin
        coinTimer += dt;
        while (coinTimer >= coinFrameTime)
        {
            coinTimer -= coinFrameTime;
            coinFrame = (coinFrame + 1) % coinFrameCount;
        }

        // checkpoint
        checkpointTimer += dt;
        while (checkpointTimer >= checkpointFrameTime)
        {
            checkpointTimer -= checkpointFrameTime;
            checkpointFrame = (checkpointFrame + 1) % checkpointFrameCount;
        }
    }

    bool drawAnimatedTile(int tileType, gfx::Vec2 pos, gfx::Vec2 size)
    {
        // Tile IDs in your project:
        // 8  = coin
        // 10 = checkpoint
        if (tileType == 8)
        {
            if (!coinTex) return true;

            float u0{}, v0{}, u1{}, v1{};
            getCoinUv(coinFrame, u0, v0, u1, v1);

            // melon size
            gfx::Vec2 coinSize{ size.x * 0.9f, size.y * 0.9f };
            gfx::drawSprite(coinTex, pos, 0.0f, coinSize, u0, v0, u1, v1);
            return true;
        }

        if (tileType == 10)
        {
            if (!checkpointTex) return true;

            float u0{}, v0{}, u1{}, v1{};
            getCheckpointUv(checkpointFrame, u0, v0, u1, v1);

            gfx::Vec2 cpSize{ size.x * 0.85f, size.y * 0.85f };
            gfx::drawSprite(checkpointTex, pos, 0.0f, cpSize, u0, v0, u1, v1);
            return true;
        }

        return false;
    }


    // UVs from tileset
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

    AEGfxTexture* heatbar() { return heatbarTex; }

    bool getHeatBarUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        if (frame < 0 || frame > 5) return false;   // was 11
        constexpr float frameW = 48.0f / 384.0f;    // was 32.0f / 384.0f
        u0 = frame * frameW;
        u1 = u0 + frameW;
        v0 = 0.5f;
        v1 = 1.0f;
        return true;
    }
}
