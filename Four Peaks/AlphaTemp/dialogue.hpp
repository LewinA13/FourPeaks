#pragma once

#include <vector>
#include <string>
#include <map>




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
        void next();
        bool isActive() const { return isShowing; }
        void reset();

        void PLAYERNEARSIGN(bool detect);

    private:


        //! store each level dialog
        std::map<int, std::vector<std::string>> levelDialogs;


        std::vector<std::string> texts;
        int currentIndex;

        //! check dialog is showing currently or not
        bool isShowing;

        bool playerNearSign = false;

        int currentLevelID = -1;


        //! check which level have been shown up

        float toNormalizedX(float pixelX);
        float toNormalizedY(float pixelY);


        // *****************************        TYPEWRITER                   **************************************************************************
        size_t displayedChars;   
        float typeWriterTimer;   
        float timePerChar;

        float wordSize;


    };

    extern Dialog gDialog;  

}
