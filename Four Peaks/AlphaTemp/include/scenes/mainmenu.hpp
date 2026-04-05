// ----------------------------------------------------------------------------
// Done By: Skyler, Arun
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

        ~MainMenu();

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
