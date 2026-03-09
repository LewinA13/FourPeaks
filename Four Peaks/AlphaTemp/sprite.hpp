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

    // tutorial background texture (bg_tutorial.png)
    AEGfxTexture* tutorialBackground();

    // summer background texture summerbg.png
    AEGfxTexture* summerBackground();

    // autumn background texture 
    // .jpg
    AEGfxTexture* autumnBackground();

    // spring background texture springBG.jpg
    AEGfxTexture* springBackground();

    // coin spritesheet (coin_.png)
    AEGfxTexture* coin();

    // ice texture (ice1.jpg)
    AEGfxTexture* ice();

    // checkpoint spritesheet (Checkpoint.png)
    AEGfxTexture* checkpoint();

    // ice_crack texture (b_ice.jpg)
    AEGfxTexture* crack();

    // heatbar spritesheet (heatbar.png)
    AEGfxTexture* heatbar();

    AEGfxTexture* mmBackground();

    // sign texture (sign.jpg)
    AEGfxTexture* sign();
    // tile12 texture (Tile_12.jpg)
    AEGfxTexture* tile12();
    // tile02 texture (Tile_02.jpg)
    AEGfxTexture* tile02();
    // bottle texture (bottle.jpg)
    AEGfxTexture* bottle();

    // textbox texture (textbox.png)
    AEGfxTexture* textbox();

    // textbox arrow texture
    AEGfxTexture* textboxUp();    // (textboxUp.png)
    AEGfxTexture* textboxDown();    // (textboxDown.png)



    // -----------------------------------------------------------------
    // Standalone seasonal tile textures
    // Tile IDs:
    //   1 -> spring1
    //   3 -> spring2
    //   5 -> autumn1
    //   7 -> autumn2
    // -----------------------------------------------------------------
    AEGfxTexture* spring1();
    AEGfxTexture* spring2();
    AEGfxTexture* autumn1();
    AEGfxTexture* autumn2();

    // Tile ID 4 -> WinterC.png, Tile ID 6 -> WinterT.png
    AEGfxTexture* winterC();
    AEGfxTexture* winterT();

    // -----------------------------------------------------------------
    // New tile textures
    // 23 -> grass (grasss.png, single frame)
    // 24 -> fire  (fire.png,   animated spritesheet, 8 frames 20x24)
    // 25 -> saw   (saw.png,    animated spritesheet, 8 frames 38x38)
    // 26 -> spike facing LEFT  (uses spikes texture, rotated)
    // 27 -> spike facing RIGHT (uses spikes texture, rotated)
    // -----------------------------------------------------------------
    AEGfxTexture* grass();
    AEGfxTexture* fireTex();
    AEGfxTexture* sawTex();

    bool getFireUv(int frame, float& u0, float& v0, float& u1, float& v1);
    bool getSawUv(int frame, float& u0, float& v0, float& u1, float& v1);


    // UVs for checkpoint animation frames
    bool getCheckpointUv(int frame, float& u0, float& v0, float& u1, float& v1);

    // UVs for coin animation frames 
    bool getCoinUv(int frame, float& u0, float& v0, float& u1, float& v1);

    // UVs for crack animation frames
    bool getCrackUv(int frame, float& u0, float& v0, float& u1, float& v1);

    // UVs from tileset
    bool getTileUv(int tileType, float& u0, float& v0, float& u1, float& v1);

    // Update shared animated tiles (coin/checkpoint/crack) once per frame
    void updateAnimatedTiles(f32 dt);

    // Draw animated tiles by tileType. Returns true if it drew something.
    // Use this inside any stage tile loop.
    bool drawAnimatedTile(int tileType, gfx::Vec2 pos, gfx::Vec2 size);

    // frame 0 = full, frame 11 = empty
    bool getHeatBarUv(int frame, float& u0, float& v0, float& u1, float& v1);

    //! this two following var move from sprite.cpp to this headers
    constexpr int crackFrameCount = 5;
    constexpr float crackFrameTime = 0.30f;

}