#include "sprite.hpp"
#include <AEGraphics.h>
#include "graphics.hpp"
#include <cmath>




namespace sprite
{


    namespace
    {
        AEGfxTexture* tilesetTex{};
        AEGfxTexture* spikesTex{};
        AEGfxTexture* backgroundTex{};
        AEGfxTexture* mmBackgroundTex{};
        AEGfxTexture* summerBgTex{};
        AEGfxTexture* tutorialBgTex{};
        AEGfxTexture* autumnBgTex{};
        AEGfxTexture* springBgTex{};
        AEGfxTexture* coinTex{};
        AEGfxTexture* bgStripTex{};
        AEGfxTexture* iceTex{};
        AEGfxTexture* checkpointTex{};
        AEGfxTexture* crackTex{};
        AEGfxTexture* heatbarTex{};
        AEGfxTexture* signTex{};
        AEGfxTexture* tile12Tex{};
        AEGfxTexture* tile02Tex{};
        AEGfxTexture* bottleTex{};
        AEGfxTexture* textboxTex{};
        AEGfxTexture* textboxUpTex{};
        AEGfxTexture* textboxDownTex{};
        AEGfxTexture* winterCTex{};   // tile ID 4 -> WinterC.png
        AEGfxTexture* winterTTex{};   // tile ID 6 -> WinterT.png


        // New tile textures (IDs 23-25)
        AEGfxTexture* grassTex{};
        AEGfxTexture* fireTex_{};
        AEGfxTexture* sawTex_{};

        // Fire animation (fire.png = 160x24, 8 frames 20x24)
        int fireFrame = 0;
        float fireTimer = 0.0f;
        constexpr int fireFrameCount = 8;
        constexpr float fireFrameTime = 0.08f;
        constexpr float fireSheetW = 160.0f;
        constexpr float fireSheetH = 24.0f;
        constexpr float fireFrameW = 20.0f;
        constexpr float fireFrameH = 24.0f;

        // Saw animation (saw.png = 304x38, 8 frames 38x38)
        int sawFrame = 0;
        float sawTimer = 0.0f;
        constexpr int sawFrameCount = 8;
        constexpr float sawFrameTime = 0.06f;
        constexpr float sawSheetW = 304.0f;
        constexpr float sawSheetH = 38.0f;
        constexpr float sawFrameW = 38.0f;
        constexpr float sawFrameH = 38.0f;

        // Standalone seasonal tile textures
        AEGfxTexture* spring1Tex{};
        AEGfxTexture* spring2Tex{};
        AEGfxTexture* autumn1Tex{};
        AEGfxTexture* autumn2Tex{};


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

        // Bottle float animation
        float bottleFloatTimer = 0.0f;
        constexpr float bottleFloatSpeed = 2.0f;   // radians per second
        constexpr float bottleFloatAmp = 6.0f;    // pixels up/down



    }

    void init()
    {
        // Always (re)load these if missing
        if (!backgroundTex) {
            backgroundTex = AEGfxTextureLoad("Assets/BG.png");
            if (!backgroundTex) backgroundTex = AEGfxTextureLoad("BG.png");
        }
        if (!mmBackgroundTex) {
            mmBackgroundTex = AEGfxTextureLoad("Assets/mmf1.png");
            if (!mmBackgroundTex) mmBackgroundTex = AEGfxTextureLoad("Assets/mmf1.png");
        }
        if (!summerBgTex) {
            summerBgTex = AEGfxTextureLoad("Assets/summerbg.png");
            if (!summerBgTex) summerBgTex = AEGfxTextureLoad("summerbg.jpg");
        }
        if (!tutorialBgTex) {
            tutorialBgTex = AEGfxTextureLoad("Assets/bg_tutorial.png");
            if (!tutorialBgTex) tutorialBgTex = AEGfxTextureLoad("bg_tutorial.png");
        }
        if (!autumnBgTex) {
            autumnBgTex = AEGfxTextureLoad("Assets/autumnbg.jpg");
            if (!autumnBgTex) autumnBgTex = AEGfxTextureLoad("autumnbg.jpg");
        }
        if (!springBgTex) {
            springBgTex = AEGfxTextureLoad("Assets/springBG2.jpg");
            if (!springBgTex) springBgTex = AEGfxTextureLoad("springBG2.jpg");
        }
        if (!tilesetTex) {
            tilesetTex = AEGfxTextureLoad("Assets/winter_.png");
            if (!tilesetTex) tilesetTex = AEGfxTextureLoad("winter_.png");
        }
        if (!spikesTex) {
            spikesTex = AEGfxTextureLoad("Assets/idle.png");
            if (!spikesTex) spikesTex = AEGfxTextureLoad("idle.png");
        }
        if (!coinTex) {
            coinTex = AEGfxTextureLoad("Assets/Melon.png");
            if (!coinTex) coinTex = AEGfxTextureLoad("Melon.png");
        }
        if (!iceTex) {
            iceTex = AEGfxTextureLoad("Assets/IceBox.png");
            if (!iceTex) iceTex = AEGfxTextureLoad("IceBox.png");
        }
        if (!checkpointTex) {
            checkpointTex = AEGfxTextureLoad("Assets/Checkpoint.png");
            if (!checkpointTex) checkpointTex = AEGfxTextureLoad("Checkpoint.png");
        }
        if (!crackTex) {
            crackTex = AEGfxTextureLoad("Assets/b_ice.jpg");
            if (!crackTex) crackTex = AEGfxTextureLoad("b_ice.jpg");
        }
        if (!heatbarTex) {
            heatbarTex = AEGfxTextureLoad("Assets/player/heatbar.png");
            if (!heatbarTex) heatbarTex = AEGfxTextureLoad("heatbar.png");
        }
        if (!signTex) {
            signTex = AEGfxTextureLoad("Assets/sign.png");
            if (!signTex) signTex = AEGfxTextureLoad("sign.jpg");
        }
        if (!tile12Tex) {
            // Support both your project asset paths and direct project-root drops.
            tile12Tex = AEGfxTextureLoad("Assets/center.png");
            if (!tile12Tex) tile12Tex = AEGfxTextureLoad("Assets/center.png");
            if (!tile12Tex) tile12Tex = AEGfxTextureLoad("Tile_12.png");
            if (!tile12Tex) tile12Tex = AEGfxTextureLoad("Tile_12.jpg");
        }
        if (!tile02Tex) {
            tile02Tex = AEGfxTextureLoad("Assets/top1.png");
            if (!tile02Tex) tile02Tex = AEGfxTextureLoad("Assets/top1.png");
            if (!tile02Tex) tile02Tex = AEGfxTextureLoad("Tile_02.png");
            if (!tile02Tex) tile02Tex = AEGfxTextureLoad("Tile_02.jpg");
        }
        if (!bottleTex) {
            bottleTex = AEGfxTextureLoad("Assets/bottle.png");
            if (!bottleTex) bottleTex = AEGfxTextureLoad("bottle.jpg");
        }
        if (!textboxTex) {
            textboxTex = AEGfxTextureLoad("Assets/textbox.png");
            if (!textboxTex) textboxTex = AEGfxTextureLoad("textbox.jpg");
        }
        if (!textboxUpTex) {
            textboxUpTex = AEGfxTextureLoad("Assets/textboxUp.png");
            if (!textboxUpTex) textboxUpTex = AEGfxTextureLoad("textboxUp.jpg");
        }
        if (!textboxDownTex) {
            textboxDownTex = AEGfxTextureLoad("Assets/textboxDown.png");
            if (!textboxDownTex) textboxDownTex = AEGfxTextureLoad("textboxDown.jpg");
        }

        // Winter tile overrides (standalone images)
        if (!winterCTex) winterCTex = AEGfxTextureLoad("Assets/WinterC.png");
        if (!winterTTex) winterTTex = AEGfxTextureLoad("Assets/WinterT.png");

        // Seasonal tile textures (standalone images)
        if (!spring1Tex) spring1Tex = AEGfxTextureLoad("Assets/spring1.png");
        if (!spring2Tex) spring2Tex = AEGfxTextureLoad("Assets/spring2.png");
        if (!autumn1Tex) autumn1Tex = AEGfxTextureLoad("Assets/autumn1.png");
        if (!autumn2Tex) autumn2Tex = AEGfxTextureLoad("Assets/autumn2.png");

        // New tile textures
        if (!grassTex)  grassTex = AEGfxTextureLoad("Assets/grasss.png");
        if (!fireTex_)  fireTex_ = AEGfxTextureLoad("Assets/fire.png");
        if (!sawTex_)   sawTex_ = AEGfxTextureLoad("Assets/saw.png");
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

        if (summerBgTex)
        {
            AEGfxTextureUnload(summerBgTex);
            summerBgTex = nullptr;
        }

        if (mmBackgroundTex)
        {
            AEGfxTextureUnload(mmBackgroundTex);
            mmBackgroundTex = nullptr;
        }

        if (tutorialBgTex)
        {
            AEGfxTextureUnload(tutorialBgTex);
            tutorialBgTex = nullptr;
        }

        if (autumnBgTex)
        {
            AEGfxTextureUnload(autumnBgTex);
            autumnBgTex = nullptr;
        }
        if (springBgTex) { AEGfxTextureUnload(springBgTex); springBgTex = nullptr; }

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
        if (signTex)
        {
            AEGfxTextureUnload(signTex);
            signTex = nullptr;
        } // make it look nc later - A
        if (tile12Tex) { AEGfxTextureUnload(tile12Tex); tile12Tex = nullptr; }
        if (tile02Tex) { AEGfxTextureUnload(tile02Tex); tile02Tex = nullptr; }
        if (bottleTex) { AEGfxTextureUnload(bottleTex); bottleTex = nullptr; }
        if (textboxTex) { AEGfxTextureUnload(textboxTex); textboxTex = nullptr; }
        if (textboxUpTex) { AEGfxTextureUnload(textboxUpTex); textboxUpTex = nullptr; }
        if (textboxDownTex) { AEGfxTextureUnload(textboxDownTex); textboxDownTex = nullptr; }

        if (winterCTex) { AEGfxTextureUnload(winterCTex); winterCTex = nullptr; }
        if (winterTTex) { AEGfxTextureUnload(winterTTex); winterTTex = nullptr; }

        if (spring1Tex) { AEGfxTextureUnload(spring1Tex); spring1Tex = nullptr; }
        if (spring2Tex) { AEGfxTextureUnload(spring2Tex); spring2Tex = nullptr; }
        if (autumn1Tex) { AEGfxTextureUnload(autumn1Tex); autumn1Tex = nullptr; }
        if (autumn2Tex) { AEGfxTextureUnload(autumn2Tex); autumn2Tex = nullptr; }

        if (grassTex) { AEGfxTextureUnload(grassTex);  grassTex = nullptr; }
        if (fireTex_) { AEGfxTextureUnload(fireTex_);  fireTex_ = nullptr; }
        if (sawTex_) { AEGfxTextureUnload(sawTex_);   sawTex_ = nullptr; }

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

    AEGfxTexture* summerBackground()
    {
        return summerBgTex;
    }

    AEGfxTexture* mmBackground()
    {
        return mmBackgroundTex;
    }

    AEGfxTexture* tutorialBackground()
    {
        return tutorialBgTex;
    }

    AEGfxTexture* autumnBackground()
    {
        return autumnBgTex;
    }

    AEGfxTexture* springBackground()
    {
        return springBgTex;
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

    // make it look nice later - Arun
    AEGfxTexture* sign() { return signTex; }
    AEGfxTexture* tile12() { return tile12Tex; }
    AEGfxTexture* tile02() { return tile02Tex; }
    AEGfxTexture* bottle() { return bottleTex; }
    AEGfxTexture* textbox() { return textboxTex; }
    AEGfxTexture* textboxUp() { return textboxUpTex; }
    AEGfxTexture* textboxDown() { return textboxDownTex; }




    AEGfxTexture* spring1() { return spring1Tex; }
    AEGfxTexture* spring2() { return spring2Tex; }
    AEGfxTexture* autumn1() { return autumn1Tex; }
    AEGfxTexture* autumn2() { return autumn2Tex; }

    AEGfxTexture* winterC() { return winterCTex; }   // tile ID 4 -> WinterC.png
    AEGfxTexture* winterT() { return winterTTex; }   // tile ID 6 -> WinterT.png

    AEGfxTexture* grass() { return grassTex; }
    AEGfxTexture* fireTex() { return fireTex_; }
    AEGfxTexture* sawTex() { return sawTex_; }

    bool getFireUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        if (frame < 0) frame = 0;
        frame %= fireFrameCount;
        constexpr float insetPx = 0.5f;
        float px = frame * fireFrameW;
        u0 = (px + insetPx) / fireSheetW;
        u1 = (px + fireFrameW - insetPx) / fireSheetW;
        v0 = insetPx / fireSheetH;
        v1 = (fireFrameH - insetPx) / fireSheetH;
        return true;
    }

    bool getSawUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        if (frame < 0) frame = 0;
        frame %= sawFrameCount;
        constexpr float insetPx = 0.5f;
        float px = frame * sawFrameW;
        u0 = (px + insetPx) / sawSheetW;
        u1 = (px + sawFrameW - insetPx) / sawSheetW;
        v0 = insetPx / sawSheetH;
        v1 = (sawFrameH - insetPx) / sawSheetH;
        return true;
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

        // fire (tile 24)
        fireTimer += dt;
        while (fireTimer >= fireFrameTime)
        {
            fireTimer -= fireFrameTime;
            fireFrame = (fireFrame + 1) % fireFrameCount;
        }

        // saw (tile 25)
        sawTimer += dt;
        while (sawTimer >= sawFrameTime)
        {
            sawTimer -= sawFrameTime;
            sawFrame = (sawFrame + 1) % sawFrameCount;
        }

        // bottle float (tile 18)
        bottleFloatTimer += dt;
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

            // Draw same height as signboard (1.5x cell height), slightly wider
            gfx::Vec2 cpSize{ size.x * 1.2f, size.y * 1.5f };
            pos.y += (cpSize.y - size.y) * 0.5f;  // shift up so bottom aligns with tile bottom
            gfx::drawSprite(checkpointTex, pos, 0.0f, cpSize, u0, v0, u1, v1);
            return true;
        }

        // Tile 24 = fire (animated, bigger to look denser)
        if (tileType == 24)
        {
            if (!fireTex_) return true;
            float u0{}, v0{}, u1{}, v1{};
            getFireUv(fireFrame, u0, v0, u1, v1);
            // Make fire 1.5x wider and 2x taller, anchored at bottom of cell
            gfx::Vec2 fireSize{ size.x * 1.5f, size.y * 2.0f };
            gfx::Vec2 firePos{ pos.x, pos.y + (fireSize.y - size.y) * 0.5f };
            gfx::drawSprite(fireTex_, firePos, 0.0f, fireSize, u0, v0, u1, v1);
            return true;
        }

        // Tile 25 = saw (animated, fits exactly in cell)
        if (tileType == 25)
        {
            if (!sawTex_) return true;
            float u0{}, v0{}, u1{}, v1{};
            getSawUv(sawFrame, u0, v0, u1, v1);
            gfx::drawSprite(sawTex_, pos, 0.0f, size, u0, v0, u1, v1);
            return true;
        }

        // Tile 18 = bottle (floating animation on Y axis)
        if (tileType == 18)
        {
            if (!bottleTex) return true;
            float yOffset = sinf(bottleFloatTimer * bottleFloatSpeed) * bottleFloatAmp;
            gfx::Vec2 floatPos{ pos.x, pos.y + yOffset };
            gfx::Vec2 bottleSize{ size.x * 0.75f, size.y * 0.75f };
            gfx::drawSprite(bottleTex, floatPos, 0.0f, bottleSize, 0.0f, 0.0f, 1.0f, 1.0f);
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
            // case 4 and 6 are now standalone textures (WinterC.png / WinterT.png)
        case 5: uvFromPixels(144, 288, 32, 32, u0, v0, u1, v1); return true; // ice   (32x32 version of big)

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