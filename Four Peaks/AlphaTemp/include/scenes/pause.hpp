// ----------------------------------------------------------------------------
// Done By: Justin
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

#ifndef PAUSE_HPP
#define PAUSE_HPP

// enum for pause menu actions, returned by update() to indicate what the main game loop should do next
namespace pause
{
    enum Action
    {
        None = 0,
        Resume,
        MainMenu,
        ExitGame
    };

// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    Action update();
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void draw();
}

#endif // PAUSE_HPP
