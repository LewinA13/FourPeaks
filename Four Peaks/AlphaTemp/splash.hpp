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

#include <AEEngine.h> // AEGfxTexture

namespace game
{
    class SplashScreen
    {
    public:
        SplashScreen();
        ~SplashScreen();

        int  update();   // returns 1 when the 5 s timer expires
        void draw() const;

    private:
        static constexpr float SPLASH_DURATION = 5.0f;   // seconds
        static constexpr float FADE_TIME = 0.6f;   // seconds for fade in/out

        // *** Change these to match your splash image's actual pixel dimensions ***
        // DigiPen_Singapore_WEB_RED.png is 1525x445 px, scaled to 80% to give breathing room
        static constexpr float SPLASH_WIDTH = 1220.0f;  // 1525 * 0.8
        static constexpr float SPLASH_HEIGHT = 356.0f;  //  445 * 0.8

        float timer;

        AEGfxTexture* splashTex;
        AEGfxVertexList* splashMesh;
    };

} // namespace game