// ---------------------------------------------------------------------------
// splashscreen.cpp
// ---------------------------------------------------------------------------
// Shows Assets/splash.png centered at its actual size for 5 seconds,
// with a smooth fade in and fade out, then returns 1 to go to MainMenu.
//
// Skippable: pressing ENTER, SPACE, or ESCAPE jumps straight to the menu.
// ---------------------------------------------------------------------------

#include "splash.hpp"
#include "AEEngine.h"
#include "AEFrameRateController.h"

namespace game
{

    // -------------------------------------------------------------------------
    // Constructor — load texture and build a standard 1x1 unit quad.
    // The actual size is applied in draw() via a scale transform.
    // -------------------------------------------------------------------------
    SplashScreen::SplashScreen()
        : timer(0.0f)
        , splashTex(nullptr)
        , splashMesh(nullptr)
    {
        // Load your splash image — place it in Assets/ and update the path if needed
        splashTex = AEGfxTextureLoad("Assets/Splash/DigiPen_Singapore_WEB_RED.png");

        // Standard 1x1 unit quad centered at the origin.
        // The transform in draw() will scale it to SPLASH_WIDTH x SPLASH_HEIGHT pixels.
        AEGfxMeshStart();

        AEGfxTriAdd(
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,   // top-left
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,   // top-right
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);  // bottom-left

        AEGfxTriAdd(
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,   // top-right
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,   // bottom-right
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);  // bottom-left

        splashMesh = AEGfxMeshEnd();
    }

    // -------------------------------------------------------------------------
    // Destructor — free GPU resources
    // -------------------------------------------------------------------------
    SplashScreen::~SplashScreen()
    {
        if (splashMesh) { AEGfxMeshFree(splashMesh);       splashMesh = nullptr; }
        if (splashTex) { AEGfxTextureUnload(splashTex);   splashTex = nullptr; }
    }

    // -------------------------------------------------------------------------
    // Update — returns 1 when it's time to move to Main Menu
    // -------------------------------------------------------------------------
    int SplashScreen::update()
    {
        float dt = static_cast<float>(AEFrameRateControllerGetFrameTime());
        timer += dt;

        // Allow the player to skip the splash early
        if (AEInputCheckTriggered(AEVK_RETURN) ||
            AEInputCheckTriggered(AEVK_SPACE) ||
            AEInputCheckTriggered(AEVK_ESCAPE))
        {
            return 1;
        }

        if (timer >= SPLASH_DURATION)
            return 1;

        return 0;
    }

    // -------------------------------------------------------------------------
    // Draw — renders the splash image centered at its actual pixel size
    // -------------------------------------------------------------------------
    void SplashScreen::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        if (!splashMesh || !splashTex)
            return;

        // --- Compute fade alpha ----------------------------------------------
        float alpha = 1.0f;
        if (timer < FADE_TIME)
            alpha = timer / FADE_TIME;
        else if (timer > SPLASH_DURATION - FADE_TIME)
            alpha = (SPLASH_DURATION - timer) / FADE_TIME;
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        // --- Build a scale transform so the unit mesh becomes the right size -
        // Alpha Engine world space == pixels, so scaling by pixel dimensions
        // gives us the image at its true size, centered at the origin (0, 0).
        AEMtx33 transform;
        AEMtx33Scale(&transform, SPLASH_WIDTH, SPLASH_HEIGHT);

        // --- Draw ------------------------------------------------------------
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetTransparency(alpha);

        AEGfxTextureSet(splashTex, 0.0f, 0.0f);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(splashMesh, AE_GFX_MDM_TRIANGLES);
    }

} // namespace game