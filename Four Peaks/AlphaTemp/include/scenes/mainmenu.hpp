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

// ---------------------------------------------------------------------------
// Save Volume Settings
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void SaveVolumeSettings();
// ---------------------------------------------------------------------------
// Load Volume Settings
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void LoadVolumeSettings();

    class MainMenu
    {
    public:
// ---------------------------------------------------------------------------
// Main Menu
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        MainMenu();

// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int update();   // handle input
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void draw() const; // draw menu each frame

    private:
        int  selectedIndex; // 0=Play, 1=How To Play, 2=Exit
        bool showHowTo;     // true when help screen is shown
        bool showSettings;
		int  settingsRow;    //For Background and SFX Selection
        bool confirmDeleteSave;
        bool deleteSaveResult;

// ---------------------------------------------------------------------------
// Draw How To Play
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawHowToPlay() const;
// ---------------------------------------------------------------------------
// Draw Settings
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawSettings() const;
    };
}

#endif // MAINMENU_HPP
