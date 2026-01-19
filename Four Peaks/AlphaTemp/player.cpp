#include "player.hpp"


void PlayerInit(Player& p)
{
    p.pos = { 100.0f, 100.0f };
    p.size = { 400.0f, 400.0f };
    p.speed = 400.0f;

    p.velY = 0.0f;
    p.grounded = false;

    //hy test
    p.horzSpeed = 0.0f;


    p.gravity = -2000.0f;     
    p.terminalVel = -1200.0f; 
}

void PlayerUpdate(Player& p, float dt) {
    (void)dt;



    // ===================== HORIZONTAL INPUT (A/D) =====================
    float moveX = 0.0f;

    if (AEInputCheckCurr(AEVK_A)) moveX -= 1.0f;
    if (AEInputCheckCurr(AEVK_D)) moveX += 1.0f;

    // ===================== Horizontal Movement (Acce/Decel) (Ground/Air) =====================
    // Apply different acceleration/deceleration on ground/air
    float accel = p.grounded ? 8.0f : 4.0f;
    float decel = p.grounded ? 4.0f : 2.0f;

    // Set a speed limit to clamp horinzontal speed
    float maxHorzSpeed = 2.0f;
    float minHorzSpeed = -2.0f;


    // if user pressing A/D, accelerate
    if (moveX != 0.0f) {
        p.horzSpeed += moveX * accel * dt;
    }
    else {
        // if user not pressing, decelerate
        if (p.horzSpeed > 0) {
            p.horzSpeed -= decel * dt;
            /*
                avoid p.horzSpeed stuck at some value greater than 0 but after calculating less than 0, and never equal 0
                E.X: p.horzSpeed = 0.1, 
                       0.1 -= 3.0f*0.0167, get -0.067
            */
            if (p.horzSpeed < 0) p.horzSpeed = 0;  
        }
        else if (p.horzSpeed < 0) {
            p.horzSpeed += decel * dt;
            /*
                avoid p.horzSpeed stuck at some value less than 0 but after calculating greater than 0, and never equal 0
                E.X: p.horzSpeed = -0.1,
                        -0.1 += 3.0f*0.0167, get 0.0499
            */
            if (p.horzSpeed > 0) p.horzSpeed = 0;  
        }
    }

    // Clamp horzSpeed between in maxHorzSpeed and minHorzSpeed
    if (p.horzSpeed > maxHorzSpeed) {
        p.horzSpeed = maxHorzSpeed;
    }
    else if (p.horzSpeed < minHorzSpeed){
        p.horzSpeed = minHorzSpeed;
    }

    p.pos.x += p.horzSpeed * p.speed * dt;




    // ===================== GRAVITY (VERTICAL) =====================
    p.gravity = -2000.0f;     
    p.terminalVel = -1200.0f;

    p.jumpVel = 950.0f;

    p.coyoteTime = 0.08f;  // tweak: 0.06 - 0.12 feels normal
    p.coyoteTimer = 0.0f;

    p.jumpCutMult = 2.5f; // tweak: 2.0 - 4.0

    // sprite initialisation
    p.idleTex = AEGfxTextureLoad("Assets/player/male_hero-idle.png");

    p.idleFrame = 0;
    p.idleFrameCount = 10;

    p.idleAnimTimer = 0.0f;
    p.idleFrameTime = 0.10f; // 10 FPS idle animation

    // running sprite initialisation
    p.runTex = AEGfxTextureLoad("Assets/player/male_hero-run.png");

    p.runFrame = 0;
    p.runFrameCount = 10;     // run sheet has 10 frames
    p.runAnimTimer = 0.0f;
    p.runFrameTime = 0.08f;   // faster than idle

    p.facing = 1;

    // jumping sprite initialisation
    p.jumpTex = AEGfxTextureLoad("Assets/player/male_hero-jump.png");

    p.jumpFrame = 0;
    p.jumpFrameCount = 6;      // 6 frames
    p.jumpAnimTimer = 0.0f;
    p.jumpFrameTime = 0.06f;   // can change

    // falling sprite initialisation
    p.fallTex = AEGfxTextureLoad("Assets/player/male_hero-fall_loop.png");

    p.fallFrame = 0;
    p.fallFrameCount = 3;      // 384x128 => 3 frames
    p.fallAnimTimer = 0.0f;
    p.fallFrameTime = 0.08f;   // can change

}

void PlayerUpdate(Player& p, float dt) {

    if (dt > 0.05f) dt = 0.05f;

    // ===================== HORIZONTAL INPUT (A/D) ===================================
    f32 moveX = 0.0f;
    if (AEInputCheckCurr(AEVK_A)) {
        moveX -= 1.0f;
        p.facing = -1;
    }
    if (AEInputCheckCurr(AEVK_D)) {
        moveX += 1.0f;
        p.facing = 1;
    }

    p.pos.x += moveX * p.speed * dt;

    // ===================== GRAVITY (VERTICAL) =====================

    // Coyote timer ------------ COYOTE JUMP ALLOWS PLAYER TO JUMP 0.08 SECOND AFTER FALLING OFF PLATFORM
    if (p.grounded)
    {
        p.coyoteTimer = p.coyoteTime;   // refresh while on ground
    }
    else
    {
        p.coyoteTimer -= dt;
        if (p.coyoteTimer < 0.0f) p.coyoteTimer = 0.0f;
    }

    bool canCoyoteJump = (p.coyoteTimer > 0.0f);

    if (AEInputCheckTriggered(AEVK_SPACE) && (p.grounded || canCoyoteJump))
    {
        p.velY = p.jumpVel;
        p.grounded = false;
        p.coyoteTimer = 0.0f;
        p.jumpFrame = 0;
        p.jumpAnimTimer = 0.0f;
    }

    // If you're on ground and not moving up, keep velY at 0
    if (p.grounded && p.velY <= 0.0f)
        p.velY = 0.0f;

    // Apply gravity to velocity
    p.velY += p.gravity * dt;


    // ===================== IDLE ANIMATION  =====================
    // only when not moving
    bool isMoving = (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_D));

    if (p.grounded && !isMoving)
    {
        p.idleAnimTimer += dt; // accumulate idle timer
        while (p.idleAnimTimer >= p.idleFrameTime) // if idle timer is more than set frame time (0.10f)
        {
            p.idleAnimTimer -= p.idleFrameTime; // minus frame timer
            p.idleFrame = (p.idleFrame + 1) % p.idleFrameCount; // frame loop, adds 1 to advance loop
        }                                                       // eg: if we on frame 8: 8 + 1 = 9, 9 % 10 = 9. if on frame 9, 9 + 1 = 10 % 10 = 0.
    }
    else
    {
        // reset idle frame and idle timer
        p.idleFrame = 0;
        p.idleAnimTimer = 0.0f;
    }

    // ===================== RUN ANIMATION  =====================
    isMoving = (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_D));

    if (p.grounded && isMoving)
    {
        p.runAnimTimer += dt;
        while (p.runAnimTimer >= p.runFrameTime)
        {
            p.runAnimTimer -= p.runFrameTime;
            p.runFrame = (p.runFrame + 1) % p.runFrameCount;
        }
    }
    else
    {
        p.runFrame = 0;
        p.runAnimTimer = 0.0f;
    }

    // ===================== AIR ANIMATION (JUMP / FALL) =====================
    if (!p.grounded)
    {
        if (p.velY > 0.0f)
        {
            // JUMP: advance until last frame, then hold
            p.jumpAnimTimer += dt;
            while (p.jumpAnimTimer >= p.jumpFrameTime)
            {
                p.jumpAnimTimer -= p.jumpFrameTime;

                if (p.jumpFrame < p.jumpFrameCount - 1)
                    p.jumpFrame += 1; // no loop
            }
        }
        else
        {
            // FALL: loop frames
            p.fallAnimTimer += dt;
            while (p.fallAnimTimer >= p.fallFrameTime)
            {
                p.fallAnimTimer -= p.fallFrameTime;
                p.fallFrame = (p.fallFrame + 1) % p.fallFrameCount;
            }
        }
    }
    else
    {
        // reset fall when on ground
        p.fallFrame = 0;
        p.fallAnimTimer = 0.0f;
    }

    // ===================== VARIABLE JUMP HEIGHT =====================
    // check if going up and if space is held
    if (p.velY > 0.0f && !AEInputCheckCurr(AEVK_SPACE))
    {
        // We've already applied 1x gravity. Apply extra gravity to make it cut.
        p.velY += p.gravity * (p.jumpCutMult - 1.0f) * dt;
    }

    // Clamp to terminal fall speed
    if (p.velY < p.terminalVel)
        p.velY = p.terminalVel;

    // Integrate velocity into position
    p.pos.y += p.velY * dt;


    // ===================== GROUNDED CHECK (FLOOR) (PLATFORM IN FUTURE) =====================
    static const float GROUND_Y = -450.0f;

    float halfH = p.size.y * 0.5f;
    float feetY = p.pos.y - halfH;

    // If feet went below the ground, snap back up
    if (feetY <= GROUND_Y)
    {
        p.pos.y = GROUND_Y + halfH; // put feet exactly on ground
        p.velY = 0.0f;
        p.grounded = true;
    }
    else
    {
        p.grounded = false;
    }

}


void PlayerDraw(Player& p)
{
    bool isMoving = (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_D));

    AEGfxTexture* tex = nullptr;
    int frame = 0;
    int frameCount = 1;

    // Priority: air states first
    if (!p.grounded)
    {
        if (p.velY > 0.0f)
        {
            tex = p.jumpTex;
            frame = p.jumpFrame;
            frameCount = p.jumpFrameCount;
        }
        else
        {
            tex = p.fallTex;
            frame = p.fallFrame;
            frameCount = p.fallFrameCount;
        }
    }
    else
    {
        // Ground states
        if (isMoving)
        {
            tex = p.runTex;
            frame = p.runFrame;
            frameCount = p.runFrameCount;
        }
        else
        {
            tex = p.idleTex;
            frame = p.idleFrame;
            frameCount = p.idleFrameCount;
        }
    }

    // Compute UV slice for the chosen frame
    f32 frameW = 1.0f / (f32)frameCount;
    f32 u0 = frame * frameW;
    f32 u1 = u0 + frameW;

    // Flip left/right by swapping u0/u1
    if (p.facing < 0)
    {
        f32 tmp = u0;
        u0 = u1;
        u1 = tmp;
    }

    gfx::drawSprite(tex, p.pos, 0.0f, p.size, u0, 0.0f, u1, 1.0f);
}

void PlayerShutdown(Player& p)
{
    if (p.idleTex)
    {
        AEGfxTextureUnload(p.idleTex);
        p.idleTex = nullptr;
    }
    if (p.runTex)
    {
        AEGfxTextureUnload(p.runTex);
        p.runTex = nullptr;
    }
    if (p.jumpTex)
    {
        AEGfxTextureUnload(p.jumpTex);
        p.jumpTex = nullptr;
    }
    if (p.fallTex)
    {
        AEGfxTextureUnload(p.fallTex);
        p.fallTex = nullptr;
    }
}