// ----------------------------------------------------------------------------
// Done By: Arun
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

#pragma once
// ---------------------------------------------------------------------------
// thankyou.hpp
// ---------------------------------------------------------------------------
// "Thank You for Playing" end screen.
// Shown after the player completes Autumn Stage 4 (the final level).
//
// update() return values:
//   0  — still showing
//   1  — go to MainMenu  (player pressed ENTER / SPACE / ESCAPE, or timer expires)
// ---------------------------------------------------------------------------

#include <AEEngine.h>

namespace game
{
    // ===================================================================
    // ThankYouScreen
    // Displays the "Thank You for Playing" end screen with background,
    // stars animation, and final time.
    // ===================================================================
    class ThankYouScreen
    {
    public:
        // ------------------------------------------------------------------
        // Constructor
        // Initializes background mesh/texture and stars animation
        // ------------------------------------------------------------------
        ThankYouScreen();

        // ------------------------------------------------------------------
        // Destructor
        // Frees background mesh/texture and resets star initialization
        // ------------------------------------------------------------------
        ~ThankYouScreen();

        // ------------------------------------------------------------------
        // update
        // Advances timer and star animation; returns 1 if player wants to
        // exit or auto-advance time is reached
        // ------------------------------------------------------------------
        int update(float dt);

        // ------------------------------------------------------------------
        // draw
        // Renders background, overlay, stars, and all text elements
        // ------------------------------------------------------------------
        void draw() const;

    private:
        // ------------------------------------------------------------------
        // Constants
        // ------------------------------------------------------------------
        static constexpr float DISPLAY_DURATION = 8.0f;  // auto-advance after 8 s
        static constexpr float FADE_TIME = 1.0f;  // fade in / fade out duration

        // ------------------------------------------------------------------
        // Member variables
        // ------------------------------------------------------------------
        float timer{ 0.0f };               // elapsed time since screen started

        AEGfxTexture* bgTex{};         // background texture
        AEGfxVertexList* bgMesh{};        // mesh for textured quad
    };

} // namespace game