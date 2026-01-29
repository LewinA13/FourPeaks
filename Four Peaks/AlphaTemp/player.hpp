#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "graphics.hpp"
#include "AEEngine.h"

// Player data
struct Player
{
    gfx::Vec2 pos;
    gfx::Vec2 size;
    f32 speed;

    f32 velY;        // vertical velocity (units/sec)
    f32 velX;
    bool grounded;   // are we on the floor?

    f32 gravity;     // units/sec^2  (negative if +Y is up)
    f32 terminalVel; // max falling speed (negative)

    f32 horzSpeed;
    f32 jumpVel; // jump velocity

    f32 coyoteTime;   // how long coyote lasts
    f32 coyoteTimer;  // counts down after leaving ground

    f32 jumpCutMult; // extra gravity when jump not held

    bool alive;

    // ======== IDLE ANIMATION ==========
    AEGfxTexture* idleTex;

    int idleFrame;
    int idleFrameCount;

    f32 idleAnimTimer;
    f32 idleFrameTime; // seconds per frame (e.g. 0.10f)

    // ======== RUN ANIMATION ==========
    AEGfxTexture* runTex;

    int runFrame;
    int runFrameCount;

    f32 runAnimTimer;
    f32 runFrameTime;

    // Facing direction: +1 = right, -1 = left
    int facing;

    // ======== JUMP ANIMATION ==========
    AEGfxTexture* jumpTex;

    int jumpFrame;
    int jumpFrameCount;

    f32 jumpAnimTimer;
    f32 jumpFrameTime;

    // wall jump
    f32 wallRegrabTimer;     // seconds before re-hang
    f32 wallRegrabTime;

    f32 wallHangBufferTime;
    f32 wallHangBufferTimer;

    f32 wallClimbSpeed;     // upward speed (positive)
    f32 wallClimbDownSpeed; // downward speed magnitude (positive)


    // ======== WALL SLIDE ANIMATION ==========
    AEGfxTexture* wallSlideTex;

    int wallSlideFrame;
    int wallSlideFrameCount;

    f32 wallSlideAnimTimer;
    f32 wallSlideFrameTime;



    // ======== FALL ANIMATION ==========
    AEGfxTexture* fallTex;

    int fallFrame;
    int fallFrameCount;

    f32 fallAnimTimer;
    f32 fallFrameTime;

    // ======== COLLIDER BOX ==========
    gfx::Vec2 colliderSize;  // physics box size (used for grounded/collision)
    gfx::Vec2 spriteSize;    // visual size (used for drawing only)

    // optional: small tweak if your art has padding
    float spriteOffsetY;     // visual feet adjustment (usually a small number)
    float spriteOffsetX;        // default (usually 0)
    float wallHangOffsetX;

    // ======== WALL DETECTION && MOVEMENT ==========
    bool onWallLeft;
    bool onWallRight;

    bool wallHanging;
    float wallSlideSpeed;   
    float wallHangGravityScale; // multiplier of downward speed
    bool wallHangRequested;   // input for wall hang

    // ======== DASH ==========
    bool  dashing;
    f32 dashTimer;
    f32 dashDuration;

    f32 dashSpeed;
    int   dashDir;        // -1 = left, +1 = right

    int   dashCount;
    int   maxDashCount;

    // ======== DASH ANIMATION ==========
    AEGfxTexture* dashTex;

    int dashFrame;
    int dashFrameCount;

    f32 dashAnimTimer;
    f32 dashFrameTime;
};

// function declarations (NO function bodies here)
void PlayerInit(Player& p);
void PlayerUpdate(Player& p, float dt);
void PlayerDraw(Player& p);
void PlayerShutdown(Player& p);

#endif


