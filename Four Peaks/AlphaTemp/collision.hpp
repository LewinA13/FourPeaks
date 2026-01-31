#pragma once
#include "player.hpp"

void CollisionUpdate(Player &player,f32 dt);
void CollisionUpdateWallFlags(Player& player); // update wall booleans in player
void CollisionResolveSpawn(Player& player);
