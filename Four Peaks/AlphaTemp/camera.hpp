#pragma once

namespace camera
{
    // Call once at startup (after window init), and any time window size changes.
    void init();

    // Call every frame (dt in seconds)
    void update(float dt);

    // Apply camera to Alpha Engine (call once per frame, after update)
    void apply();

    // Instantly set camera Y (no animation)
    void setY(float y);

    // Get camera Y
    float getY();

    // Screen height in world units (WinMaxY - WinMinY)
    float screenHeight();

    // Start a smooth vertical transition (Y only)
    // Returns true if it started, false if already transitioning.
    bool startTransitionY(float fromY, float toY, float durationSec);

    // Is camera currently transitioning?
    bool isTransitioning();

    // 0..1 progress of the current transition (0 if not transitioning)
    float transitionProgress();

    // Has a transition just finished this frame?
    // (useful to trigger a state switch once)
    bool consumeJustFinished();
}
