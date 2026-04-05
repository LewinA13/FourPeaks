// ---------------------------------------------------------------------------
// splashscreen.cpp
// ---------------------------------------------------------------------------
// Shows Assets/splash.png centered at its actual size for 5 seconds,
// with a smooth fade in and fade out, then returns 1 to go to MainMenu.
//
// Skippable: pressing ENTER, SPACE, or ESCAPE jumps straight to the menu.
// ---------------------------------------------------------------------------

#include "scenes/splash.hpp"
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
        , currentSplash(0)
        , splashTex1(nullptr)
        , splashTex2(nullptr)
        , splashMesh(nullptr)
    {
        splashTex1 = AEGfxTextureLoad("Assets/Splash/DigiPen_Singapore_WEB_RED.png");
        splashTex2 = AEGfxTextureLoad("Assets/FourPeaksLogo.png"); // your game logo

        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        AEGfxTriAdd(0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
        splashMesh = AEGfxMeshEnd();
    }

    // -------------------------------------------------------------------------
    // Destructor — free GPU resources
    // -------------------------------------------------------------------------
    SplashScreen::~SplashScreen()
    {
        if (splashMesh) { AEGfxMeshFree(splashMesh);        splashMesh = nullptr; }
        if (splashTex1) { AEGfxTextureUnload(splashTex1);   splashTex1 = nullptr; }
        if (splashTex2) { AEGfxTextureUnload(splashTex2);   splashTex2 = nullptr; }
    }

    // -------------------------------------------------------------------------
    // Update — returns 1 when it's time to move to Main Menu
    // -------------------------------------------------------------------------
    int SplashScreen::update()
    {
        float dt = static_cast<float>(AEFrameRateControllerGetFrameTime());
        timer += dt;

        bool skip = AEInputCheckTriggered(AEVK_RETURN) ||
            AEInputCheckTriggered(AEVK_SPACE) ||
            AEInputCheckTriggered(AEVK_ESCAPE);

        if (timer >= SPLASH_DURATION || skip)
        {
            if (currentSplash == 0)
            {
                currentSplash = 1;  // advance to game logo
                timer = 0.0f;       // reset timer for second splash
                return 0;
            }
            return 1; // both splashes done, go to menu
        }

        return 0;
    }

    // -------------------------------------------------------------------------
    // Draw — renders the splash image centered at its actual pixel size
    // -------------------------------------------------------------------------
    void SplashScreen::draw() const
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
        if (!splashMesh) return;

        // Pick active texture and dimensions
        AEGfxTexture* activeTex = (currentSplash == 0) ? splashTex1 : splashTex2;
        float         activeWidth = (currentSplash == 0) ? SPLASH1_WIDTH : SPLASH2_WIDTH;
        float         activeHeight = (currentSplash == 0) ? SPLASH1_HEIGHT : SPLASH2_HEIGHT;

        if (!activeTex) return;

        float alpha = 1.0f;
        if (timer < FADE_TIME)
            alpha = timer / FADE_TIME;
        else if (timer > SPLASH_DURATION - FADE_TIME)
            alpha = (SPLASH_DURATION - timer) / FADE_TIME;
        alpha = (alpha < 0.0f) ? 0.0f : (alpha > 1.0f) ? 1.0f : alpha;

        AEMtx33 transform;
        AEMtx33Scale(&transform, activeWidth, activeHeight);

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetTransparency(alpha);
        AEGfxTextureSet(activeTex, 0.0f, 0.0f);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(splashMesh, AE_GFX_MDM_TRIANGLES);
    }

} // namespace game
