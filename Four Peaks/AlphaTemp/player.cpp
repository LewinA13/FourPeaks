#include "player.hpp"

void PlayerInit(Player& p)
{
    p.pos = { 100.0f, 100.0f };
    p.size = { 40.0f, 40.0f };
    p.speed = 200.0f;

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
    // If you're on ground and not moving up, keep velY at 0
    if (p.grounded && p.velY <= 0.0f)
        p.velY = 0.0f;

    // Apply gravity to velocity
    p.velY += p.gravity * dt;

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


void PlayerDraw(Player& p) {
    gfx::drawRectangle(p.pos, 0.0f, p.size, 0xFFFF0000);
}