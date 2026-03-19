#include "stageselection.hpp"
#include "AEEngine.h"
#include "sprite.hpp"
#include "gamestate.hpp"

extern s8 gFontId;      // Font handle created in main.cpp

static AEVec2 WorldToNorm(float worldX, float worldY)
{
    float screenW = AEGfxGetWinMaxX() - AEGfxGetWinMinX();
    float screenH = AEGfxGetWinMaxY() - AEGfxGetWinMinY();
    return { worldX / (screenW * 0.5f), worldY / (screenH * 0.5f) };
}

static int SeasonStageToUnlockIndex(int seasonIndex, int stageIndex)
{
    return seasonIndex * 4 + stageIndex;
}

static bool IsSeasonUnlockedLocal(int seasonIndex)
{
    int firstStage = seasonIndex * 4;
    return gGame.unlockedStages[firstStage];
}

static bool IsStageUnlockedLocal(int seasonIndex, int stageIndex)
{
    int idx = SeasonStageToUnlockIndex(seasonIndex, stageIndex);
    if (idx < 0 || idx >= 16) return false;
    return gGame.unlockedStages[idx];
}

namespace game {

    StageSelect::StageSelect():seasonIndex(0), stageIndex(0), selectingStage(false) {}


    int StageSelect::update(float dt)
    {
        // ---- Choose season ----
        if (!selectingStage)
        {
            if (AEInputCheckTriggered(AEVK_LEFT))
                seasonIndex = (seasonIndex + 3) % 4;  // cycle by left
            if (AEInputCheckTriggered(AEVK_RIGHT))
                seasonIndex = (seasonIndex + 1) % 4;  // cycle by right

            if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
            {
                // only enter stage if this season has been unlocked
                if (IsSeasonUnlockedLocal(seasonIndex))
                {
                    selectingStage = true;
                    stageIndex = 0;
                }
            }

            if (AEInputCheckTriggered(AEVK_ESCAPE))
                return 2;  
        }
        else
        {
            if (AEInputCheckTriggered(AEVK_LEFT))
                stageIndex = (stageIndex + 3) % 4;
            if (AEInputCheckTriggered(AEVK_RIGHT))
                stageIndex = (stageIndex + 1) % 4;

            if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
            {
                // Only return an action if the selected stage is unlocked
                if (IsStageUnlockedLocal(seasonIndex, stageIndex))
                {
                    // seasonIndex 0~3, stageIndex 0~3
                    // action code: Winter=10~13, Summer=20~23, Spring=30~33, Autumn=40~43
                    return (seasonIndex + 1) * 10 + stageIndex;
                }
            }

            if (AEInputCheckTriggered(AEVK_ESCAPE))
            {
                selectingStage = false;  
            }
        }

        return 0;
    }


   
    void StageSelect::draw() const
    {

        float screenW = (float)AEGfxGetWindowWidth();
        float screenH = (float)AEGfxGetWindowHeight();
        float minX = -screenW * 0.5f;
        float maxX = screenW * 0.5f;
        float minY = -screenH * 0.5f;
        float maxY = screenH * 0.5f;

// ----------------------------------------------------------------
// set up beackground
// ----------------------------------------------------------------

        AEGfxTexture* bg = sprite::stgselectBackground();

        if (bg)
        {
            gfx::drawSprite(bg, { (minX + maxX) * 0.5f, (minY + maxY) * 0.5f }, 0.0, { (maxX - minX), (maxY - minY) }, 0, 0, 1, 1);
        }


// ----------------------------------------------------------------
// season bg + name + color
// ----------------------------------------------------------------
        // 0 - Winter,  1 - Summer,  2 - Spring,  3 - Autumn
        AEGfxTexture* seasonBg[4] = {
            sprite::background(),      
            sprite::summerBackground(),
            sprite::springBackground(),
            sprite::autumnBackground()
        };

        static const char* seasonNames[] = { "WINTER", "SUMMER", "SPRING", "AUTUMN" };

        static const u32 seasonBorderColor[] = {
            0xFF88CCFFu,  // Winter - light blue
            0xFFFFCC00u,  // Summer - golden yellow
            0xFF88FF88u,  // Spring - light green
            0xFFFF8844u,  // Autumn - orange
        };

        // Use for text color           Winter Summer Spring Autumn
        static const float seasonR[] = { 0.53f, 1.0f,  0.53f, 1.0f };
        static const float seasonG[] = { 0.80f, 0.80f, 1.0f,  0.53f };
        static const float seasonB[] = { 1.0f,  0.0f,  0.53f, 0.27f };

// ----------------------------------------------------------------
// Season card 
// ----------------------------------------------------------------

        AEGfxTexture* centerframetex = sprite::centerframe();

        // scale bigger if choosing season
        float CardW = selectingStage ? screenW * 0.42f : screenW * 0.52f;
        float CardH = selectingStage ? screenH * 0.30f : screenH * 0.48f;

        float mainCardW = selectingStage ? screenW * 0.42f : screenW * 0.52f;
        float mainCardH = selectingStage ? screenH * 0.30f : screenH * 0.48f;

        // offset to determine how width side card show
        float sideCardW = screenW * 0.16f;

        float cardY = 0.0f + screenH * 0.05f;

        int leftSeason = (seasonIndex + 3) % 4;
        int rightSeason = (seasonIndex + 1) % 4;

        float leftBgCX = minX + sideCardW - CardW * 0.5f;
        float rightBgCX = maxX - sideCardW + CardW * 0.5f;

        // ----------------------------------------------------------------
        // left half
        // ----------------------------------------------------------------
        if (seasonBg[leftSeason])
        {
            bool unlocked = IsSeasonUnlockedLocal(leftSeason);

            gfx::drawRectangle({ leftBgCX, cardY }, 0.0f, { mainCardW + 6, mainCardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[leftSeason], { leftBgCX, cardY }, 0.0f, { mainCardW, mainCardH }, 0, 0, 1, 1);

            // Locked seasons get a stronger dark overlay.
            gfx::drawRectangle({ leftBgCX, cardY }, 0.0f, { mainCardW, mainCardH },
                unlocked ? 0x77000000u : 0xCC444444u);

            // hong yang code below
            gfx::drawRectangle({ leftBgCX, cardY }, 0.0f, { CardW + 6, CardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[leftSeason], { leftBgCX, cardY }, 0.0f, { CardW, CardH }, 0, 0, 1, 1);
            gfx::drawRectangle({ leftBgCX, cardY }, 0.0f, { CardW, CardH }, 0x77000000u);
        }
        // show grey if not being selected
        {
            float wx = AEGfxGetWinMinX() + 10.0f;
            float wy = cardY + CardH * 0.5f - 30.0f;
            AEVec2 pos = WorldToNorm(wx, wy);
            AEGfxPrint(gFontId, seasonNames[leftSeason], pos.x, pos.y, 1.0f, 0.6f, 0.6f, 0.6f, 1.0f);
        }

        // ----------------------------------------------------------------
        // right half
        // ----------------------------------------------------------------
        if (seasonBg[rightSeason])
        {
            bool unlocked = IsSeasonUnlockedLocal(rightSeason);

            gfx::drawRectangle({ rightBgCX, cardY }, 0.0f, { mainCardW + 6, mainCardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[rightSeason], { rightBgCX, cardY }, 0.0f, { mainCardW, mainCardH }, 0, 0, 1, 1);

            // Locked seasons get a stronger dark overlay.
            gfx::drawRectangle({ rightBgCX, cardY }, 0.0f, { mainCardW, mainCardH },
                unlocked ? 0x77000000u : 0xCC444444u);

            //hy code below
            gfx::drawRectangle({ rightBgCX, cardY }, 0.0f, { CardW + 6, CardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[rightSeason], { rightBgCX, cardY }, 0.0f, { CardW, CardH }, 0, 0, 1, 1);
            gfx::drawRectangle({ rightBgCX, cardY }, 0.0f, { CardW, CardH }, 0x77000000u);
        }
        {
            float wx = AEGfxGetWinMaxX() - sideCardW + 10.0f;
            float wy = cardY + CardH * 0.5f - 30.0f;
            AEVec2 pos = WorldToNorm(wx, wy);
            AEGfxPrint(gFontId, seasonNames[rightSeason], pos.x, pos.y, 1.0f, 0.6f, 0.6f, 0.6f, 1.0f);
        }
      
        // ----------------------------------------------------------------
        // center 
        // ----------------------------------------------------------------
        if (seasonBg[seasonIndex])
        {
            bool unlocked = IsSeasonUnlockedLocal(seasonIndex);
            u32 borderColor = unlocked ? seasonBorderColor[seasonIndex] : 0xFF555555u;

            gfx::drawRectangle({ 0.0f, cardY }, 0.0f, { mainCardW + 10, mainCardH + 10 }, borderColor);
            gfx::drawSprite(seasonBg[seasonIndex], { 0.0f, cardY }, 0.0f, { mainCardW, mainCardH }, 0, 0, 1, 1);

            // Grey overlay when this season is still locked.
            if (!unlocked)
                gfx::drawRectangle({ 0.0f, cardY }, 0.0f, { mainCardW, mainCardH }, 0xCC444444u);

            // hy code below
            gfx::drawRectangle({ 0.0f, cardY }, 0.0f, { CardW + 10, CardH + 10 }, seasonBorderColor[seasonIndex]);
            //gfx::drawSprite(centerframetex, { -10.0f, cardY - 120.0f }, 0.0f, { mainCardW + 500, mainCardH + 590 }, 0, 0, 1, 1);
            gfx::drawSprite(seasonBg[seasonIndex], { 0.0f, cardY }, 0.0f, { CardW, CardH }, 0, 0, 1, 1);
          
          
        }
        {
            float wx = 0.0f - CardW * 0.5f + 20.0f;
            float wy = cardY + CardH * 0.5f - 40.0f;
            AEVec2 pos = WorldToNorm(wx, wy);
            AEGfxPrint(gFontId, seasonNames[seasonIndex], pos.x, pos.y, 2.0f, seasonR[seasonIndex], seasonG[seasonIndex], seasonB[seasonIndex], 1.0f);}




        AEGfxPrint(gFontId, "STAGE SELECTION",
            -0.28f, 0.88f, 1.8f, 1.0f, 1.0f, 0.8f, 1.0f);

// ----------------------------------------------------------------
//  Stage card
// ----------------------------------------------------------------
        static const char* stageLabels[] = { "Stage 1", "Stage 2", "Stage 3", "Stage 4" };

        float stageCardW = screenW * 0.18f;
        float stageCardH = screenH * 0.22f;
        float stageRowY = minY + screenH * 0.22f;
        float stageSpacing = screenW * 0.21f;
        float stageStartX = 0.0f - stageSpacing * 1.5f;

      
    
        if (selectingStage)
        {
            for (int i = 0; i < 4; ++i)
            {
                float cx = stageStartX + i * stageSpacing;

                bool sel = selectingStage && (i == stageIndex);
                bool unlocked = IsStageUnlockedLocal(seasonIndex, i);

                u32 borderCol = 0xFF555555u;
                u32 bgCol = 0xFF111111u;

                if (!unlocked)
                {
                    borderCol = 0xFF444444u;
                    bgCol = 0xFF333333u;
                }
                else if (sel)
                {
                    borderCol = seasonBorderColor[seasonIndex];
                    bgCol = 0xFF222222u;
                }

                gfx::drawRectangle({ cx, stageRowY }, 0.0f,
                    { stageCardW + 6, stageCardH + 6 }, borderCol);
                gfx::drawRectangle({ cx, stageRowY }, 0.0f,
                    { stageCardW, stageCardH }, bgCol);

                char code[4];
                // get each season first alphabet
                code[0] = seasonNames[seasonIndex][0];
                code[1] = '1' + i;
                code[2] = '\0';

                float tX = cx / (screenW * 0.5f) - 0.04f;
                float tY = stageRowY / (screenH * 0.5f);
                float tR = 0.85f;
                float tG = 0.85f;
                float tB = 0.85f;

                if (!unlocked)
                {
                    tR = 0.55f;
                    tG = 0.55f;
                    tB = 0.55f;
                }
                else if (sel)
                {
                    tR = seasonR[seasonIndex];
                    tG = seasonG[seasonIndex];
                    tB = seasonB[seasonIndex];
                }



                AEGfxPrint(gFontId, code, tX, tY + 0.08f, 1.4f, tR, tG, tB, 1.0f);
                AEGfxPrint(gFontId, stageLabels[i], tX - 0.02f, tY - 0.10f, 0.75f, tR, tG, tB, 0.9f);

                if (!unlocked)
                {
                    AEGfxPrint(gFontId, "LOCKED", tX - 0.03f, tY - 0.22f, 0.60f, 0.65f, 0.65f, 0.65f, 0.9f);
                }
            }


            char selText[64];
            sprintf_s(selText, "SELECT STAGE: %c%d",
                seasonNames[seasonIndex][0], stageIndex + 1);
            AEGfxPrint(gFontId, selText,
                -0.20f, -0.78f, 1.0f, 1.0f, 1.0f, 0.6f, 1.0f);
            AEGfxPrint(gFontId, "UP / DOWN to choose    ENTER to start    ESC to go back",
                -0.45f, -0.90f, 0.75f, 0.6f, 0.6f, 0.6f, 1.0f);
        }
        else
        {
            AEGfxPrint(gFontId, "LEFT / RIGHT to change season    ENTER to select    ESC to go back",
                -0.50f, -0.90f, 0.75f, 0.6f, 0.6f, 0.6f, 1.0f);
        }
    }

    void StageSelect::reset()
    {
        seasonIndex = 0;
        stageIndex = 0;
        selectingStage = false;
    }
}