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
    bool grounded;   // are we on the floor?

    f32 gravity;     // units/sec^2  (negative if +Y is up)
    f32 terminalVel; // max falling speed (negative)
};

// function declarations (NO function bodies here)
void PlayerInit(Player& p);
void PlayerUpdate(Player& p, float dt);
void PlayerDraw(Player& p);

#endif


