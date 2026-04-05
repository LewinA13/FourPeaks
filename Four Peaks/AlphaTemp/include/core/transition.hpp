#pragma once
// ---------------------------------------------------------------------------
// transition.hpp  -  Simple fade-to-black screen transition
// ---------------------------------------------------------------------------
// HOW TO USE:
//   1. In main.cpp:
//        #include "transition.hpp"
//        static Transition gTransition;
//
//   2. When you want to change scenes, call:
//        gTransition.start();
//        transitionTarget = SceneState::WinterS2;
//
//   3. In the main game loop, call each frame:
//        gTransition.update(dt);
//        if (gTransition.isReadyToSwitch()) {
//            currentState = transitionTarget;
//            gTransition.notifySwitch();
//        }
//        // ...all game drawing...
//        gTransition.draw();   // LAST, on top of everything
// ---------------------------------------------------------------------------

#include "AEEngine.h"
#include <cstdint>

class Transition
{
public:
    Transition() = default;
    ~Transition() = default;

    // Speed multiplier: higher = faster fade
    float speed = 2.5f;

    // Start a new fade-to-black. Safe to call even mid-transition (restarts).
    void start();

    // Call once per frame
    void update(float dt);

    // Call AFTER all game drawing
    void draw() const;

    // True for ONE frame at the fully-black mid-point - switch scene now
    bool isReadyToSwitch() const { return switchPending; }

    // Call after switching scene in response to isReadyToSwitch()
    void notifySwitch() { switchPending = false; }

    // True while fade-in or fade-out is playing
    bool isActive() const { return phase != Phase::Idle; }

    // True after the full transition cycle has completed
    bool isComplete() const { return phase == Phase::Idle && switchDone; }

    // Reset for reuse
    void reset()
    {
        phase         = Phase::Idle;
        t             = 0.0f;
        switchPending = false;
        switchDone    = false;
    }

private:
    enum class Phase { Idle, FadeIn, Hold, FadeOut };

    Phase phase         = Phase::Idle;
    float t             = 0.0f;
    bool  switchPending = false;
    bool  switchDone    = false;

    static void drawBlackOverlay(float alpha);
};
