#include "dialogue.hpp"
#include "graphics.hpp"
#include "sprite.hpp"
#include "gamestate.hpp"
extern s8 gFontId;      // Font handle created in main.cpp


namespace UI {

    Dialog::Dialog() 
    : currentIndex(0)
    , isShowing(false)
    , displayedChars(0)
    , typeWriterTimer(0.0f)
    , timePerChar(1.0f / 20.0f)
    , wordSize(1.6f)
    , isAutoDialog(false)
    , autoDialogCloseTimer(0.0f)
    , autoDialogCloseDelay(1.0f)
    , waitingForInput(false)
    , signWorldPos{0.0f, 0.0f}   
    , currentLevelID(-1)
    , playerNearSign(false)
    {}

    void Dialog::initialize() {


        // =========================================================
        // Prologue
        // =========================================================
        levelDialogs[50] = {
            "For generations, my family sought the relics.",
            "Four seasonal artifacts, hidden atop this mountain.",
            "No archaeologist has ever returned with all four.",
            "My father... and his father before him... never came back.",
            "All they left behind was this map - and their notes.",
            "The mountain only reveals its path during specific seasons.",
            "This time... I will finish what they started."
        };

   
        // =========================================================
        // Tutorial stage
        // =========================================================
        levelDialogs[0] = {  // Tutorial 1
            "I've trained my whole life for this climb.",
            "Move with care. Every step matters.",
            "Press [W]/ [A]/ [S]/ [D] for moving",
            "[SHIFT] for dashing",
            "[SPACE] for jumping"
            "[L] for climbing"
        };

        levelDialogs[1] = {  // Tutorial 2
            "The flag play a crucial part!!!",
            "Touch it to save your progress."
        };

        levelDialogs[2] = {  // Tutorial 3
            "At the peak lie four relics - Frost, Flame, Wind, and Harvest.",
            "Winter is the first path to open. The climb begins now."
        };


        // =========================================================
        // Winter stage
        // =========================================================
        levelDialogs[10] = {  // WinterS1
            "Remembering Dad's Note:",
            "The ice will betray your footing. Don't trust the ground.",
            "...but first, watch your step. Those spikes are unforgiving."
        };

        levelDialogs[11] = {  // WinterS2
        };

        levelDialogs[12] = {  // WinterS3
            "The ice... it doesn't feel stable."
        };

        levelDialogs[13] = {  // WinterS4 sign
            "Relic of Frost obtained.",
            "One relic down. I'm closer than they ever were."
        };


        // =========================================================
        // Summer stage
        // =========================================================
        levelDialogs[20] = {  // SummerS1
            "Dad's Note:",
            "Watch the heat bar on the top-left, ",
            "find water bottles to stay cool!",
        };

        levelDialogs[21] = {  // SummerS2
            "The heat is getting worse the higher I climb."
        };

        levelDialogs[22] = {  // SummerS3
            "Water... I need water.",
            "Dad never mentioned it would be this bad."
        };

        levelDialogs[23] = {  // SummerS4 sign
            "Relic of Flame obtained.",
            "The mountain grows harsher. But so do I."
        };

        // =========================================================
        // Spring stage
        // =========================================================
        levelDialogs[30] = {  // SpringS1
            "The winds push against me.",
            "Every step forward is a fight."
        };

        levelDialogs[31] = {  // SpringS2
            "Dad's Note:",
            "The giant mushrooms can launch you higher.",
            "Use them wisely."
        };

        levelDialogs[32] = {  // SpringS3
            "The wind howls louder the higher I climb.",
            "One wrong jump... and I'm gone."
        };

        levelDialogs[33] = {  // SpringS4 sign
            "Relic of Wind obtained.",
            "Just one more season."
        };

        // =========================================================
        // Autumn stage
        // =========================================================
        levelDialogs[40] = {  // AutumnS1
            "Dad's Note:",
            "When the leaves fall thick... you won't see what's ahead.",
            "Wonder what that means."
        };

        levelDialogs[41] = {  // AutumnS2
            "The ground is buried in leaves.",
            "Every step drags."
        };

        levelDialogs[42] = {  // AutumnS3
            "I can't see the traps beneath.",
            "I have to trust my instincts."
        };

        levelDialogs[43] = {  // AutumnS4 sign
            "This must be the final stretch."
        };

        levelDialogs[44] = {  // AutumnS5 relic
            "Relic of Harvest obtained.",
            "Four relics.",
            "The mountain is conquered."
        };

        levelDialogs[45] = {  // All artifacts achievement
            "Achievement unlocked:",
            "Congrats! You have collected all 4 artifacts.",
            "Frost, Flame, Wind, and Harvest are all yours."
        };


    }


    //! check current level and show dialog for that level
    void Dialog::showForLevel(int levelID) {
        if (levelDialogs.find(levelID) == levelDialogs.end()) return;

        if (currentLevelID == levelID && isShowing) {
            return;
        }

        if (!waitingForInput && !isShowing) {
            waitingForInput = true;
            currentLevelID = levelID;
            return;
        }
    }


    void Dialog::update(float dt) {
        // artifact dialog
        if (isAutoDialog) {
            if (!isShowing) return;

            size_t currentTextLength = texts[currentIndex].length();
            bool isLastLine = (currentIndex + 1 >= (int)texts.size());
            bool isFullyTyped = (displayedChars >= currentTextLength);

            if (!isFullyTyped) {
                typeWriterTimer += dt;
                if (typeWriterTimer >= timePerChar) {
                    displayedChars++;
                    typeWriterTimer = 0.0f;
                }
            }

            if (isLastLine && isFullyTyped) {
                autoDialogCloseTimer += dt;
                if (autoDialogCloseTimer >= autoDialogCloseDelay) {
                    reset();
                    return;
                }
            }

            if (AEInputCheckTriggered(AEVK_DOWN)) {
                if (!isFullyTyped) {
                    displayedChars = currentTextLength;
                }
                else if (!isLastLine) {
                    currentIndex++;
                    displayedChars = 0;
                    typeWriterTimer = 0.0f;
                    autoDialogCloseTimer = 0.0f;
                }
                else {
                    reset();
                }
            }

            return; 
        }

        // ---- signboard dialog ----
        if (!playerNearSign) {
            isShowing = false;
            waitingForInput = false;
            return;
        }

        if (waitingForInput || isShowing) {
            if (AEInputCheckTriggered(AEVK_E)) {
                if (isShowing) {
                    isShowing = false;
                    waitingForInput = true;
                    currentIndex = 0;
                    displayedChars = 0;
                    typeWriterTimer = 0.0f;
                }
                else {
                    waitingForInput = false;
                    texts = levelDialogs[currentLevelID];
                    currentIndex = 0;
                    displayedChars = 0;
                    typeWriterTimer = 0.0f;
                    isShowing = true;
                }
                return;
            }
        }

        if (!isShowing) return;

        size_t currentTextLength = texts[currentIndex].length();
        bool isLastLine = (currentIndex + 1 >= (int)texts.size());
        bool isFullyTyped = (displayedChars >= currentTextLength);

        if (!isFullyTyped) {
            typeWriterTimer += dt;
            if (typeWriterTimer >= timePerChar) {
                displayedChars++;
                typeWriterTimer = 0.0f;
            }
        }

        if (AEInputCheckTriggered(AEVK_UP)) {
            if (currentIndex > 0) {
                currentIndex--;
                displayedChars = 0;
                typeWriterTimer = 0.0f;
            }
        }

        if (AEInputCheckTriggered(AEVK_DOWN)) {
            if (!isFullyTyped) {
                displayedChars = currentTextLength;
            }
            else if (!isLastLine) {
                currentIndex++;
                displayedChars = 0;
                typeWriterTimer = 0.0f;
            }
        }
    }

    void Dialog::render()
    {

        if ((waitingForInput || isShowing) && playerNearSign && !isAutoDialog) {
            float oldX, oldY;
            AEGfxGetCamPosition(&oldX, &oldY);
            AEGfxSetCamPosition(0.0f, 0.0f);

            const char* hint = isShowing ? "Press [E] to close" : "Press [E] to read";
            float scale = 1.2f;

            f32 tw, th;
            AEGfxGetPrintSize(gFontId, hint, scale, &tw, &th);

            float halfW = (float)AEGfxGetWindowWidth() * 0.5f;
            float halfH = (float)AEGfxGetWindowHeight() * 0.5f;

            float screenX = signWorldPos.x - oldX;
            float screenY = signWorldPos.y - oldY + 40.0f; 

            float normX = screenX / halfW;
            float normY = screenY / halfH;

            float drawX = normX - tw * 0.5f;
            float drawY = normY - th * 0.5f;

            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxPrint(gFontId, hint, drawX, drawY, scale, 1.0f, 1.0f, 1.0f, 1.0f); 

            AEGfxSetCamPosition(oldX, oldY);
        }


        if (!isShowing)
            return;

        //! record current camera coord
        float oldX, oldY;
        AEGfxGetCamPosition(&oldX, &oldY);
        // drawSprite using world coordinates, so set to (0,0) first
        AEGfxSetCamPosition(0.0f, 0.0f);


        

        float boxX = 0.0f;
        float boxY = 300.0f;
        float boxWidth = 950.0f;
        float boxHeight = 200.0f;
        AEGfxTexture* textboxTexture = sprite::textbox();
        AEGfxTexture* textboxUpTexture = sprite::textboxUp();
        AEGfxTexture* textboxDownTexture = sprite::textboxDown();


        // If not add this line code, sprite dont show up
        gfx::drawRectangle({ -99999.0f, -99999.0f }, 0.0f, { 1.0f, 1.0f }, 0x00000000u);

        if (textboxTexture) {
            gfx::drawSprite(textboxTexture, { boxX, boxY }, 0.0f, { boxWidth, boxHeight }, 0.0f, 0.0f, 1.0f, 1.0f);
        }



        if (currentIndex > 0) {
            gfx::drawSprite(textboxUpTexture, { 400, 310 }, 0.0f, { 30.0f, 30.0f }, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        if (currentIndex + 1 < (int)texts.size() || displayedChars < texts[currentIndex].length()) {
            gfx::drawSprite(textboxDownTexture, { 400, 270 }, 0.0f, { 30.0f, 30.0f }, 0.0f, 0.0f, 1.0f, 1.0f);
        }
     

        if (currentIndex < texts.size() && (playerNearSign || isAutoDialog)){
            std::string fullText = texts[currentIndex];

            // typewriter effect
            std::string visibleText = fullText.substr(0, displayedChars);
                                                 
            // .str() is convert from "std::string" to "const char*" 
            // reason is let "AEGfxPrint" can use
            const char* pText = visibleText.c_str();

            AEGfxSetBlendMode(AE_GFX_BM_BLEND);

            // norrmalise it so can handle with textWidth/textHeight 
            float normX = boxX / (AEGfxGetWindowWidth() * 0.5f);  // world half width
            float normY = boxY / (AEGfxGetWindowHeight() * 0.5f);  // world half height

            f32 textWidth, textHeight;
            // get textWidth/ textHeight
            AEGfxGetPrintSize(gFontId, pText, wordSize, &textWidth, &textHeight);

            // get center x and y
            // "AEGfxPrint" start from left up 
            float drawX = normX - textWidth / 2.0f;
            float drawY = normY - textHeight / 2.0f;

            AEGfxPrint(gFontId, pText, drawX, drawY, wordSize, 1.0f, 1.0f, 1.0f, 1.0f);
        }

  
        AEGfxSetCamPosition(oldX, oldY);
    }

    void Dialog::playerNearSignBoard(bool detect) {
        playerNearSign = detect;
    }

    void Dialog::triggerAutoDialog(int levelID) {
        if (levelDialogs.find(levelID) == levelDialogs.end()) return;

        currentLevelID = levelID;
        texts = levelDialogs[levelID];
        currentIndex = 0;
        displayedChars = 0;
        typeWriterTimer = 0.0f;
        isAutoDialog = true;
        autoDialogCloseTimer = 0.0f;
        isShowing = true;
    }

    void Dialog::reset() {
        isShowing = false;
        isAutoDialog = false;
        autoDialogCloseTimer = 0.0f;
        currentIndex = 0;
        displayedChars = 0;
        waitingForInput = false;  
    }
   
    // helper funct for render 
    void Dialog::setSignPos(float x, float y) {
        signWorldPos.x = x;
        signWorldPos.y = y;
    }

    bool Dialog::dialogBoxShowing() const {
        return isShowing;
    }
}
	