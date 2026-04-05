// ----------------------------------------------------------------------------
// Done By: Skyler
// ----------------------------------------------------------------------------

#pragma once
// ---------------------------------------------------------------------------
// splashscreen.hpp
// ---------------------------------------------------------------------------
// Displays a centered splash image for SPLASH_DURATION seconds,
// then signals the game loop to switch to the Main Menu.
//
// *** Set SPLASH_WIDTH / SPLASH_HEIGHT to match your actual image size ***
//
// Return values from update():
//   0  — still showing splash
//   1  — done, switch to Main Menu
// ---------------------------------------------------------------------------

#pragma once
#include <AEEngine.h>

namespace game
{
    class SplashScreen
    {
    public:
// ---------------------------------------------------------------------------
// Splash Screen
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        SplashScreen();
// ---------------------------------------------------------------------------
// Splash Screen
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        ~SplashScreen();

// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int  update();
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void draw() const;

    private:
        static constexpr float SPLASH_DURATION = 5.0f;
        static constexpr float FADE_TIME = 0.6f;

        // Splash 1 — DigiPen logo
        static constexpr float SPLASH1_WIDTH = 1220.0f;
        static constexpr float SPLASH1_HEIGHT = 356.0f;

        // Splash 2 - Game Logo
        static constexpr float SPLASH2_WIDTH = 500.0f;
        static constexpr float SPLASH2_HEIGHT = 500.0f;

        float timer;
        int   currentSplash; // 0 = first, 1 = second

        AEGfxTexture* splashTex1;
        AEGfxTexture* splashTex2;
        AEGfxVertexList* splashMesh;
    };

} // namespace game
