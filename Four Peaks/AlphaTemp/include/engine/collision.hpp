// ----------------------------------------------------------------------------
// Done By: Hong Yang
// ----------------------------------------------------------------------------

#pragma once
#include "gameplay/player.hpp"
#include <vector>
#include <string>

// -------------------------------------------------------------------------
// Pointer to the active stage's tile map (20 rows x 32 cols).
// Set by each scene before calling any collision functions.
// -------------------------------------------------------------------------
extern int (*g_currentMap)[32];

// -------------------------------------------------------------------------
// Tile ID of the sign the player is currently near (used by the dialogue system).
// -------------------------------------------------------------------------
extern int g_currentSignID;

// -------------------------------------------------------------------------
// World-space Y offset of the current stage layer, used to map player positions
// onto the correct row in the tile grid.
// -------------------------------------------------------------------------
extern float g_currentY;

// -------------------------------------------------------------------------
// Name of the currently active scene/stage, used for melon tracking and save data.
// -------------------------------------------------------------------------
extern std::string g_currentScene;

// -------------------------------------------------------------------------
// Runs the full per-frame collision pipeline for the player:
// resets ground type, resolves tile overlaps, detects ground surface type,
// and applies the appropriate physics modifiers (accel/decel/speed).
// -------------------------------------------------------------------------
void CollisionUpdate(Player &player,f32 dt);

// -------------------------------------------------------------------------
// Probes one tile-width to the left and right of the player and updates
// the player's onWallLeft / onWallRight booleans accordingly.
// Call this separately from CollisionUpdate when wall-slide / wall-jump
// detection is required.
// -------------------------------------------------------------------------
void CollisionUpdateWallFlags(Player& player);

// -------------------------------------------------------------------------
// Iteratively pushes the player upward until they are clear of any solid
// geometry, then snaps them back down onto the surface. Used after a respawn
// to ensure the player does not start inside a tile.
// -------------------------------------------------------------------------
void CollisionResolveSpawn(Player& player);

// -------------------------------------------------------------------------
// Samples the player's bounding box at evenly-spaced points along the path
// from startPos to endPos (used during dashes) to detect checkpoints and
// ground-type tiles that a single-frame AABB test might miss at high speed.
// -------------------------------------------------------------------------
void CheckPathForCheckpoint(Player& player, gfx::Vec2 startPos, gfx::Vec2 endPos);

// -------------------------------------------------------------------------
// Returns true only when both the left and right foot probes below the player
// detect a solid tile simultaneously, confirming the player is fully supported
// rather than teetering on a ledge edge.
// -------------------------------------------------------------------------
bool OnGroundExactly(Player& player);

// -------------------------------------------------------------------------
// Records a single tile (by row and column) that has been triggered for
// the crack/break animation. Shared between the collision and stage systems.
// -------------------------------------------------------------------------
struct breakableTileTrigger{
    int row, col;
};

// -------------------------------------------------------------------------
// Breakable ice tiles that were stepped on this frame (winter stages).
// Consumed by the stage's update to drive the crack animation.
// -------------------------------------------------------------------------
extern std::vector<breakableTileTrigger> g_triggeredIceTiles;

// -------------------------------------------------------------------------
// Breakable regular tiles that were stepped on this frame (autumn/other stages).
// Consumed by the stage's update to drive the crack animation.
// -------------------------------------------------------------------------
extern std::vector<breakableTileTrigger> g_triggeredbrkTiles;
