#include "stageselection.hpp"
#include "AEEngine.h"
#include "sprite.hpp"

extern s8 gFontId;      // Font handle created in main.cpp


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
                selectingStage = true;
                stageIndex = 0;  
            }

            if (AEInputCheckTriggered(AEVK_ESCAPE))
                return 2;  
        }
        else
        {
            if (AEInputCheckTriggered(AEVK_UP))
                stageIndex = (stageIndex + 3) % 4;
            if (AEInputCheckTriggered(AEVK_DOWN))
                stageIndex = (stageIndex + 1) % 4;

            if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
            {
                // seasonIndex 0~3, stageIndex 0~3
                // action code: Winter=10~13, Summer=20~23, Spring=30~33, Autumn=40~43
                return (seasonIndex + 1) * 10 + stageIndex;
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

        float minX = AEGfxGetWinMinX(), maxX = AEGfxGetWinMaxX();
        float minY = AEGfxGetWinMinY(), maxY = AEGfxGetWinMaxY();
        float screenW = maxX - minX;
        float screenH = maxY - minY;

        //float screenW = AEGfxGetWindowWidth();
        //float screenH = AEGfxGetWindowHeight();

        float centerX = (minX + maxX) * 0.5f;
        float centerY = (minY + maxY) * 0.5f;

// ----------------------------------------------------------------
// set up beackground
// ----------------------------------------------------------------
        AEGfxSetBackgroundColor(0.05f, 0.05f, 0.1f);

      

// ----------------------------------------------------------------
// season bg + name + color
// ----------------------------------------------------------------
        AEGfxTexture* seasonBg[4] = {
            sprite::background(),
            sprite::summerBackground(),
            sprite::springBackground(),
            sprite::autumnBackground()
        };

        static const char* seasonNames[] = { "WINTER", "SUMMER", "SPRING", "AUTUMN" };

        static const u32 seasonBorderColor[] = {
            0xFF88CCFFu,   
            0xFFFFCC00u,   
            0xFF88FF88u,  
            0xFFFF8844u,   
        };

        // Use for text color
        static const float seasonR[] = { 0.53f, 1.0f,  0.53f, 1.0f };
        static const float seasonG[] = { 0.80f, 0.80f, 1.0f,  0.53f };
        static const float seasonB[] = { 1.0f,  0.0f,  0.53f, 0.27f };

// ----------------------------------------------------------------
// Season card 
// ----------------------------------------------------------------
        float mainCardW = screenW * 0.52f;
        float mainCardH = screenH * 0.30f;
        float sideCardW = screenW * 0.16f;
        float cardY = centerY + screenH * 0.05f;

        int leftSeason = (seasonIndex + 3) % 4;
        int rightSeason = (seasonIndex + 1) % 4;

        float leftBgCX = minX + sideCardW - mainCardW * 0.5f;
        float rightBgCX = maxX - sideCardW + mainCardW * 0.5f;

    // ----------------------------------------------------------------
    // left half
    // ----------------------------------------------------------------
        if (seasonBg[leftSeason])
        {
            gfx::drawRectangle({ leftBgCX, cardY }, 0.0f,{ mainCardW + 6, mainCardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[leftSeason],{ leftBgCX, cardY }, 0.0f,{ mainCardW, mainCardH }, 0, 0, 1, 1);
            gfx::drawRectangle({ leftBgCX, cardY }, 0.0f,{ mainCardW, mainCardH }, 0x77000000u);
        }
        // show grey if not being selected
        AEGfxPrint(gFontId, seasonNames[leftSeason], minX / (screenW * 0.5f) + 0.02f, cardY / (screenH * 0.5f) + 0.05f, 1.2f, 0.6f, 0.6f, 0.6f, 1.0f);

    // ----------------------------------------------------------------
    // right half
    // ----------------------------------------------------------------
        if (seasonBg[rightSeason])
        {
            gfx::drawRectangle({ rightBgCX, cardY }, 0.0f,
                { mainCardW + 6, mainCardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[rightSeason],
                { rightBgCX, cardY }, 0.0f,
                { mainCardW, mainCardH }, 0, 0, 1, 1);
            gfx::drawRectangle({ rightBgCX, cardY }, 0.0f,
                { mainCardW, mainCardH }, 0x77000000u);
        }
        AEGfxPrint(gFontId, seasonNames[rightSeason],
            (maxX / (screenW * 0.5f)) - 0.25f,
            cardY / (screenH * 0.5f) + 0.05f,
            1.2f, 0.6f, 0.6f, 0.6f, 1.0f);

    // ----------------------------------------------------------------
    // center card
    // ----------------------------------------------------------------
        if (seasonBg[seasonIndex])
        {
            gfx::drawRectangle({ centerX, cardY }, 0.0f,
                { mainCardW + 10, mainCardH + 10 },
                seasonBorderColor[seasonIndex]);
            gfx::drawSprite(seasonBg[seasonIndex],
                { centerX, cardY }, 0.0f,
                { mainCardW, mainCardH }, 0, 0, 1, 1);
        }

        AEGfxPrint(gFontId, seasonNames[seasonIndex],
            -0.48f, 0.22f, 2.5f,
            seasonR[seasonIndex], seasonG[seasonIndex], seasonB[seasonIndex], 1.0f);

    
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
        float stageStartX = centerX - stageSpacing * 1.5f;

        for (int i = 0; i < 4; ++i)
        {
            float cx = stageStartX + i * stageSpacing;
            bool  sel = selectingStage && (i == stageIndex);

            // choose season color, otherwise choose grey
            u32 borderCol = sel ? seasonBorderColor[seasonIndex] : 0xFF444444u;
            u32 bgCol = sel ? 0xFF222222u : 0xFF111111u;

            gfx::drawRectangle({ cx, stageRowY }, 0.0f,
                { stageCardW + 6, stageCardH + 6 }, borderCol);
            gfx::drawRectangle({ cx, stageRowY }, 0.0f,
                { stageCardW, stageCardH }, bgCol);

            char code[4];
            code[0] = seasonNames[seasonIndex][0];
            code[1] = '1' + i;
            code[2] = '\0';

            float tX = cx / (screenW * 0.5f) - 0.04f;
            float tY = stageRowY / (screenH * 0.5f);
            float tR = sel ? seasonR[seasonIndex] : 0.85f;
            float tG = sel ? seasonG[seasonIndex] : 0.85f;
            float tB = sel ? seasonB[seasonIndex] : 0.85f;

            AEGfxPrint(gFontId, code, tX, tY + 0.08f, 1.4f, tR, tG, tB, 1.0f);
            AEGfxPrint(gFontId, stageLabels[i], tX - 0.02f, tY - 0.10f, 0.75f, tR, tG, tB, 0.9f);
        }

    
        if (selectingStage)
        {
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
}