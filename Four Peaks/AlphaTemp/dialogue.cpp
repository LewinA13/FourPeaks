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
   
        levelDialogs[1] = {  // Tutorial 1
            "Press W/ A/ S/ D for moving",
            "Shift for dashing",
            "Space for jumping"
        };

        levelDialogs[2] = {  // WinterS2
           /* "Stage 2!",
            "Watch out for spikes."*/
            "Where is my song?"
        };

        levelDialogs[3] = {  // WinterS3
            /*"You're doing great!",
            "The summit is near."*/
            "Count by the count by the left foot count yay!",
            "1"
        };

        levelDialogs[4] = {  // WinterS4
            //"One last push!"
            "Cant hear you lah"
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

        size_t currentTextLength = texts[currentIndex].length();

        //! check if current showing char is less than length of string
        if (displayedChars < currentTextLength) {

            typeWriterTimer += dt;

            //! if reach the timing set before, display another char, reset typewriterTimer
            if (typeWriterTimer >= timePerChar) {
                displayedChars++;        
                typeWriterTimer = 0.0f;     
            }
        }

        if (AEInputCheckTriggered(AEVK_RETURN)) {
            // skip typewriter animation if ENTER pressed mid-sentence
            if (displayedChars < currentTextLength) {
                displayedChars = currentTextLength;
            }
            else {//! if string finished

                currentIndex++;

                if (currentIndex >= texts.size()) {
                    isShowing = false;  
                }
                else { 
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
   
}
	