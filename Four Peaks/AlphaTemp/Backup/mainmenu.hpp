// ---------------------------------------------------------------------------
// mainmenu.hpp
// ---------------------------------------------------------------------------
//
// Simple main menu for the seasonal platformer.
// Shows three text "buttons":
//   - Play
//   - How To Play (placeholder)
//   - Exit
//
// update() return codes:
//   0 = stay on main menu
//   1 = start winter
//   2 = exit game
//

#ifndef MAINMENU_HPP
#define MAINMENU_HPP

namespace game
{

    void SaveVolumeSettings();
    void LoadVolumeSettings();

    class MainMenu
    {
    public:
        MainMenu();

        int update();   // handle input
        void draw() const; // draw menu each frame

    private:
        int  selectedIndex; // 0=Play, 1=How To Play, 2=Exit
        bool showHowTo;     // true when help screen is shown
        bool showSettings;
		int  settingsRow;    //For Background and SFX Selection
        bool confirmDeleteSave;
        bool deleteSaveResult;

        void drawHowToPlay() const;
        void drawSettings() const;
    };
}

#endif // MAINMENU_HPP
