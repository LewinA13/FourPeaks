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

        //! collision
        void PLAYERNEARSIGN(bool detect);

        void reset();


        void next();
        bool isActive() const { return isShowing; }


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


    };

    extern Dialog gDialog;  

}
