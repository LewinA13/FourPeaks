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
    class ThankYouScreen
    {
    public:
        ThankYouScreen();
        ~ThankYouScreen();

        int  update(float dt);
        void draw() const;

    private:
        static constexpr float DISPLAY_DURATION = 8.0f;  // auto-advance after 8 s
        static constexpr float FADE_TIME = 1.0f;  // fade in / fade out

        float timer{ 0.0f };

        // Optional background texture — place Assets/thankyou.png in your project.
        // If the file is missing the screen still works (solid black + text).
        AEGfxTexture* bgTex{};
        AEGfxVertexList* bgMesh{};

        // Font id (extern declared in main / text)
        // We just call AEGfxPrint directly.
    };

} // namespace game