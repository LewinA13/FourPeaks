#include "stageselection.hpp"
#include "AEEngine.h"
#include "sprite.hpp"
#include "gamestate.hpp"

extern s8 gFontId;      // Font handle created in main.cpp

// ---------------------------------------------------------------------------
// Convert world-space coordinates to normalized screen coordinates (-1 ~ 1)
// ---------------------------------------------------------------------------
static AEVec2 WorldToNorm(float worldX, float worldY)
{
    float screenW = AEGfxGetWinMaxX() - AEGfxGetWinMinX();
    float screenH = AEGfxGetWinMaxY() - AEGfxGetWinMinY();
    return { worldX / (screenW * 0.5f), worldY / (screenH * 0.5f) };
}

// ---------------------------------------------------------------------------
// Convert (season, stage) pair into a global unlock index
// ---------------------------------------------------------------------------
static int SeasonStageToUnlockIndex(int seasonIndex, int stageIndex)
{
    return seasonIndex * 4 + stageIndex;
}

// ---------------------------------------------------------------------------
// Check if a season is unlocked (based on its first stage)
// ---------------------------------------------------------------------------
static bool IsSeasonUnlockedLocal(int seasonIndex)
{
    int firstStage = seasonIndex * 4;
    return gGame.unlockedStages[firstStage];
}

// ---------------------------------------------------------------------------
// Check if a specific stage is unlocked
// ---------------------------------------------------------------------------
static bool IsStageUnlockedLocal(int seasonIndex, int stageIndex)
{
    int idx = SeasonStageToUnlockIndex(seasonIndex, stageIndex);
    if (idx < 0 || idx >= 16) return false;
    return gGame.unlockedStages[idx];
}

// ---------------------------------------------------------------------------
// Get lock message for a season
// ---------------------------------------------------------------------------
static const char* GetSeasonLockMessage(int seasonIndex)
{
    switch (seasonIndex)
    {
    case 0: return "Please complete tutorial first";
    case 1: return "Please complete winter first";
    case 2: return "Please complete summer first";
    case 3: return "Please complete spring first";
    default: return "";
    }
}

// ---------------------------------------------------------------------------
// Get lock message for a stage
// ---------------------------------------------------------------------------
static const char* GetStageLockMessage(int seasonIndex, int stageIndex)
{
    switch (seasonIndex)
    {
    case 0: // winter
        switch (stageIndex)
        {
        case 1: return "Please complete Winter1 first";
        case 2: return "Please complete Winter2 first";
        case 3: return "Please complete Winter3 first";
        default: return "";
        }

    case 1: // summer
        switch (stageIndex)
        {
        case 1: return "Please complete Summer1 first";
        case 2: return "Please complete Summer2 first";
        case 3: return "Please complete Summer3 first";
        default: return "";
        }

    case 2: // spring
        switch (stageIndex)
        {
        case 1: return "Please complete Spring1 first";
        case 2: return "Please complete Spring2 first";
        case 3: return "Please complete Spring3 first";
        default: return "";
        }

    case 3: // autumn
        switch (stageIndex)
        {
        case 1: return "Please complete Autumn1 first";
        case 2: return "Please complete Autumn2 first";
        case 3: return "Please complete Autumn3 first";
        default: return "";
        }

    default:
        return "";
    }
}

// ---------------------------------------------------------------------------
// Calculate half width of rendered text (for centering)
// ---------------------------------------------------------------------------
static f32 textHalfWidth(const char* text, f32 scale = 1.0f)
{
    f32 w = 0.f, h = 0.f;
    AEGfxGetPrintSize(gFontId, text, scale, &w, &h);
    return w * 0.5f;
}

// ---------------------------------------------------------------------------
// Print text centered horizontally on screen
// ---------------------------------------------------------------------------
static void printCentered(f32 y, u32 argbColor, const char* text, f32 scale = 1.0f)
{
    f32 a = ((argbColor >> 24) & 0xFF) / 255.0f;
    f32 r = ((argbColor >> 16) & 0xFF) / 255.0f;
    f32 g = ((argbColor >> 8) & 0xFF) / 255.0f;
    f32 b = ((argbColor >> 0) & 0xFF) / 255.0f;
    f32 x = -textHalfWidth(text, scale);
    AEGfxPrint(gFontId, text, x, y, scale, r, g, b, a);
}

namespace game {

    // ---------------------------------------------------------------------------
    // Constructor: initialize selection state
    // ---------------------------------------------------------------------------
    StageSelect::StageSelect() :seasonIndex(0), stageIndex(0), selectingStage(false) {}

    // ---------------------------------------------------------------------------
    // Handle input and update selection state
    // Returns action code when a stage is selected
    // ---------------------------------------------------------------------------
    int StageSelect::update(float dt)
    {
        (void)dt;
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


    // ---------------------------------------------------------------------------
    // Render stage selection UI (season cards + stage cards)
    // ---------------------------------------------------------------------------
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

        // scale bigger if choosing season
        float CardW = selectingStage ? screenW * 0.42f : screenW * 0.52f;
        float CardH = selectingStage ? screenH * 0.40f : screenH * 0.58f;

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

            gfx::drawRectangle({ leftBgCX, cardY }, 0.0f, { CardW + 6, CardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[leftSeason], { leftBgCX, cardY }, 0.0f, { CardW, CardH }, 0, 0, 1, 1);

            // draw overlay last so it is not covered by another sprite draw
            gfx::drawRectangle(
                { leftBgCX, cardY },
                0.0f,
                { CardW, CardH },
                unlocked ? 0x77000000u : 0xCC444444u
            );

            if (!unlocked)
            {
                float textX = leftBgCX / (screenW * 0.5f) - 0.12f;
                float textY = cardY / (screenH * 0.5f) - 0.02f;

                AEGfxPrint(gFontId, GetSeasonLockMessage(leftSeason),
                    textX, textY, 0.7f,
                    1.0f, 1.0f, 1.0f, 1.0f);
            }
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

            gfx::drawRectangle({ rightBgCX, cardY }, 0.0f, { CardW + 6, CardH + 6 }, 0xFF333333u);
            gfx::drawSprite(seasonBg[rightSeason], { rightBgCX, cardY }, 0.0f, { CardW, CardH }, 0, 0, 1, 1);

            // draw overlay last so it stays visible
            gfx::drawRectangle(
                { rightBgCX, cardY },
                0.0f,
                { CardW, CardH },
                unlocked ? 0x77000000u : 0xCC444444u
            );

            if (!unlocked)
            {
                float textX = rightBgCX / (screenW * 0.5f) - 0.12f;
                float textY = cardY / (screenH * 0.5f) - 0.02f;

                AEGfxPrint(gFontId, GetSeasonLockMessage(rightSeason),
                    textX, textY, 0.7f,
                    1.0f, 1.0f, 1.0f, 1.0f);
            }
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

            gfx::drawRectangle({ 0.0f, cardY }, 0.0f, { CardW + 10, CardH + 10 }, borderColor);
            gfx::drawSprite(seasonBg[seasonIndex], { 0.0f, cardY }, 0.0f, { CardW, CardH }, 0, 0, 1, 1);

            // draw locked overlay after the final sprite draw
            if (!unlocked)
            {
                gfx::drawRectangle({ 0.0f, cardY }, 0.0f, { CardW, CardH }, 0xCC444444u);

                float textX = -0.18f;
                float textY = cardY / (screenH * 0.5f) - 0.02f;

                AEGfxPrint(gFontId, GetSeasonLockMessage(seasonIndex),
                    textX, textY, 0.9f,
                    1.0f, 1.0f, 1.0f, 1.0f);
            }
          
        }
        {
            float wx = 0.0f - CardW * 0.5f + 20.0f;
            float wy = cardY + CardH * 0.5f - 40.0f;
            AEVec2 pos = WorldToNorm(wx, wy);
            AEGfxPrint(gFontId, seasonNames[seasonIndex], pos.x, pos.y, 2.0f, seasonR[seasonIndex], seasonG[seasonIndex], seasonB[seasonIndex], 1.0f);
        }




        printCentered(0.88f, 0xFFFFFFCCu, "STAGE SELECTION", 1.8f);


        // ----------------------------------------------------------------
        //  Stage card
        // ----------------------------------------------------------------
        static const char* stageLabels[] = { "Stage 1", "Stage 2", "Stage 3", "Stage 4" };

        float stageCardW = screenW * 0.18f;
        float stageCardH = screenH * 0.22f;
        float stageRowY = minY + screenH * 0.22f;
        float stageSpacing = screenW * 0.21f;
        float stageStartX = 0.0f - stageSpacing * 1.5f;


        AEGfxTexture* stageTex[4][4] = {
        { sprite::winterStage(0), sprite::winterStage(1), sprite::winterStage(2), sprite::winterStage(3) },
        { sprite::summerStage(0), sprite::summerStage(1), sprite::summerStage(2), sprite::summerStage(3) },
        { sprite::springStage(0), sprite::springStage(1), sprite::springStage(2), sprite::springStage(3) },
        { sprite::autumnStage(0), sprite::autumnStage(1), sprite::autumnStage(2), sprite::autumnStage(3) },
        };



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

                AEGfxTexture* preview = stageTex[seasonIndex][i];
                if (preview)
                {
                    // Dim non-selected cards; full brightness on selected
                    gfx::drawSprite(preview, { cx, stageRowY }, 0.0f,
                        { stageCardW, stageCardH }, 0, 0, 1, 1);
                    // Darken overlay for non-selected cards
                    if (!sel)
                        gfx::drawRectangle({ cx, stageRowY }, 0.0f,
                            { stageCardW, stageCardH }, 0x88000000u);
                }

                char code[4];
                // get each season first alphabet
                code[0] = static_cast<char>(seasonNames[seasonIndex][0]);
                code[1] = static_cast<char>('1' + i);
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



                //AEGfxPrint(gFontId, code, tX, tY + 0.08f, 1.4f, tR, tG, tB, 1.0f);
                //AEGfxPrint(gFontId, stageLabels[i], tX - 0.02f, tY - 0.10f, 0.75f, tR, tG, tB, 0.9f);

                if (!unlocked)
                {
                    gfx::drawRectangle({ cx, stageRowY }, 0.0f,
                        { stageCardW, stageCardH }, 0xAA444444u);

                    AEGfxPrint(gFontId, GetStageLockMessage(seasonIndex, i),
                        tX - 0.04f, tY ,
                        0.52f, 1.0f, 1.0f, 1.0f, 1.0f);
                }
            }


            

            printCentered(-0.9f, 0xFFFFFFCCu, "LEFT / RIGHT to change season    ENTER to select    ESC to go back", 1.0f);
        }
        else
        {
            printCentered(-0.8f, 0xFFFFFFCCu, "LEFT / RIGHT to change season    ENTER to select    ESC to go back", 1.0f);
        }
    }

    // ---------------------------------------------------------------------------
    // Reset selection state to default
    // ---------------------------------------------------------------------------
    void StageSelect::reset()
    {
        seasonIndex = 0;
        stageIndex = 0;
        selectingStage = false;
    }
}