#pragma once

#include <vector>
#include <string>
#include <map>
#include <AEEngine.h>



namespace UI{

    class Dialog
    {
    public:


        Dialog();


        // -------------------------------------------------------------------------
        // Populates levelDialogs with every line of text for all level IDs.
        // Must be called once after construction before any dialog is shown.
        // -------------------------------------------------------------------------
        void initialize();

        // -------------------------------------------------------------------------
        // Advances the dialog state each frame. Handles the typewriter character
        // reveal, input to advance lines, and auto-close timing for artifact dialogs.
        // -------------------------------------------------------------------------
        void update(float deltaTime);

        // -------------------------------------------------------------------------
        // Draws the dialog box and visible text over the HUD layer (camera-space).
        // Also draws the "Press E to read" hint when the player is near a sign
        // but the dialog box is not yet open.
        // -------------------------------------------------------------------------
        void render();

        // -------------------------------------------------------------------------
        // Queues a sign-board dialog for levelID to show on the next [E] press.
        // Silently ignored if levelID has no registered text or if the dialog is
        // already showing for that level.
        // -------------------------------------------------------------------------
        void showForLevel(int levelID);

        // -------------------------------------------------------------------------
        // Called each frame by the collision system to report whether the player
        // is within interaction range of a sign. Closing this signal will dismiss
        // any open sign dialog on the next update tick.
        // -------------------------------------------------------------------------
        void playerNearSignBoard(bool detect);

        // -------------------------------------------------------------------------
        // Immediately starts a non-interactive (auto) dialog for levelID — used
        // when the player picks up an artifact or triggers a scripted cutscene.
        // The dialog advances automatically and closes after the last line.
        // -------------------------------------------------------------------------
        void triggerAutoDialog(int levelID);

        // -------------------------------------------------------------------------
        // Sets the world-space position of the nearest sign so the "Press E" hint
        // can be drawn above it regardless of camera position.
        // -------------------------------------------------------------------------
        void setSignPos(float x, float y);

        // -------------------------------------------------------------------------
        // Returns true while a dialog box is actively drawn on screen.
        // -------------------------------------------------------------------------
        bool dialogBoxShowing() const;

        // -------------------------------------------------------------------------
        // Closes the dialog box, clears the typewriter state, and resets all flags
        // (including isAutoDialog and waitingForInput) to their default values.
        // -------------------------------------------------------------------------
        void reset();




    private:


        // -------------------------------------------------------------------------
        // Maps each level ID to its ordered list of dialog lines.
        // -------------------------------------------------------------------------
        std::map<int, std::vector<std::string>> levelDialogs;


        std::vector<std::string> texts;
        int currentIndex;

        //! check dialog is showing currently or not
        bool isShowing;

        bool playerNearSign = false;

        int currentLevelID = -1;

        // *****************************        TYPEWRITER                   **************************************************************************
        size_t displayedChars;   
        float typeWriterTimer;   
        float timePerChar;

        float wordSize;



        bool  isAutoDialog;
        float autoDialogCloseTimer;
        float autoDialogCloseDelay;

        bool waitingForInput;   
        AEVec2 signWorldPos;
    };

    extern Dialog gDialog;  

}
