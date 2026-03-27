#include "winter.hpp"
#include "player.hpp"
#include <iostream>
#include "collision.hpp"
#include "audio.hpp"
#include "dialogue.hpp"
#include "graphics.hpp"

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
std::vector<breakableTileTrigger> g_triggeredIceTiles;
std::vector<breakableTileTrigger> g_triggeredbrkTiles;
std::string g_currentScene = "";

const int mapRows = 20;
const int mapColm = 32;

int tileW = 50;
int tileH = 45;

int   (*g_currentMap)[32] = nullptr;
int   g_currentSignID = 0;
float g_currentY = 0;

// ---------------------------------------------------------------------------
// Internal structs
// ---------------------------------------------------------------------------
struct TileRange {
    int colStart, colEnd;
    int rowStart, rowEnd;
};

// ---------------------------------------------------------------------------
// Player position helpers function
// ---------------------------------------------------------------------------
static float getPlayerScreenY(const Player& player)
{
    float halfWinH = (float)AEGfxGetWindowHeight() * 0.5f;
    return (player.pos.y + halfWinH) - g_currentY;
}

static float getPlayerFeetY(const Player& player)
{
    return getPlayerScreenY(player) - player.colliderSize.y * 0.5f;
}

static float getPlayerHeadY(const Player& player)
{
    return getPlayerScreenY(player) + player.colliderSize.y * 0.5f;
}

// ---------------------------------------------------------------------------
// Tile range calculation
// Converts player world position to tile grid indices.
// ---------------------------------------------------------------------------
TileRange calTileRange(f32 x, f32 y, f32 width, f32 height)
{
    TileRange box{};

    const float fullH = (float)AEGfxGetWindowHeight();
    const float halfH = fullH * 0.5f;
    const float halfW = (float)AEGfxGetWindowWidth() * 0.5f;

    // Convert center-origin to screen-space 
    float btmCoordPostX = x + halfW;
    float btmCoordPostY = y + halfH;

    // get a Y value that maps correctly to the tile grid of that stage
    float screenY = btmCoordPostY - g_currentY;

    // Invalidate if player is completely outside the current layer
    float playerBottom = screenY - height * 0.5f;
    if (playerBottom > fullH || screenY < 0) {
        box.colStart = box.colEnd = box.rowStart = box.rowEnd = -1;
        return box;
    }

    // +- 1.0f is to shrink player collider box
    float left = btmCoordPostX - width * 0.5f + 1.0f;
    float right = btmCoordPostX + width * 0.5f - 1.0f;
    float top = screenY + height * 0.5f - 1.0f;
    float bottom = screenY - height * 0.5f + 1.0f;

    box.colStart = static_cast<int>(left / tileW);
    box.colEnd = static_cast<int>(right / tileW);
    box.rowStart = static_cast<int>(bottom / tileH);
    box.rowEnd = static_cast<int>(top / tileH);

    // Clamp to map bounds
    box.colStart = max(box.colStart, 0);
    box.colEnd = min(box.colEnd, mapColm - 1);
    box.rowStart = max(box.rowStart, 0);
    box.rowEnd = min(box.rowEnd, mapRows - 1);

    if (box.rowStart > box.rowEnd || box.colStart > box.colEnd)
        box.colStart = box.colEnd = box.rowStart = box.rowEnd = -1;

    return box;
}

// ---------------------------------------------------------------------------
// Solid tile check
// ---------------------------------------------------------------------------
static bool isSolidTile(int tile)
{
    switch (tile) {
    case 1:   // ice
    case 3:   // tile (brown)
    case 4:   // tile (grey)
    case 5:   // tile
    case 6:   // underground
    case 7:   // platform
    case 11:  // winter ice tile
    case 16:  // summer tile top
    case 17:  // summer tile bottom
    case 30:  // Spring tile
        return true;
    default:
        return false;
    }
}

// ---------------------------------------------------------------------------
// Map collision check
// playerHeadY : screen-space head Y, used for breakable tile upper-half collision
// ---------------------------------------------------------------------------
bool checkMapCollision(TileRange box, int levelLayout[][mapColm],
    float velY = 0.0f, float playerHeadY = 0.0f)
{
    if (box.colStart == -1) return false;

    for (int r = box.rowStart; r <= box.rowEnd; r++) {
        for (int c = box.colStart; c <= box.colEnd; c++) {
            int tile = levelLayout[r][c];

            if (tile == 15) {
                // Breakable tile (special case): only the upper half is solid.
                float tileMiddleY = r * tileH + tileH * 0.5f;
                if (playerHeadY >= tileMiddleY)
                    return true;
                continue;
            }

            if (isSolidTile(tile))
                return true;
        }
    }
    return false;
}

// helper function for damage tile
static void checkDamageTile(Player& player, int r, TileRange box,
    int levelLayout[][mapColm], int tileCase)
{
    float screenY = getPlayerScreenY(player);
    float tileDeadZoneY = (r + 0.6f) * tileH;

    switch (tileCase)
    {
    case 2:
    case 24:
    {
        float playerFeetY = getPlayerFeetY(player);
        bool  spikeNotAtFeet = (r != box.rowStart);
        Player::GroundType damageType = (tileCase == 24)? Player::GroundType::Fire : Player::GroundType::Spikes;

        if (spikeNotAtFeet) {
               player.currGroundType = damageType;
        }
        else {
            if (playerFeetY <= tileDeadZoneY)
                player.currGroundType = damageType;
        }
        break;
    }
    case 9:
    {
        float playerHeadY = getPlayerHeadY(player);
        bool  spikeNotAtHead = (r != box.rowEnd);

        if (spikeNotAtHead) {
            //if (calculateDamageOverlapRatio(player, r, box, levelLayout) > 0.0f)
                player.currGroundType = Player::GroundType::Spikes;
        }
        else {
            if (playerHeadY >= tileDeadZoneY)
                //if (calculateDamageOverlapRatio(player, r, box, levelLayout) >= 0.5f)
                    player.currGroundType = Player::GroundType::Spikes;
        }
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Ground type detection
// Iterates over the player's tile box and applies gameplay effects per tile.
// ---------------------------------------------------------------------------
void checkGroundType(Player& player, TileRange box, int levelLayout[][mapColm])
{
    for (int r = box.rowStart; r <= box.rowEnd; r++) {
        for (int c = box.colStart; c <= box.colEnd; c++) {

            // immediately return if out of bound
            if (box.colStart == -1) return;  

            printf("Current levelLayout[%i][%i]: %i\n", r, c, levelLayout[r][c]);
            switch (levelLayout[r][c]) {

            case 10: // checkpoint
            {
                player.currGroundType = Player::GroundType::CheckPoint;

                float halfWinW = (float)AEGfxGetWindowWidth() * 0.5f;
                float halfWinH = (float)AEGfxGetWindowHeight() * 0.5f;
                int   stageLevel = static_cast<int>(
                    floorf((player.pos.y + halfWinH) / AEGfxGetWindowHeight()));

                gfx::Vec2 res;
                res.x = (c + 0.5f) * tileW - halfWinW;
                res.y = (stageLevel * AEGfxGetWindowHeight()) + ((r + 1.0f) * tileH - halfWinH);

                player.checkpointScene = g_currentScene;
                PlayerSetRespawn(player, res);
                PlayerSaveCheckpoint(player, "checkpoint.txt");
                break;
            }

            case 23: // grass
                player.currGroundType = Player::GroundType::Grass;
                break;

            case 4: case 5: case 6: case 7: case 30: // normal solid tiles
                player.currGroundType = Player::GroundType::Normal;
                break;

            case 2: case 9: case 24: // spikes / fire
                checkDamageTile(player, r, box, levelLayout, levelLayout[r][c]);
                break;

            case 25: // saw
                player.currGroundType = Player::GroundType::Saw;
                break;

            case 8:	// melon 
                // only collect once
                if (!IsMelonCollected(g_currentScene.c_str(), r, c))
                {
                    player.melonsCollected += 1;
                    MarkMelonCollected(g_currentScene.c_str(), r, c);

                    // remove from current in-memory map immediately
                    levelLayout[r][c] = 0;

                    // if player has not touched any checkpoint yet, create a valid save anchor
                    if (player.checkpointScene.empty())
                    {
                        player.checkpointScene = g_currentScene;
                        PlayerSetRespawn(player, PlayerGetFeetWorld(player));
                    }

                    PlayerSaveCheckpoint(player, "checkpoint.txt");
                }
                else
                {
                    // safety: if already collected, keep it removed
                    levelLayout[r][c] = 0;
                }
                break;

            //Issue Lies Here with Tile ID.
            case 1: // ice tile 
            {
                // Only trigger when player feet are near the tile top.
                // Prevents triggering when player brushes the tile from the side.
                float feetY = getPlayerFeetY(player);
                float tileTopY = static_cast<float>(r + 1) * static_cast<float>(tileH);

                // trigger when player feet are within the top 5% of the ice tile
                if (feetY >= tileTopY - static_cast<float>(tileH) * 0.05f) {
                    g_triggeredIceTiles.push_back({ r, c });
                    player.currGroundType = Player::GroundType::Ice;
                }

                break;
            }

            case 15: // breakable tile
            {
                // Mirror the upper-half solid logic in checkMapCollision.
                float feetY = getPlayerFeetY(player);
                float tileTopY = static_cast<float>(r + 1) * static_cast<float>(tileH);

                if (feetY >= tileTopY - static_cast<float>(tileH) * 0.05f)
                    g_triggeredbrkTiles.push_back({ r, c });
                break;
            }

            case 18: // water bottle
                player.heat = min(player.heat + 0.35f, player.maxHeat);
                levelLayout[r][c] = 0;
                break;

            case 19:
            {
                float halfW = (float)AEGfxGetWindowWidth() * 0.5f;
                float halfH = (float)AEGfxGetWindowHeight() * 0.5f;

                float signWorldX = (c + 0.5f) * tileW - halfW;
                float signWorldY = g_currentY - halfH + (r + 0.5f) * tileH;

                UI::gDialog.setSignPos(signWorldX, signWorldY);
                UI::gDialog.playerNearSignBoard(true);
                UI::gDialog.showForLevel(g_currentSignID);
                break;
            }
         

            case 34: // winter artifacts
                UI::gDialog.triggerFromArtifact(13);
                levelLayout[r][c] = 0;
                break;

            case 31: // summer artifacts
                UI::gDialog.triggerFromArtifact(23);
                levelLayout[r][c] = 0;
                break;

            case 32: // spring artifacts
                UI::gDialog.triggerFromArtifact(33);
                levelLayout[r][c] = 0;
                break;

            case 33: // autumn artifacts
                UI::gDialog.triggerFromArtifact(44);
                levelLayout[r][c] = 0;
                break;

            default:
                break;
            }
        }
    }
}

static bool checkAABBCollisionAt(float x, float y, float w, float h)
{
    TileRange box = calTileRange(x, y, w, h);
    return checkMapCollision(box, g_currentMap);
}

// ---------------------------------------------------------------------------
// Spawn resolution ? pushes player out of geometry after a respawn.
// ---------------------------------------------------------------------------
void CollisionResolveSpawn(Player& player)
{
    const float step = 0.5f;

    // Push up until clear
    TileRange box{};  
    for (int i = 0; i < 200; ++i) {
        box = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);
        if (!checkMapCollision(box, g_currentMap, player.velY, getPlayerHeadY(player))) break;
        player.pos.y += step;
    }

    // Snap down a small amount so player lands on the surface
    const int maxSteps = (int)(10.0f / step);
    bool landed = false;
    for (int i = 0; i < maxSteps; ++i) {
        player.pos.y -= step;
        box = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);
        if (checkMapCollision(box, g_currentMap, player.velY, getPlayerHeadY(player))) {
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

// ---------------------------------------------------------------------------
// Per-frame collision resolution
// ---------------------------------------------------------------------------
void resolvePlayerCollision(Player& player, int levelLayout[][mapColm], f32 dt)
{
  

    float currentY = player.pos.y;
    float oldY = currentY - player.velY * dt;

    // --- Horizontal collision ---
    // Temporarily revert to last frame Y so horizontal and vertical are resolved independently.
    player.pos.y = oldY;
    TileRange hBox = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);

    if (player.velX != 0.0f && checkMapCollision(hBox, levelLayout, player.velY, getPlayerHeadY(player))) {
        float push = (player.velX > 0.0f) ? -0.5f : 0.5f;
        for (int i = 0; i < 200; ++i) {
            player.pos.x += push;
            hBox = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);
            if (!checkMapCollision(hBox, levelLayout, player.velY, getPlayerHeadY(player))) {
                break;
            }
        }
        player.velX = 0.0f;
    }

    // Clamp to screen left / right borders
    const float halfWinW = (float)AEGfxGetWindowWidth() * 0.5f;
    const float halfPlayerW = player.colliderSize.x * 0.5f;
    if (player.pos.x - halfPlayerW < -halfWinW || player.pos.x + halfPlayerW > halfWinW) {
        player.pos.x = AEClamp(player.pos.x, -halfWinW + halfPlayerW, halfWinW - halfPlayerW);
        player.velX = 0.0f;
        player.horzSpeed = 0.0f;
    }

    // --- Vertical collision ---
    player.pos.y = currentY;

    float fullH = (float)AEGfxGetWindowHeight();
    float btmCoordPostY = player.pos.y + fullH * 0.5f;
    float screenY = btmCoordPostY - g_currentY;

    // Kill player if they fall below the layer
    if (screenY < -50.0f) {
        PlayerKill(player);
        return;
    }

    float feetY = getPlayerFeetY(player);
    // GROUND_Y: the minimum screen-space Y before the player is considered out of bounds.
    // Derived from window height so it adapts if resolution changes.
    const float GROUND_Y = -(float)AEGfxGetWindowHeight() * 0.5f;

    TileRange vBox = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);


    if (feetY > GROUND_Y  && checkMapCollision(vBox, levelLayout, player.velY, getPlayerHeadY(player))) {
        float push = (player.velY > 0) ? -0.5f : 0.5f;
        for (int i = 0; i < 200; ++i) {
            player.pos.y += push;
            vBox = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);
            if (!checkMapCollision(vBox, levelLayout, player.velY, getPlayerHeadY(player))) {
                break;
            }
            
        }


        player.grounded = (player.velY < 0) ? true : player.grounded;
        player.velY = 0;
    }
}

// ---------------------------------------------------------------------------
// Ground physics 
// ---------------------------------------------------------------------------
void applyGroundPhysics(Player& player)
{
    switch (player.currGroundType) {
    case Player::GroundType::Normal:
        player.accel = 8.0f;
        player.decel = 4.0f;
        player.speed = 120.0f;
        break;

    case Player::GroundType::Spikes:
    case Player::GroundType::Saw:
    case Player::GroundType::Fire:
        PlayerKill(player);
        break;

    case Player::GroundType::Ice:
        player.accel = 25.0f;
        player.decel = 1.0f;
        break;

    case Player::GroundType::Grass:
        player.speed = 50.0f;
        break;

    default:
        break;
    }
}

void CollisionUpdate(Player& player, f32 dt)
{

    player.currGroundType = Player::GroundType::Normal;
    resolvePlayerCollision(player, g_currentMap, dt);

    // Player has been pushed out of tiles by resolvePlayerCollision, 
    // and calTileRange shrinks the box by 1.0f at the bottom,
    // so probe 2.0f downward to ensure checkGroundType can detect the tile below player's feet
    TileRange box = calTileRange(player.pos.x, player.pos.y - 2.0f, player.colliderSize.x, player.colliderSize.y);
    checkGroundType(player, box, g_currentMap);

    applyGroundPhysics(player);

    if (!player.grounded) {
        player.accel = 8.0f;
        player.decel = 4.0f;
    }
}

void CollisionUpdateWallFlags(Player& player)
{
    const float PROBE = 2.0f;
    float w = player.colliderSize.x;
    float h = player.colliderSize.y;

    player.onWallLeft = checkAABBCollisionAt(player.pos.x - PROBE, player.pos.y, w, h);
    player.onWallRight = checkAABBCollisionAt(player.pos.x + PROBE, player.pos.y, w, h);
}

// ---------------------------------------------------------------------------
// Dash checkpoint sampling
// Subdivides the dash path into segments to catch any tiles the player
// tunnelled through due to high speed.
// ---------------------------------------------------------------------------
void CheckPathForCheckpoint(Player& player, gfx::Vec2 startPos, gfx::Vec2 endPos) {
    const int numSamples = 10;
    // Slightly enlarged collider so checkpoint detection is less strict
    float fakeW = player.colliderSize.x + 5.0f;
    float fakeH = player.colliderSize.y + 5.0f;

    for (int i = 0; i <= numSamples; ++i) {
        float t = (float)i / (float)numSamples;
        float sx = startPos.x + (endPos.x - startPos.x) * t;
        float sy = startPos.y + (endPos.y - startPos.y) * t;

        TileRange box = calTileRange(sx, sy, fakeW, fakeH);
        checkGroundType(player, box, g_currentMap);
    }
}