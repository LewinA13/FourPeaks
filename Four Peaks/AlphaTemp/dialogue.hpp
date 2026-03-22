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

        void initialize();
        void update(float deltaTime);
        void render();

        //! for specific level
        void showForLevel(int levelID);

        //! collision
        void playerNearSignBoard(bool detect);

        //! shown up after picking up artifacts
        void triggerFromArtifact(int levelID);

        void setSignPos(float x, float y);

        void reset();


    private:


        //! store each level dialog
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

        bool  isArtifactDialog;
        float artifactAutoCloseTimer;
        float artifactAutoCloseDelay;

        bool waitingForInput;   
        AEVec2 signWorldPos;


    };

    extern Dialog gDialog;  

}
