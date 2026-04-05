// ----------------------------------------------------------------------------
// Done By: Arun, Hong Yang, Skyler
// ----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Copyright (C) 2026 Team Game++ (Four Peaks)
// All rights reserved.
//
// This file is part of the Four Peaks project. All code, design, and original
// assets are the work of LewinA and team members unless otherwise stated.
//
// Audio assets are sourced from Soundly and used under appropriate licensing.
//
// Reproduction, distribution, or modification of this file or its contents,
// in whole or in part, without prior written permission is strictly prohibited.
//
//---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Sprite.cpp
// Manages loading and access to all textures used in the game.
// Also manages shared animation state for animated tiles (coin, checkpoint, crack).
// ---------------------------------------------------------------------------
#include "engine/sprite.hpp"
#include <AEGraphics.h>
#include "engine/graphics.hpp"
#include <cmath>


// ---------------------------------------------------------------------------
// namespace sprite that contains all texture pointers and related functions for loading, accessing, and animating sprites used in the game.
// ---------------------------------------------------------------------------
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
        AEGfxTexture* winterCTex{};   
        AEGfxTexture* winterTTex{};   
        AEGfxTexture* breakabletileTex{};
        AEGfxTexture* stgselectbgTex{};
        AEGfxTexture* centerframeTex{};

        // New tile textures (IDs 23-25)
        AEGfxTexture* grassTex{};
        AEGfxTexture* fireTex_{};
        AEGfxTexture* sawTex_{};

        // artifacts 
        AEGfxTexture* winterArtifactsTex{};
        AEGfxTexture* summerArtifactsTex{};
        AEGfxTexture* springArtifactsTex{};
        AEGfxTexture* autumnArtifactsTex{};

        // ----------------------------------------------------------------
        // Stage preview textures [4 seasons][4 stages]
        // Loaded once on first use via a static array inside each helper.
        // ----------------------------------------------------------------
        AEGfxTexture* winterStageTex[4]{};
        AEGfxTexture* summerStageTex[4]{};
        AEGfxTexture* springStageTex[4]{};
        AEGfxTexture* autumnStageTex[4]{};

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
        // ---------------------------------------------------------------------------
        // Uv From Pixels
        // Explains what this function does and where its main work happens.
        // ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Init
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
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
            summerBgTex = AEGfxTextureLoad("Assets/Summer/summerbg.png");
            if (!summerBgTex) summerBgTex = AEGfxTextureLoad("summerbg.jpg");
        }
        if (!tutorialBgTex) {
            tutorialBgTex = AEGfxTextureLoad("Assets/bg_tutorial.png");
            if (!tutorialBgTex) tutorialBgTex = AEGfxTextureLoad("bg_tutorial.png");
        }
        if (!autumnBgTex) {
            autumnBgTex = AEGfxTextureLoad("Assets/Autumn/autumnbg.jpg");
            if (!autumnBgTex) autumnBgTex = AEGfxTextureLoad("autumnbg.jpg");
        }
        if (!springBgTex) {
            springBgTex = AEGfxTextureLoad("Assets/Spring/springBG2.jpg");
            if (!springBgTex) springBgTex = AEGfxTextureLoad("springBG2.jpg");
        }
        if (!tilesetTex) {
            tilesetTex = AEGfxTextureLoad("Assets/Winter/winter_.png");
            if (!tilesetTex) tilesetTex = AEGfxTextureLoad("winter_.png");
        }
        if (!spikesTex) {
            spikesTex = AEGfxTextureLoad("Assets/idle.png");
            if (!spikesTex) spikesTex = AEGfxTextureLoad("idle.png");
        }
        if (!coinTex) {
            coinTex = AEGfxTextureLoad("Assets/Collectibles/Melon.png");
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
            crackTex = AEGfxTextureLoad("Assets/Winter/b_ice.jpg");
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
            bottleTex = AEGfxTextureLoad("Assets/Collectibles/bottle.png");
            if (!bottleTex) bottleTex = AEGfxTextureLoad("bottle.jpg");
        }
        if (!textboxTex) {
            textboxTex = AEGfxTextureLoad("Assets/textbox.png");
            if (!textboxTex) textboxTex = AEGfxTextureLoad("textbox.jpg");
        }
     
        if (!breakabletileTex) {
            breakabletileTex = AEGfxTextureLoad("Assets/Autumn/breakabletile.png");
            if (!breakabletileTex) breakabletileTex = AEGfxTextureLoad("breakabletile.jpg");
        }

        if (!stgselectbgTex) {
            stgselectbgTex = AEGfxTextureLoad("Assets/stgselectbg.png");
            if (!stgselectbgTex) stgselectbgTex = AEGfxTextureLoad("stgselectbg.jpg");
        }

        if (!centerframeTex) {
            centerframeTex = AEGfxTextureLoad("Assets/Centerframe.png");
            if (!centerframeTex) centerframeTex = AEGfxTextureLoad("Centerframe.jpg");
        }

        // Winter tile overrides (standalone images)
        if (!winterCTex) winterCTex = AEGfxTextureLoad("Assets/Winter/WinterC.png");
        if (!winterTTex) winterTTex = AEGfxTextureLoad("Assets/Winter/WinterT.png");

        // Seasonal tile textures (standalone images)
        if (!spring1Tex) spring1Tex = AEGfxTextureLoad("Assets/Spring/spring1.png");
        if (!spring2Tex) spring2Tex = AEGfxTextureLoad("Assets/Spring/spring2.png");
        if (!autumn1Tex) autumn1Tex = AEGfxTextureLoad("Assets/Autumn/autumn1.png");
        if (!autumn2Tex) autumn2Tex = AEGfxTextureLoad("Assets/Autumn/autumn2.png");

        // New tile textures
        if (!grassTex)  grassTex = AEGfxTextureLoad("Assets/Autumn/grasss.png");
        if (!fireTex_)  fireTex_ = AEGfxTextureLoad("Assets/Summer/fire.png");
        if (!sawTex_)   sawTex_ = AEGfxTextureLoad("Assets/saw.png");

        // artifacts 
        if (!winterArtifactsTex)  winterArtifactsTex = AEGfxTextureLoad("Assets/Winter/winterArtifacts.png");
        if (!summerArtifactsTex)  summerArtifactsTex = AEGfxTextureLoad("Assets/Summer/summerArtifacts.png");
        if (!springArtifactsTex)  springArtifactsTex = AEGfxTextureLoad("Assets/Spring/springArtifacts.png");
        if (!autumnArtifactsTex)  autumnArtifactsTex = AEGfxTextureLoad("Assets/Autumn/autumnArtifacts.png");

        // ----------------------------------------------------------------
        // Stage preview textures — loaded here so they are ready before the
        // stage selection screen is first drawn.
        // Update the file paths below to match your actual asset filenames.
        // ----------------------------------------------------------------
        if (!winterStageTex[0]) winterStageTex[0] = AEGfxTextureLoad("Assets/Winter/winter_stage1.png");
        if (!winterStageTex[1]) winterStageTex[1] = AEGfxTextureLoad("Assets/Winter/winter_stage2.png");
        if (!winterStageTex[2]) winterStageTex[2] = AEGfxTextureLoad("Assets/Winter/winter_stage3.png");
        if (!winterStageTex[3]) winterStageTex[3] = AEGfxTextureLoad("Assets/Winter/winter_stage4.png");

        if (!summerStageTex[0]) summerStageTex[0] = AEGfxTextureLoad("Assets/Summer/summer_stage1.png");
        if (!summerStageTex[1]) summerStageTex[1] = AEGfxTextureLoad("Assets/Summer/summer_stage2.png");
        if (!summerStageTex[2]) summerStageTex[2] = AEGfxTextureLoad("Assets/Summer/summer_stage3.png");
        if (!summerStageTex[3]) summerStageTex[3] = AEGfxTextureLoad("Assets/Summer/summer_stage4.png");

        if (!springStageTex[0]) springStageTex[0] = AEGfxTextureLoad("Assets/Spring/spring_stage1.png");
        if (!springStageTex[1]) springStageTex[1] = AEGfxTextureLoad("Assets/Spring/spring_stage2.png");
        if (!springStageTex[2]) springStageTex[2] = AEGfxTextureLoad("Assets/Spring/spring_stage3.png");
        if (!springStageTex[3]) springStageTex[3] = AEGfxTextureLoad("Assets/Spring/spring_stage4.png");

        if (!autumnStageTex[0]) autumnStageTex[0] = AEGfxTextureLoad("Assets/Autumn/autumn_stage1.png");
        if (!autumnStageTex[1]) autumnStageTex[1] = AEGfxTextureLoad("Assets/Autumn/autumn_stage2.png");
        if (!autumnStageTex[2]) autumnStageTex[2] = AEGfxTextureLoad("Assets/Autumn/autumn_stage3.png");
        if (!autumnStageTex[3]) autumnStageTex[3] = AEGfxTextureLoad("Assets/Autumn/autumn_stage4.png");
    }


// ---------------------------------------------------------------------------
// Shutdown
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
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
        }
        if (tile12Tex) { AEGfxTextureUnload(tile12Tex); tile12Tex = nullptr; }
        if (tile02Tex) { AEGfxTextureUnload(tile02Tex); tile02Tex = nullptr; }
        if (bottleTex) { AEGfxTextureUnload(bottleTex); bottleTex = nullptr; }
        if (textboxTex) { AEGfxTextureUnload(textboxTex); textboxTex = nullptr; }

        if (winterCTex) { AEGfxTextureUnload(winterCTex); winterCTex = nullptr; }
        if (winterTTex) { AEGfxTextureUnload(winterTTex); winterTTex = nullptr; }

        if (spring1Tex) { AEGfxTextureUnload(spring1Tex); spring1Tex = nullptr; }
        if (spring2Tex) { AEGfxTextureUnload(spring2Tex); spring2Tex = nullptr; }
        if (autumn1Tex) { AEGfxTextureUnload(autumn1Tex); autumn1Tex = nullptr; }
        if (autumn2Tex) { AEGfxTextureUnload(autumn2Tex); autumn2Tex = nullptr; }

        if (breakabletileTex) { AEGfxTextureUnload(breakabletileTex); breakabletileTex = nullptr; }
        if (stgselectbgTex) { AEGfxTextureUnload(stgselectbgTex); stgselectbgTex = nullptr; }
        if (centerframeTex) { AEGfxTextureUnload(centerframeTex); centerframeTex = nullptr; }

        if (grassTex) { AEGfxTextureUnload(grassTex);  grassTex = nullptr; }
        if (fireTex_) { AEGfxTextureUnload(fireTex_);  fireTex_ = nullptr; }
        if (sawTex_) { AEGfxTextureUnload(sawTex_);   sawTex_ = nullptr; }

        if (winterArtifactsTex) { AEGfxTextureUnload(winterArtifactsTex); winterArtifactsTex = nullptr; }
        if (summerArtifactsTex) { AEGfxTextureUnload(summerArtifactsTex); summerArtifactsTex = nullptr; }
        if (springArtifactsTex) { AEGfxTextureUnload(springArtifactsTex); springArtifactsTex = nullptr; }
        if (autumnArtifactsTex) { AEGfxTextureUnload(autumnArtifactsTex); autumnArtifactsTex = nullptr; }

        // Unload stage preview textures
        for (int i = 0; i < 4; ++i)
        {
            if (winterStageTex[i]) { AEGfxTextureUnload(winterStageTex[i]); winterStageTex[i] = nullptr; }
            if (summerStageTex[i]) { AEGfxTextureUnload(summerStageTex[i]); summerStageTex[i] = nullptr; }
            if (springStageTex[i]) { AEGfxTextureUnload(springStageTex[i]); springStageTex[i] = nullptr; }
            if (autumnStageTex[i]) { AEGfxTextureUnload(autumnStageTex[i]); autumnStageTex[i] = nullptr; }
        }
    }

// ---------------------------------------------------------------------------
// Tileset
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* tileset()
    {
        return tilesetTex;
    }

// ---------------------------------------------------------------------------
// Spikes
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* spikes()
    {
        return spikesTex;
    }

// ---------------------------------------------------------------------------
// Background
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* background()
    {
        return backgroundTex;
    }

// ---------------------------------------------------------------------------
// Summer Background
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* summerBackground()
    {
        return summerBgTex;
    }

// ---------------------------------------------------------------------------
// Mm Background
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* mmBackground()
    {
        return mmBackgroundTex;
    }

// ---------------------------------------------------------------------------
// Tutorial Background
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* tutorialBackground()
    {
        return tutorialBgTex;
    }

// ---------------------------------------------------------------------------
// Autumn Background
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* autumnBackground()
    {
        return autumnBgTex;
    }

// ---------------------------------------------------------------------------
// Spring Background
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* springBackground()
    {
        return springBgTex;
    }

// ---------------------------------------------------------------------------
// Coin
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* coin()
    {
        return coinTex;
    }

// ---------------------------------------------------------------------------
// Ice
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* ice()
    {
        return iceTex;
    }

// ---------------------------------------------------------------------------
// Checkpoint
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* checkpoint()
    {
        return checkpointTex;
    }

// ---------------------------------------------------------------------------
// Crack
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* crack()
    {
        return crackTex;
    }

// ---------------------------------------------------------------------------
// Sign
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* sign() { return signTex; }
// ---------------------------------------------------------------------------
// Tile12
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* tile12() { return tile12Tex; }
// ---------------------------------------------------------------------------
// Tile02
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* tile02() { return tile02Tex; }
// ---------------------------------------------------------------------------
// Bottle
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* bottle() { return bottleTex; }
// ---------------------------------------------------------------------------
// Textbox
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* textbox() { return textboxTex; }

// ---------------------------------------------------------------------------
// Breakabletile
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* breakabletile() { return breakabletileTex; }
// ---------------------------------------------------------------------------
// Stgselect Background
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* stgselectBackground() { return stgselectbgTex; }
// ---------------------------------------------------------------------------
// Centerframe
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* centerframe() { return centerframeTex; }


// ---------------------------------------------------------------------------
// Spring1
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* spring1() { return spring1Tex; }
// ---------------------------------------------------------------------------
// Spring2
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* spring2() { return spring2Tex; }
// ---------------------------------------------------------------------------
// Autumn1
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* autumn1() { return autumn1Tex; }
// ---------------------------------------------------------------------------
// Autumn2
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* autumn2() { return autumn2Tex; }

// ---------------------------------------------------------------------------
// Winter C
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* winterC() { return winterCTex; }
// ---------------------------------------------------------------------------
// Winter T
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* winterT() { return winterTTex; }

// ---------------------------------------------------------------------------
// Grass
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* grass() { return grassTex; }
// ---------------------------------------------------------------------------
// Fire Tex
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* fireTex() { return fireTex_; }
// ---------------------------------------------------------------------------
// Saw Tex
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* sawTex() { return sawTex_; }

// ---------------------------------------------------------------------------
// Winter Artifacts
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* winterArtifacts() { return winterArtifactsTex; }
// ---------------------------------------------------------------------------
// Summer Artifacts
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* summerArtifacts() { return summerArtifactsTex; }
// ---------------------------------------------------------------------------
// Spring Artifacts
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* springArtifacts() { return springArtifactsTex; }
// ---------------------------------------------------------------------------
// Autumn Artifacts
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* autumnArtifacts() { return autumnArtifactsTex; }

    // ----------------------------------------------------------------
    // Public stage preview accessors (declared in sprite.hpp)
    // Returns nullptr if index is out of range or texture failed to load.
    // ----------------------------------------------------------------
    AEGfxTexture* winterStage(int stageIndex)
    {
        if (stageIndex < 0 || stageIndex > 3) return nullptr;
        return winterStageTex[stageIndex];
    }

// ---------------------------------------------------------------------------
// Summer Stage
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* summerStage(int stageIndex)
    {
        if (stageIndex < 0 || stageIndex > 3) return nullptr;
        return summerStageTex[stageIndex];
    }

// ---------------------------------------------------------------------------
// Spring Stage
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* springStage(int stageIndex)
    {
        if (stageIndex < 0 || stageIndex > 3) return nullptr;
        return springStageTex[stageIndex];
    }

// ---------------------------------------------------------------------------
// Autumn Stage
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* autumnStage(int stageIndex)
    {
        if (stageIndex < 0 || stageIndex > 3) return nullptr;
        return autumnStageTex[stageIndex];
    }

// ---------------------------------------------------------------------------
// Get Fire Uv
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Get Saw Uv
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
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



// ---------------------------------------------------------------------------
// Get Coin Uv
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
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


// ---------------------------------------------------------------------------
// Get Checkpoint Uv
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Get Crack Uv
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    bool getCrackUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        constexpr int frameCount = 7;
        constexpr float sheetW = 160.0f;
        constexpr float sheetH = 32.0f;
        constexpr float frameW = 32.0f;
        constexpr float frameH = 32.0f;

        if (frame < 0) frame = 0;
        frame %= frameCount;

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


// ---------------------------------------------------------------------------
// Update Animated Tiles
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Draw Animated Tile
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    bool drawAnimatedTile(int tileType, gfx::Vec2 pos, gfx::Vec2 size)
    {
        if (tileType == 8)
        {
            if (!coinTex) return true;

            float u0{}, v0{}, u1{}, v1{};
            getCoinUv(coinFrame, u0, v0, u1, v1);

            gfx::Vec2 coinSize{ size.x * 0.9f, size.y * 0.9f };
            gfx::drawSprite(coinTex, pos, 0.0f, coinSize, u0, v0, u1, v1);
            return true;
        }

        if (tileType == 10)
        {
            if (!checkpointTex) return true;

            float u0{}, v0{}, u1{}, v1{};
            getCheckpointUv(checkpointFrame, u0, v0, u1, v1);

            gfx::Vec2 cpSize{ size.x * 1.2f, size.y * 1.5f };
            pos.y += (cpSize.y - size.y) * 0.5f;
            gfx::drawSprite(checkpointTex, pos, 0.0f, cpSize, u0, v0, u1, v1);
            return true;
        }

        if (tileType == 24)
        {
            if (!fireTex_) return true;
            float u0{}, v0{}, u1{}, v1{};
            getFireUv(fireFrame, u0, v0, u1, v1);
            gfx::Vec2 fireSize{ size.x * 1.5f, size.y * 2.0f };
            gfx::Vec2 firePos{ pos.x, pos.y + (fireSize.y - size.y) * 0.5f };
            gfx::drawSprite(fireTex_, firePos, 0.0f, fireSize, u0, v0, u1, v1);
            return true;
        }

        if (tileType == 25)
        {
            if (!sawTex_) return true;
            float u0{}, v0{}, u1{}, v1{};
            getSawUv(sawFrame, u0, v0, u1, v1);
            gfx::drawSprite(sawTex_, pos, 0.0f, size, u0, v0, u1, v1);
            return true;
        }

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


// ---------------------------------------------------------------------------
// Get Tile Uv
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    bool getTileUv(int tileType, float& u0, float& v0, float& u1, float& v1)
    {
        switch (tileType)
        {
        case 1: uvFromPixels(0, 288, 32, 32, u0, v0, u1, v1); return true;
        case 3: uvFromPixels(144, 48, 32, 32, u0, v0, u1, v1); return true;
        case 5: uvFromPixels(144, 288, 32, 32, u0, v0, u1, v1); return true;
        case 7: uvFromPixels(48, 128, 32, 32, u0, v0, u1, v1); return true;
        }

        return false;
    }

// ---------------------------------------------------------------------------
// Heatbar
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    AEGfxTexture* heatbar() { return heatbarTex; }

// ---------------------------------------------------------------------------
// Get Heat Bar Uv
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    bool getHeatBarUv(int frame, float& u0, float& v0, float& u1, float& v1)
    {
        if (frame < 0 || frame > 5) return false;
        constexpr float frameW = 48.0f / 384.0f;
        u0 = frame * frameW;
        u1 = u0 + frameW;
        v0 = 0.5f;
        v1 = 1.0f;
        return true;
    }
}
