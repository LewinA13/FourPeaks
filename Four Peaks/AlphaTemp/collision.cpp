#include "winter.hpp"
#include "player.hpp"
#include <iostream>
#include "collision.hpp"
#include "audio.hpp"
#include "dialogue.hpp"


std::vector<IceTrigger> g_triggeredIceTiles;
std::string g_currentScene = "";

const int mapRows = 20;
const int mapColm = 32;

struct TileRange {
	int colStart, colEnd;
	int rowStart, rowEnd;
};

int tileW = 50;
int tileH = 45;



int (*g_currentMap)[32] = nullptr;
int g_currentSignID = 0;  


TileRange calTileRange(f32 x, f32 y, f32 width, f32 height) {
	TileRange box{};

	const float fullH = static_cast<float>(AEGfxGetWindowHeight());       
	const float halfH = fullH / 2.0f;  

	const float halfW = AEGfxGetWindowWidth() / 2.0f;


	//! Convert x from center coordinates to left coordinates [-900, 900] to [0, 1600]
	float btmCoordPostX = x + halfW;
	//! Convert y from center coordinates to bottom coordinates [-450, 450] to [0, 900]
	float btmCoordPostY = y + halfH;        

	//! Since in different map, y will be different value, so use "fmodf()" get relatively heigth
	float screenY = fmodf(btmCoordPostY, fullH); 

	if (screenY < 0) screenY += fullH;  

	float left = btmCoordPostX - (width / 2.0f) + 0.1f;
	float right = btmCoordPostX + (width / 2.0f) - 0.1f;
	float top = screenY + (height / 2.0f) - 0.1f;
	float bottom = screenY - (height / 2.0f) + 0.1f;


	box.colStart = static_cast<int>((left) / tileW);
	box.colEnd = static_cast<int>((right) / tileW);
	box.rowStart = static_cast<int>((bottom) / tileH); 
	box.rowEnd = static_cast<int>((top) / tileH);

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

bool checkMapCollision(TileRange box, int levelLayout[][mapColm], float velY = -1.0f) {//! velY is defauly parameter, use for check one-way platform case

	//! return false once member in box equal to -1, means player out of screen
	if (box.colStart == -1) {
		return false;
	}

	for (int r = box.rowStart; r <= box.rowEnd; r++) {
		for (int c = box.colStart; c <= box.colEnd; c++) {
			int tile = levelLayout[r][c];

			//! Assume platform is 7 first, ****change in future
			if (tile == 7) {
				// If (player is dropping or staying) and (player bottom is near with platform), collision detect
				if (velY <= 0 && (box.rowStart == r || box.rowStart == r + 1)) {
					return true;
				}
				//! Skip this tile, check remaining tiles for collision
				continue;
			}

			//! start checking, if not equal 0, 10, 8, 2, 9  means collision with solid block, return true
			if (tile != 0 && tile != 10 && tile != 8 && tile != 2 && tile != 9 && tile != 19) {
				return true;
			}
		}
	}

	//! return false if not collision with solid block
	return false;
}

float calculateSpikeOverlapRatio(const Player& player, int row, TileRange box, int levelLayout[][mapColm]) {
	//! translate x center coord from (0) to (-1600)
	float halfWinW = (float)AEGfxGetWindowWidth() / 2.0f;
	float btmCoordPostX = player.pos.x + halfWinW;

	/*
		get player left and right for checking player body step on strikes,
		if using player center, cant accurate check player step how many strikes
	*/
	float playerLeft = btmCoordPostX - player.colliderSize.x / 2.0f;
	float playerRight = btmCoordPostX + player.colliderSize.x / 2.0f;

	float totalSpikeWidth = 0.0f;

	for (int checkCol = box.colStart; checkCol <= box.colEnd; checkCol++) {
		if (levelLayout[row][checkCol] == 2 || levelLayout[row][checkCol] == 9) {
			//! calc current spike tile left boundary 
			float tileLeft = static_cast<float>(checkCol * tileW);
			//! calc current spike tile right boundary 
			float tileRight = static_cast<float>((checkCol + 1) * tileW);

			// calc overlap area left boundary
			float overlapLeft = fmaxf(playerLeft, tileLeft);
			// calc overlap area right boundary
			float overlapRight = fminf(playerRight, tileRight);

			// if overlapRight > overlapLeft, means overlap exists
			if (overlapRight > overlapLeft) {
				// add the overlap width to total spike contact width
				totalSpikeWidth += (overlapRight - overlapLeft);
			}
		}
	}


	return totalSpikeWidth / player.colliderSize.x;
}




//! Check and change the type grounded type player step on
void checkGroundType(Player& player, TileRange box, int levelLayout[][mapColm]) {

	printf("Player Y: %.2f, Box rows: %d to %d, cols: %d to %d\n",
		player.pos.y, box.rowStart, box.rowEnd, box.colStart, box.colEnd);

	for (int r = box.rowStart; r <= box.rowEnd; r++) {
		for (int c = box.colStart; c <= box.colEnd; c++) {
			printf("Detect box [%i][%i] = %i\n",
				r,c,levelLayout[r][c]);
			switch (levelLayout[r][c]) {
			case 10: {
				player.currGroundType = Player::GroundType::CheckPoint;

				float halfWinW = (float)AEGfxGetWindowWidth() / 2.0f;
				float halfWinH = (float)AEGfxGetWindowHeight() / 2.0f;

				gfx::Vec2 res;
				int stageLevel = static_cast<int>(floorf((player.pos.y + halfWinH) / AEGfxGetWindowHeight()));

				res.x = (c + 0.5f) * tileW - halfWinW;
				res.y = (stageLevel * AEGfxGetWindowHeight()) + ((r + 1.0f) * tileH - halfWinH);
				player.checkpointScene = g_currentScene;
				PlayerSetRespawn(player, res);
				PlayerSaveCheckpoint(player, "checkpoint.txt");
				break;
			}

			case 4:
			case 6:
			case 7:
				player.currGroundType = Player::GroundType::Normal;
				break;

			
			case 2: {
				float halfWinH = (float)AEGfxGetWindowHeight() / 2.0f;
				float screenY = fmodf(player.pos.y + halfWinH, (float)AEGfxGetWindowHeight());
				if (screenY < 0) screenY += (float)AEGfxGetWindowHeight();
				float playerFeetY = screenY - player.colliderSize.y / 2.0f;

				//! set this val to check if playerfeet is over 60% of tile, further check overlap, otherwise dont check and return
				float tileDeadZoneY = (r + 0.6f) * tileH;

				//! true if spike tile is above player's feet row, means player body is inside spike
				bool spikeNotAtFeet = (r != box.rowStart);

				if (spikeNotAtFeet) {
					float overlapRatio = calculateSpikeOverlapRatio(player, r, box, levelLayout);
					if (overlapRatio > 0.0f) {
						player.currGroundType = Player::GroundType::Spikes;
						printf("detect spikes\n");

						return;
					}
				}
				else {
					//! player standing on spike, check if spike edge is next to empty tile
					//! if not player will see themselves step on edge of the spikes but still alive
					bool edgeCase = (box.colStart != box.colEnd) &&
						((levelLayout[r][box.colStart] == 0) || (levelLayout[r][box.colEnd] == 0));
					if (edgeCase || playerFeetY <= tileDeadZoneY) {
						float overlapRatio = calculateSpikeOverlapRatio(player, r, box, levelLayout);
						if (overlapRatio >= 0.5f) {
							player.currGroundType = Player::GroundType::Spikes;
							printf("detect spikes\n");

							return;
						}
					}
				}
				break;
			}
			case 9: {
				float halfWinH = (float)AEGfxGetWindowHeight() / 2.0f;
				float screenY = fmodf(player.pos.y + halfWinH, (float)AEGfxGetWindowHeight());
				if (screenY < 0) screenY += (float)AEGfxGetWindowHeight();
				float playerHeadY = screenY + player.colliderSize.y / 2.0f;

				bool spikeNotAtHead = (r != box.rowEnd);

				float tileDeadZoneY = (r + 0.6f) * tileH;

				if (spikeNotAtHead) {
					float overlapRatio = calculateSpikeOverlapRatio(player, r, box, levelLayout);
					if (overlapRatio > 0.0f) {
						player.currGroundType = Player::GroundType::Spikes;
						return;
					}
				}
				else {
					if (playerHeadY >= tileDeadZoneY) {
						float overlapRatio = calculateSpikeOverlapRatio(player, r, box, levelLayout);
						if (overlapRatio >= 0.5f) {
							player.currGroundType = Player::GroundType::Spikes;
							return;
						}
					}
				}
				break;
			}
			

			case 99:	// melon 
				player.melonsCollected += 1;
				PlayerSaveMelons(player, "melons.txt");
				levelLayout[r][c] = 0;
				break;



			case 1:
			{
				//! only when player above on ice &&  player (stay or down)
				if ((r == box.rowStart) && player.velY <= 0.0f) {
					g_triggeredIceTiles.push_back({ r, c });

					//This sets the ground that the player is currently standing on to be ice.
					player.currGroundType = Player::GroundType::Ice;
				}
				break;
			}


			case 8:  // water bottle
				player.heat += 0.35f;
				if (player.heat > player.maxHeat) player.heat = player.maxHeat;
				levelLayout[r][c] = 0;   // remove after pickup
				break;

			case 19:
				UI::gDialog.showForLevel(g_currentSignID);
				UI::gDialog.PLAYERNEARSIGN(true);
				break;
				
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
	if (player.dead) {
		return false;
	}
	return checkMapCollision(box, levelLayout,player.velY);
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

	//! Convert y from center coordinates to bottom coordinates [-450, 450] to [0, 900]
	float btmCoordPostY = player.pos.y + AEGfxGetWindowHeight()/2.0f;

	//! Since in different map, y will be different value, so use "fmodf()" get relatively heigth
	float screenY = fmodf(btmCoordPostY, static_cast<float>(AEGfxGetWindowHeight()));


	float feetY = screenY - player.colliderSize.y / 2.0f;

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



void applyGroundPhysics(Player& player) {
	switch (player.currGroundType) {
	case Player::GroundType::Normal:
		player.accel = 8.0f;
		player.decel = 4.0f;
		break;

	case Player::GroundType::Spikes:
		PlayerKill(player);
		break;

	case Player::GroundType::Ice:
		player.accel = 25.0f;
		player.decel = 1.0f;
		break;

	default:
		break;
	}
}


void CollisionUpdate(Player& player, f32 dt) {

	resolvePlayerCollision(player, g_currentMap, dt);

	applyGroundPhysics(player);
	if (!player.grounded) {
		player.accel = 8.0f;
		player.decel = 4.0f;
	}
	
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

/*!
	Calc the path between the player position (startPos) and 
    predicted position (endPos), spec for dashing.
    During dashing, the player tunneling through detect area, causing 
    checkpoints to be skipped. This function subdivides the path into 
    multiple segments (numSamples determine) to check if any part of the dash trajectory 
    intersects with a checkpoint or other ground types.
*/
void CheckPathForCheckpoint(Player& player, gfx::Vec2 startPos, gfx::Vec2 endPos) {
	const int numSamples = 10;

	//! Make collision detect less strict 
	float fakeWidth = player.colliderSize.x + 5.0f;
	float fakeHeight = player.colliderSize.y + 5.0f;

	for (int i = 0; i <= numSamples; ++i) {
		float t = (float)i / (float)numSamples;
		float sx = startPos.x + (endPos.x - startPos.x) * t;
		float sy = startPos.y + (endPos.y - startPos.y) * t;

		TileRange box = calTileRange(sx, sy, fakeWidth, fakeHeight);

		checkGroundType(player, box, g_currentMap);
	}
}