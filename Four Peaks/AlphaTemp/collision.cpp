#include "summer_s1.hpp"

#include "player.hpp"

#include <iostream>


const int mapRows = 20;
const int mapColm = 32;

struct TileRange {
	int colStart, colEnd;
	int rowStart, rowEnd;
};

int tileW = 50;
int tileH = 45;

int (*g_currentMap)[32] = nullptr;

TileRange calTileRange(f32 x, f32 y, f32 width, f32 height) {

	TileRange box{};

	//! player collision box: left, right, top, bottom part coordinates
	float left = x - (width / 2.0f) + 0.1f;
	float right = x + (width / 2.0f) - 0.1f;
	float top = y + (height / 2.0f) - 0.1f;
	float bottom = y - (height / 2.0f) + 0.1f;

	/*!
	find out halfWinW and halfWinH and plus on(left, right, bottom and top),
	cause "ALPHA_ENGINE" center is (0,0),
	we dont want get negative result on colStart/colEnd and rowStart/rowEnd
	*/
	float halfWinW = (float)AEGfxGetWindowWidth() / 2.0f;
	float halfWinH = (float)AEGfxGetWindowHeight() / 2.0f;


	//! colStart/colEnd and rowStart/colEnd are which tile player are colliding
	box.colStart = static_cast<int>((left + halfWinW) / tileW);
	box.colEnd = static_cast<int>((right + halfWinW) / tileW);
	box.rowStart = static_cast<int>((bottom + halfWinH) / tileH);
	box.rowEnd = static_cast<int>((top + halfWinH) / tileH);



	//! clamp "colstart" and "colEnd" value in (0, mapColm]
	if (box.colStart < 0) box.colStart = 0;
	if (box.colEnd >= mapColm) box.colEnd = mapColm - 1;
	//! clamp "rowStart" and "rowEnd" value in (0, mapRows]
	if (box.rowStart < 0) box.rowStart = 0;
	if (box.rowEnd >= mapRows) box.rowEnd = mapRows - 1;

	//! check if player out of screen then take as no collision and set all to -1
	if (box.colStart >= mapColm || box.rowStart >= mapRows || box.colEnd < 0 || box.rowEnd < 0) {
		box.colStart = box.colEnd = box.rowStart = box.rowEnd = -1;
	}

	return box;

}


bool checkMapCollision(TileRange box, int levelLayout[][mapColm]) {

	//! return false once member in box equal to -1, means player out of screen
	if (box.colStart == -1) {
		return false;
	}

	//! start checking, if not equal 0, means collision with solid block, return true
	for (int r = box.rowStart; r <= box.rowEnd; r++) {
		for (int c = box.colStart; c <= box.colEnd; c++) {
			if (levelLayout[r][c] != 0) {
				return true;
			}
		}
	}

	//! return false if not collision with solid block
	return false;

}



// Check and change the type grounded type player step on
void checkGroundType(Player& player, TileRange box, int levelLayout[][mapColm]) {

	for (int r = box.rowStart; r <= box.rowEnd; r++) {
		for (int c = box.colStart; c <= box.colEnd; c++) {
			switch (levelLayout[r][c]) {
			case 1:
				player.currGroundType = Player::GroundType::Normal;
				break;

			case 2:
				player.currGroundType = Player::GroundType::Spikes;
				break;

			case 3:
				player.currGroundType = Player::GroundType::Ice;
				break;

			default:
				break;
			}
		}
	}

}

//! This function will get player tile and check collision with maps
bool checkPlayerCollision(Player& player, int levelLayout[][mapColm]) {
	TileRange box = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);
	checkGroundType(player, box, levelLayout);
	return checkMapCollision(box, levelLayout);
}



static bool checkAABBCollisionAt(float x, float y, float w, float h)
{
	TileRange box = calTileRange(x, y, w, h);
	return checkMapCollision(box, g_currentMap);
}

void CollisionResolveSpawn(Player& player) // justin function
{
    const float step = 0.5f;

    // 1) Move UP until not colliding (no horizontal drift)
    for (int i = 0; i < 200 && checkPlayerCollision(player, g_currentMap); ++i)
        player.pos.y += step;

    // 2) Snap DOWN a small distance only (prevents falling into holes)
    const float maxSnapDown = 10.0f;
    const int maxSteps = (int)(maxSnapDown / step);

    bool landed = false;
    for (int i = 0; i < maxSteps; ++i)
    {
        player.pos.y -= step;
        if (checkPlayerCollision(player, g_currentMap))
        {
            player.pos.y += step;
            landed = true;
            break;
        }
    }

    player.grounded = landed;
    player.velX = 0.0f;
    player.velY = 0.0f;
    player.horzSpeed = 0.0f;
}

void resolvePlayerCollision(Player &player, int levelLayout[][mapColm], f32 dt) {

	static const float GROUND_Y = -450.0f;

	float currentY = player.pos.y;
	float oldY = currentY - player.velY * dt;

	//! Set player y coordinates to coordinates before this frame, to seperately handling horizontal and vertical collision
	player.pos.y = oldY;


    // Horizontal collision resolution:
    // Only resolve X when the player actually moved in X this frame.
    if (player.velX != 0.0f && checkPlayerCollision(player, levelLayout))
    {
        const float step = 0.5f;
        float push = (player.velX > 0.0f) ? -step : +step;

        int pushCount = 0;
        while (checkPlayerCollision(player, levelLayout) && pushCount < 200)
        {
            player.pos.x += push;
            pushCount++;
        }

        player.velX = 0.0f;
    }

	//! Set back player y coordinates to this frame
	player.pos.y = currentY;

	float feetY = player.pos.y - player.colliderSize.y / 2.0f;

	if (feetY > GROUND_Y + 10.0f) { //! Check player bottom whether greater than "GROUND_Y", if lower, "PlayerUpdate" will handle
		if (checkPlayerCollision(player, levelLayout)) {

			//! Check player moving up or down and determine "push" value
			f32 push = (player.velY > 0) ? -0.5f : 0.5f;

			//! Set "pushCount" to 0 in case happens infinity loops
			int pushCount = 0;

			//! use while loops to force push player until not detect collision with solid block
			while (checkPlayerCollision(player, levelLayout) && pushCount < 100) {
				player.pos.y += push;
				pushCount++;
			}

			//! Check if player are falling then set player on grounded
			player.grounded = (player.velY < 0) ? true : player.grounded;
			player.velY = 0;

		} //! end of checking collision

	}//! end of checking player bottom

}

void CollisionUpdate(Player& player, f32 dt) {
	resolvePlayerCollision(player, g_currentMap, dt);
}



void CollisionUpdateWallFlags(Player& player)
{
	// Probe distance: small number in world units
	const float PROBE = 2.0f;
	float w = player.colliderSize.x;
	float h = player.colliderSize.y;

	// add little padding on left and right side, which checks if it overlaps with the solid tiles
	player.onWallLeft = checkAABBCollisionAt(player.pos.x - PROBE, player.pos.y, w, h);
	player.onWallRight = checkAABBCollisionAt(player.pos.x + PROBE, player.pos.y, w, h);

}
