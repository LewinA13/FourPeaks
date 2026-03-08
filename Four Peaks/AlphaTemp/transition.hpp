#pragma once
// ---------------------------------------------------------------------------
// transition.hpp  —  Pokemon-style screen-wipe transition effect
// ---------------------------------------------------------------------------
// HOW TO USE (optional drop-in — game runs fine without it):
//
//   1. In main.cpp add:
//        #include "transition.hpp"
//        static Transition gTransition;
//
//   2. When you want to change scenes, call:
//        gTransition.start(SceneState::WinterS1);   // or whatever target
//      instead of setting currentState directly.
//
//   3. In your main game loop update/draw section, call:
//        gTransition.update(dt);
//        gTransition.draw();      // call AFTER all game drawing
//
//   4. In the update section check:
//        if (gTransition.isReadyToSwitch()) {
//            currentState = gTransition.getTarget();
//            gTransition.notifySwitch();
//        }
//
// The transition fires even if you don't integrate it—it is purely visual.
// ---------------------------------------------------------------------------

#include <AEEngine.h>
#include <cstdint>

// ??? Transition styles ?????????????????????????????????????????????????????
enum class TransitionStyle
{
    PokemonWipe,    // expanding diamond/circle wipe (classic Pokemon battle)
    SweepLeft,      // hard black panel sweeps from right to left
    Dissolve,       // random pixel-block dissolve
};

class Transition
{
public:
    Transition() = default;
    ~Transition() = default;

    // ?? Configuration ???????????????????????????????????????????????????????
    TransitionStyle style = TransitionStyle::PokemonWipe;
    float           speed = 2.5f;   // multiplier: higher = faster wipe

    // ?? Control API ?????????????????????????????????????????????????????????

    // Call this to kick off a transition
    void start();

    // Per-frame calls
    void update(float dt);
    void draw() const;

    // Phase queries
    bool isActive()         const { return phase != Phase::Idle; }
    bool isReadyToSwitch()  const { return switchPending; }
    bool isComplete()       const { return phase == Phase::Idle && switchDone; }

    // Call once after you've actually changed the scene
    void notifySwitch() { switchPending = false; switchDone = true; }

    // Reset so the transition can be used again next time
    void reset() {
        phase = Phase::Idle; t = 0.0f;
        switchPending = false; switchDone = false;
    }

private:
    enum class Phase { Idle, WipeIn, Hold, WipeOut };
    Phase phase = Phase::Idle;
    float t = 0.0f;   // 0..1 within current phase
    bool  switchPending = false;
    bool  switchDone = false;

    //Per-style draw helpers 
    void drawPokemonWipe(float progress, bool coverScreen) const;
    void drawSweepLeft(float progress, bool coverScreen) const;
    void drawDissolve(float progress) const;

    //Shared quad draw 
    static void drawSolidRect(float x, float y, float w, float h, std::uint32_t argb);
    static void drawCircleMask(float cx, float cy, float radius, bool invert);
};