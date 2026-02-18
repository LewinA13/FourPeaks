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

    private:


        //! store each level dialog
        std::map<int, std::vector<std::string>> levelDialogs;


        std::vector<std::string> texts;
        int currentIndex;

        //! check dialog is showing currently or not
        bool isShowing;

        //! check which level have been shown up
        bool hasShown[10];

        float toNormalizedX(float pixelX);
        float toNormalizedY(float pixelY);

        // *****************************        TYPEWRITER                   **************************************************************************
        size_t displayedChars;   
        float typeWriterTimer;   
        float timePerChar;

        float wordSize;


    };
}
