#include "summer_s1.hpp"
#include "player.hpp"
#include <iostream>


const int mapRows = 20;
const int mapColm = 32;

struct TileRange{
    int colStart, colEnd;
    int rowStart, rowEnd;
};

 int tileW = 50;
 int tileH = 45;

int levelLayout[20][32] = {
    // Row 0 (bottom ground)
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0,0,1,2,2,2,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},
    {1,1,2,2,2,2,2,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
    // Row 16-19 (empty sky)
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0}
};

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

    //! return false once member in box equal to -1, means out of screen
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

//! This function will get player tile and check collision with maps 
bool checkPlayerCollision(Player& player, int levelLayout[][mapColm]) {
    TileRange box = calTileRange(player.pos.x, player.pos.y, player.colliderSize.x, player.colliderSize.y);
    return checkMapCollision(box, levelLayout);
}

void resolvePlayerCollision(Player &player, int levelLayout[][mapColm], f32 dt) {

    static const float GROUND_Y = -450.0f;

    float currentY = player.pos.y;
    float oldY = currentY - player.velY * dt;
    //! Set player y coordinates to coordinates before this frame, to seperately handling horizontal and vertical collision
    player.pos.y = oldY;  

    //! Check whether collide with solid block
    if (checkPlayerCollision(player, levelLayout)) {

        //! Check player moving left or right and determine "push" value
        f32 push = (player.velX > 0) ? -0.5f : 0.5f;
        //! Set "pushCount" to 0 in case happens infinity loops
        int pushCount = 0;

        //! use while loops to force push player until not detect collision with solid block
        while (checkPlayerCollision(player, levelLayout) && pushCount < 100) {

            player.pos.x += push;
            pushCount++;

        }

        player.velX = 0;
    }

    //! Set back player y coordinates to this frame
    player.pos.y = currentY;


    float feetY = player.pos.y - player.colliderSize.y / 2.0f;

    if (feetY > GROUND_Y + 10.0f) {  //! Check player bottom whether greater than "GROUND_Y", if lower, "PlayerUpdate" will handle
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

    resolvePlayerCollision(player, levelLayout,dt);

}