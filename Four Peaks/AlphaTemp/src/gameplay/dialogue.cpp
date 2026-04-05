// ----------------------------------------------------------------------------
// Done By: Hong Yang
// ----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Copyright (C) 2026 Team Game++ (Four Peaks)
// All rights reserved.
//
// This file is part of the Four Peaks project. All code, design, and original
// assets are the work of LewinA and team members unless otherwise stated.
//
// Audio assets are sourced from Soundly and used under appropriate licensing.
//
// Reproduction, distribution, or modification of this file or its contents,
// in whole or in part, without prior written permission is strictly prohibited.
//
//---------------------------------------------------------------------------

#include "gameplay/dialogue.hpp"
#include "engine/graphics.hpp"
#include "engine/sprite.hpp"
#include "core/gamestate.hpp"
extern s8 gFontId;

namespace UI {

    // -------------------------------------------------------------------------
    // Initialises all dialog state to default values.
    // -------------------------------------------------------------------------
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
        , signWorldPos{ 0.0f, 0.0f }
        , currentLevelID(-1)
        , playerNearSign(false)
    {
    }

    // -------------------------------------------------------------------------
    // Registers all dialog text for every level/event ID. Keyed on the level ID
    // integer used throughout the game (0-50). Must be called once at game start.
    // -------------------------------------------------------------------------
    void Dialog::initialize()
    {
        // -------------------------------------------------------------------------
        // Prologue
        // -------------------------------------------------------------------------
        levelDialogs[50] = {
            "For generations, my family sought the relics.",
            "Four seasonal artifacts, hidden atop this mountain.",
            "No archaeologist has ever returned with all four.",
            "My father... and his father before him... never came back.",
            "All they left behind was this map - and their notes.",
            "The mountain only reveals its path during specific seasons.",
            "This time... I will finish what they started."
        };

        // -------------------------------------------------------------------------
        // Tutorial stages
        // -------------------------------------------------------------------------
        levelDialogs[0] = {
            "I've trained my whole life for this climb.",
            "Move with care. Every step matters.",
            "Press [W]/ [A]/ [S]/ [D] for moving",
            "[SHIFT] or [K] for dashing",
            "[SPACE] for jumping",
            "[L] for climbing"
        };

        levelDialogs[1] = {
            "The flag marks my progress. I should touch it.",
            "Holding [SPACE] longer gets me higher.",
            "These walls... I can climb them.",
            "Press [L] to grip. Jump off to wall jump."
        };

        levelDialogs[2] = {
            "At the peak lie four relics - Frost, Flame, Wind, and Harvest.",
            "Winter is the first path to open. The climb begins now."
        };

        // -------------------------------------------------------------------------
        // Winter stage
        // -------------------------------------------------------------------------
        levelDialogs[10] = {
            "Remembering Dad's Note:",
            "The ice will betray your footing. Don't trust the ground.",
            "...but first, watch your step. Those spikes are unforgiving."
        };

        levelDialogs[11] = {};

        levelDialogs[12] = {
            "The ice... it doesn't feel stable."
        };

        levelDialogs[13] = {
            "Relic of Frost obtained.",
            "One relic down. I'm closer than they ever were."
        };

        // -------------------------------------------------------------------------
        // Summer stage
        // -------------------------------------------------------------------------
        levelDialogs[20] = {
            "Dad's Note:",
            "Watch the heat bar on the top-left, ",
            "find water bottles to stay cool!",
        };

        levelDialogs[21] = {
            "The heat is getting worse the higher I climb."
        };

        levelDialogs[22] = {
            "Water... I need water.",
            "Dad never mentioned it would be this bad."
        };

        levelDialogs[23] = {
            "Relic of Flame obtained.",
            "The mountain grows harsher. But so do I."
        };

        // -------------------------------------------------------------------------
        // Spring stage
        // -------------------------------------------------------------------------
        levelDialogs[30] = {
            "The winds push against me.",
            "Every step forward is a fight."
        };

        levelDialogs[31] = {
            "Dad's Note:",
            "The giant mushrooms can launch you higher.",
            "Use them wisely."
        };

        levelDialogs[32] = {
            "The wind howls louder the higher I climb.",
            "One wrong jump... and I'm gone."
        };

        levelDialogs[33] = {
            "Relic of Wind obtained.",
            "Just one more season."
        };

        // -------------------------------------------------------------------------
        // Autumn stage
        // -------------------------------------------------------------------------
        levelDialogs[40] = {
            "Dad's Note:",
            "When the leaves fall thick... you won't see what's ahead.",
            "Wonder what that means."
        };

        levelDialogs[41] = {
            "The ground is buried in leaves.",
            "Every step drags."
        };

        levelDialogs[42] = {
            "I can't see the traps beneath.",
            "I have to trust my instincts."
        };

        levelDialogs[43] = {
            "This must be the final stretch."
        };

        levelDialogs[44] = {
            "Relic of Harvest obtained.",
            "Four relics.",
            "The mountain is conquered."
        };

        // -------------------------------------------------------------------------
        // Achievement notifications (auto-dialogs)
        // -------------------------------------------------------------------------
        levelDialogs[45] = {
            "Achievement unlocked:",
            "Congrats! You have collected all 4 artifacts.",
            "Frost, Flame, Wind, and Harvest are all yours."
        };

        levelDialogs[46] = {
            "Achievement unlocked:",
            "You have collected all 53 melons.",
            "You are now considered one of the greatest archaeologists ever."
        };
    }

    // -------------------------------------------------------------------------
    // Queues dialog for levelID to appear on the next [E] press, unless the
    // dialog for that level is already open. The first call puts the system
    // into waitingForInput mode; the player must press [E] to open the box.
    // -------------------------------------------------------------------------
    void Dialog::showForLevel(int levelID)
    {
        if (levelDialogs.find(levelID) == levelDialogs.end()) return;

        if (currentLevelID == levelID && isShowing) {
            return;
        }

        if (!waitingForInput && !isShowing) {
            waitingForInput = true;
            currentLevelID = levelID;
        }
    }

    // -------------------------------------------------------------------------
    // Advances the typewriter reveal, handles player input, and auto-closes
    // artifact dialogs. Two distinct code paths: isAutoDialog (artifact/event)
    // and the normal sign-board flow.
    // -------------------------------------------------------------------------
    void Dialog::update(float dt)
    {
        // -------------------------------------------------------------------------
        // Artifact / scripted auto-dialog path.
        // -------------------------------------------------------------------------
        if (isAutoDialog) {
            if (!isShowing) return;

            size_t currentTextLength = texts[currentIndex].length();
            bool isLastLine = (currentIndex + 1 >= (int)texts.size());
            bool isFullyTyped = (displayedChars >= currentTextLength);

            // -------------------------------------------------------------------------
            // Advance the typewriter one character at a time.
            // -------------------------------------------------------------------------
            if (!isFullyTyped) {
                typeWriterTimer += dt;
                if (typeWriterTimer >= timePerChar) {
                    displayedChars++;
                    typeWriterTimer = 0.0f;
                }
            }

            // -------------------------------------------------------------------------
            // Auto-close after a short delay once the last line is fully typed.
            // -------------------------------------------------------------------------
            if (isLastLine && isFullyTyped) {
                autoDialogCloseTimer += dt;
                if (autoDialogCloseTimer >= autoDialogCloseDelay) {
                    reset();
                    return;
                }
            }

            // -------------------------------------------------------------------------
            // [ENTER] skips typewriter on the current line, advances to the next,
            // or closes the dialog if on the final line.
            // -------------------------------------------------------------------------
            if (AEInputCheckTriggered(AEVK_RETURN)) {
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

        // -------------------------------------------------------------------------
        // Sign-board dialog path.
        // -------------------------------------------------------------------------

        // -------------------------------------------------------------------------
        // If the player stepped away from the sign, hide the dialog immediately.
        // -------------------------------------------------------------------------
        if (!playerNearSign) {
            isShowing = false;
            waitingForInput = false;
            return;
        }

        if (waitingForInput || isShowing) {
            if (AEInputCheckTriggered(AEVK_E)) {
                if (isShowing) {
                    // -------------------------------------------------------------------------
                    // Close the dialog and return to the waiting-for-input state.
                    // -------------------------------------------------------------------------
                    isShowing = false;
                    waitingForInput = true;
                    currentIndex = 0;
                    displayedChars = 0;
                    typeWriterTimer = 0.0f;
                }
                else {
                    // -------------------------------------------------------------------------
                    // Open the dialog and start the typewriter from the first line.
                    // -------------------------------------------------------------------------
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

        // -------------------------------------------------------------------------
        // Advance the typewriter character reveal.
        // -------------------------------------------------------------------------
        if (!isFullyTyped) {
            typeWriterTimer += dt;
            if (typeWriterTimer >= timePerChar) {
                displayedChars++;
                typeWriterTimer = 0.0f;
            }
        }

        // -------------------------------------------------------------------------
        // [ENTER] skips to end of the current line or advances to the next.
        // The last line of a sign dialog is closed via [E], not [ENTER].
        // -------------------------------------------------------------------------
        if (AEInputCheckTriggered(AEVK_RETURN)) {
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

    // -------------------------------------------------------------------------
    // Draws the "Press E" proximity hint above the sign when the player is
    // nearby but has not opened the dialog, then draws the dialog box and
    // typewriter text when the box is open. All drawing is done in camera-space
    // (camera reset to 0,0).
    // -------------------------------------------------------------------------
    void Dialog::render()
    {
        if ((waitingForInput || isShowing) && playerNearSign && !isAutoDialog) {
            // -------------------------------------------------------------------------
            // Temporarily reset camera so the hint is drawn in screen space.
            // -------------------------------------------------------------------------
            float oldX, oldY;
            AEGfxGetCamPosition(&oldX, &oldY);
            AEGfxSetCamPosition(0.0f, 0.0f);

            if (!isShowing) {
                const char* hint = "Press [E] to read";
                float scale = 1.2f;

                f32 tw, th;
                AEGfxGetPrintSize(gFontId, hint, scale, &tw, &th);

                float halfW = (float)AEGfxGetWindowWidth() * 0.5f;
                float halfH = (float)AEGfxGetWindowHeight() * 0.5f;

                // -------------------------------------------------------------------------
                // Convert sign world position to screen-space NDC.
                // -------------------------------------------------------------------------
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
        }

        if (!isShowing) return;

        // -------------------------------------------------------------------------
        // Record and override camera so dialog draws fixed to the screen.
        // -------------------------------------------------------------------------
        float oldX, oldY;
        AEGfxGetCamPosition(&oldX, &oldY);
        AEGfxSetCamPosition(0.0f, 0.0f);

        float boxX = 0.0f;
        float boxY = 300.0f;
        float boxWidth = 950.0f;
        float boxHeight = 200.0f;

        AEGfxTexture* textboxTexture = sprite::textbox();

        // -------------------------------------------------------------------------
        // A dummy off-screen draw is required to prime the sprite system before
        // drawSprite will render correctly.
        // -------------------------------------------------------------------------
        gfx::drawRectangle({ -99999.0f, -99999.0f }, 0.0f, { 1.0f, 1.0f }, 0x00000000u);

        if (textboxTexture) {
            gfx::drawSprite(textboxTexture, { boxX, boxY }, 0.0f,
                { boxWidth, boxHeight }, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        if (currentIndex < texts.size() && (playerNearSign || isAutoDialog)) {
            std::string fullText = texts[currentIndex];
            std::string visibleText = fullText.substr(0, displayedChars);

            const char* pText = visibleText.c_str();

            AEGfxSetBlendMode(AE_GFX_BM_BLEND);

            // -------------------------------------------------------------------------
            // Convert box centre from world space to NDC for AEGfxPrint.
            // -------------------------------------------------------------------------
            float normX = boxX / (AEGfxGetWindowWidth() * 0.5f);
            float normY = boxY / (AEGfxGetWindowHeight() * 0.5f);

            f32 textWidth, textHeight;
            AEGfxGetPrintSize(gFontId, pText, wordSize, &textWidth, &textHeight);

            // -------------------------------------------------------------------------
            // Centre the text within the dialog box.
            // -------------------------------------------------------------------------
            float drawX = normX - textWidth / 2.0f;
            float drawY = normY - textHeight / 2.0f;

            AEGfxPrint(gFontId, pText, drawX, drawY, wordSize, 1.0f, 1.0f, 1.0f, 1.0f);

            // -------------------------------------------------------------------------
            // Draw the advance / close hint at the bottom of the dialog box.
            // -------------------------------------------------------------------------
            float hintScale = 1.0f;
            float boxBottom = (boxY - boxHeight * 0.5f) / (AEGfxGetWindowHeight() * 0.5f);

            const char* hintText = nullptr;
            if (isAutoDialog) {
                hintText = (currentIndex + 1 < (int)texts.size() ||
                    displayedChars < texts[currentIndex].length())
                    ? "[Press \" Enter \" to continue]"
                    : "[Press \" Enter \" to skip]";
            }
            else {
                hintText = (currentIndex + 1 < (int)texts.size() ||
                    displayedChars < texts[currentIndex].length())
                    ? "[Press \" Enter \" to continue]"
                    : "[Press \" E \" to close]";
            }

            f32 hintWidth{}, hintHeight{};
            AEGfxGetPrintSize(gFontId, hintText, hintScale, &hintWidth, &hintHeight);

            float hintX = normX - hintWidth / 2.0f;
            float hintY = boxBottom + 0.03f;

            AEGfxPrint(gFontId, hintText, hintX, hintY, hintScale, 1.0f, 1.0f, 0.8f, 1.0f);
        }

        // -------------------------------------------------------------------------
        // Restore the previous camera position after all HUD drawing.
        // -------------------------------------------------------------------------
        AEGfxSetCamPosition(oldX, oldY);
    }

    // -------------------------------------------------------------------------
    // Notifies the dialog system whether the player is within sign range.
    // Passing false will dismiss any open sign dialog on the next update call.
    // -------------------------------------------------------------------------
    void Dialog::playerNearSignBoard(bool detect)
    {
        playerNearSign = detect;
    }

    // -------------------------------------------------------------------------
    // Immediately starts an auto-closing dialog for levelID. Used for artifact
    // pickups and scripted cutscene triggers. The dialog closes after the final
    // line has been visible for autoDialogCloseDelay seconds.
    // -------------------------------------------------------------------------
    void Dialog::triggerAutoDialog(int levelID)
    {
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

    // -------------------------------------------------------------------------
    // Clears all transient dialog state and returns the system to idle.
    // -------------------------------------------------------------------------
    void Dialog::reset()
    {
        isShowing = false;
        isAutoDialog = false;
        autoDialogCloseTimer = 0.0f;
        currentIndex = 0;
        displayedChars = 0;
        waitingForInput = false;
    }

    // -------------------------------------------------------------------------
    // Stores the world-space coordinates of the nearest sign so render() can
    // position the "Press E" hint label above it.
    // -------------------------------------------------------------------------
    void Dialog::setSignPos(float x, float y)
    {
        signWorldPos.x = x;
        signWorldPos.y = y;
    }

    // -------------------------------------------------------------------------
    // Returns true while the dialog box is visible on screen.
    // -------------------------------------------------------------------------
    bool Dialog::dialogBoxShowing() const
    {
        return isShowing;
    }

} // namespace UI