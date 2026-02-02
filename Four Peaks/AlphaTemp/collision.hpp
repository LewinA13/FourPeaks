#pragma once
#include "player.hpp"

extern int (*g_currentMap)[32];

void CollisionUpdate(Player &player,f32 dt);
void CollisionUpdateWallFlags(Player& player); // update wall booleans in player
void CollisionResolveSpawn(Player& player);

void CheckPathForCheckpoint(Player& player, gfx::Vec2 startPos, gfx::Vec2 endPos);