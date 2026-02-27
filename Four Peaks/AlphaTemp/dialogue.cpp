#include "dialogue.hpp"
#include "graphics.hpp"

extern s8 gFontId;      // Font handle created in main.cpp


namespace UI {

    Dialog::Dialog() {
        currentIndex = 0;
        isShowing = false;

        // *****************************        TYPEWRITER                   **************************************************************************
        displayedChars = 0;
        typeWriterTimer = 0.0f;
        timePerChar = 1.0f / 20.0f;
        wordSize = 2.0f;


     

    }

    void Dialog::initialize() {


        // =========================================================
        // Prologue
        // =========================================================
        levelDialogs[99] = {
            "For generations, my family sought the four seasonal relics hidden atop this mountain.",
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
            "Remembering Dad's Note:",
            "The ice will betray your footing. Don't trust the ground.",
            "...but first, watch your step. Those spikes are unforgiving."
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
            "Heat kills faster than hunger. Find water - or faint.",
            "I can already feel the scorching heat.",
            "I need to find water bottles to stay cool."
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


    }


    //! check current level and show dialog for that level
    void Dialog::showForLevel(int levelID) {
        //! skip if already shown or no dialog exists at that level
        if (levelDialogs.find(levelID) == levelDialogs.end()) return;
        if (isShowing) return; 


        texts = levelDialogs[levelID];
        currentIndex = 0;
        displayedChars = 0;      
        typeWriterTimer = 0.0f;    
        isShowing = true;
    }

    void Dialog::update(float dt) {
        if (!isShowing) return;

        if (!playerNearSign) {
            //reset();
            isShowing = false;   
            return;
        }

        size_t currentTextLength = texts[currentIndex].length();

        if (displayedChars < currentTextLength) {
            typeWriterTimer += dt;
            if (typeWriterTimer >= timePerChar) {
                displayedChars++;
                typeWriterTimer = 0.0f;
            }
        }

        if (playerNearSign && AEInputCheckTriggered(AEVK_UP)) {
            if (currentIndex > 0) {
                currentIndex--;
                displayedChars = 0;
                typeWriterTimer = 0.0f;
            }
        }

        if (playerNearSign && AEInputCheckTriggered(AEVK_DOWN)) {
            if (displayedChars < currentTextLength) {
                displayedChars = currentTextLength;
            }
            else {
                if (currentIndex+1 < (int)texts.size()) {
                    currentIndex++;
                    displayedChars = 0;
                    typeWriterTimer = 0.0f;
                }
            }
        }
    }

    float Dialog::toNormalizedX(float pixelX) {
        return pixelX / (AEGfxGetWindowWidth() / 2.0f);
    }

    float Dialog::toNormalizedY(float pixelY) {
        return pixelY / (AEGfxGetWindowHeight() / 2.0f);
    }

    void Dialog::render()
    {
        if (!isShowing)
            return;


        //! record current camera coord and set to center first
        float oldX, oldY;
        AEGfxGetCamPosition(&oldX, &oldY);
        AEGfxSetCamPosition(0.0f, 0.0f);

        float boxX = 0.0f;
        float boxY = 300.0f;
        float boxWidth = 1000.0f;
        float boxHeight = 100.0f;
        gfx::drawRectangle({ boxX, boxY }, 0.0f, { boxWidth, boxHeight }, 0x88000000);

        if (currentIndex < texts.size()){
            std::string fullText = texts[currentIndex];
            std::string visibleText = fullText.substr(0, displayedChars);
                                                 

            const char* pText = visibleText.c_str();
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);

            float normX = boxX / 800.0f;  // world half width
            float normY = boxY / 450.0f;  // world half height

            f32 textWidth, textHeight;
            AEGfxGetPrintSize(gFontId, pText, wordSize, &textWidth, &textHeight);

            float drawX = normX - textWidth / 2.0f;
            float drawY = normY - textHeight / 2.0f;

            AEGfxPrint(gFontId, pText, drawX, drawY, wordSize, 1.0f, 1.0f, 1.0f, 1.0f);
        }

        AEGfxSetCamPosition(oldX, oldY);
    }


    

    void Dialog::reset() {
        isShowing = false;
        currentIndex = 0;
        displayedChars = 0;
        typeWriterTimer = 0.0f;
    
    }

    void Dialog::PLAYERNEARSIGN(bool detect) {
        playerNearSign = detect;
    }
   
}
	