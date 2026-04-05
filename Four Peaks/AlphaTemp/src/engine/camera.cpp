#include "engine/camera.hpp"
#include <algorithm>
#include <cmath>

namespace camera {

    namespace {
        float camX = 0.0f;
        float camY = 0.0f;
        float fromY = 0.0f;
        float toY = 0.0f;
        float timer = 0.0f;
        float duration = 0.5f;
        bool transitioning = false;
        bool justFinished = false;
        bool followMode = false;
        float screenH = 900.0f;
        float screenW = 1600.0f;

        // Camera follow parameters
        float followSmoothness = 5.0f; // Higher = slower follow
        float targetCamX = 0.0f;
        float targetCamY = 0.0f;

        //screenshake
        float shakeOffsetX = 0.0f;
        float shakeOffsetY = 0.0f;

        float shakeTimeLeft = 0.0f;
        float shakeDuration = 0.0f;
        float shakeMagnitude = 0.0f;
        float shakeFrequencyHz = 35.0f;

        float shakePhase = 0.0f; // radians

        float clamp01(float t) {
            if (t < 0.0f) return 0.0f;
            if (t > 1.0f) return 1.0f;
            return t;
        }

        float smoothStep(float t) {
            return t * t * (3.0f - 2.0f * t);
        }

        float lerp(float a, float b, float t) {
            return a + (b - a) * t;
        }
    }

   // -------------------------------------------------------------------------
   // Public API
   // -------------------------------------------------------------------------

    void init() {
        screenH = AEGfxGetWinMaxY() - AEGfxGetWinMinY();
        screenW = AEGfxGetWinMaxX() - AEGfxGetWinMinX();
        camX = 0.0f;
        camY = 0.0f;
        transitioning = false;
        justFinished = false;
        followMode = false;

        shakeOffsetX = shakeOffsetY = 0.0f;
        shakeTimeLeft = shakeDuration = 0.0f;
        shakeMagnitude = 0.0f;
        shakeFrequencyHz = 35.0f;
        shakePhase = 0.0f;
    }

    void update(float dt) {
        justFinished = false;
        // -----------------------------
        // Screen shake update (predictable sin/cos)
        // -----------------------------
        if (shakeTimeLeft > 0.0f) {
            shakeTimeLeft -= dt;
            if (shakeTimeLeft < 0.0f) shakeTimeLeft = 0.0f;

            // decay from 1 -> 0 over lifetime
            float t = (shakeDuration > 0.0f) ? (shakeTimeLeft / shakeDuration) : 0.0f;
            t = clamp01(t);

            const float twoPi = 6.283185307f;
            shakePhase += twoPi * shakeFrequencyHz * dt;

            float mag = shakeMagnitude * t;

            shakeOffsetX = std::cos(shakePhase) * mag;
            shakeOffsetY = std::sin(shakePhase * 1.3f) * mag; //
        }
        else {
            shakeOffsetX = 0.0f;
            shakeOffsetY = 0.0f;
        }

        // Handle transitions (takes priority over follow mode)
        if (transitioning) {
            timer += dt;
            float t = clamp01(timer / duration);
            float s = smoothStep(t);
            camY = lerp(fromY, toY, s);

            if (t >= 1.0f) {
                transitioning = false;
                camY = toY;
                justFinished = true;
            }
            return;
        }

        // Smooth camera follow when in follow mode
        if (followMode) {
            // Smooth towards target position
            camX = lerp(camX, targetCamX, followSmoothness * dt);
            camY = lerp(camY, targetCamY, followSmoothness * dt);
        }
    }

    // -------------------------------------------------------------------------
    // Writes the final camera position (base + shake offset) to the renderer.
    // -------------------------------------------------------------------------

    void apply() {
        AEGfxSetCamPosition(camX + shakeOffsetX, camY + shakeOffsetY);
    }

    // -------------------------------------------------------------------------
    // Teleports the camera to the given vertical world-space coordinate and
    // cancels any active transition.
    // -------------------------------------------------------------------------
    void setY(float y) {
        camY = y;
        targetCamY = y;
        transitioning = false;
        justFinished = false;
    }

    // -------------------------------------------------------------------------
    // Returns the camera's current vertical world-space position.
    // -------------------------------------------------------------------------
    float getY() {
        return camY;
    }

    // -------------------------------------------------------------------------
    // Returns the cached screen height in pixels.
    // -------------------------------------------------------------------------
    float screenHeight() {
        return screenH;
    }

    // -------------------------------------------------------------------------
    // Returns the cached screen width in pixels.
    // -------------------------------------------------------------------------
    float screenWidth() {
        return screenW;
    }

    // -------------------------------------------------------------------------
    // Stores the new follow target. update() will smoothly move the camera
    // toward this position on subsequent frames while follow mode is active.
    // -------------------------------------------------------------------------
    void followPlayer(float playerX, float playerY, float dt) {
        (void)dt;
        if (!followMode) return;
        if (transitioning) return; // Don't follow during transitions

        // Set target camera position (camera centered on player)
        targetCamX = playerX;
        targetCamY = playerY;

        // Example: Keep camera within stage bounds
        // targetCamX = std::max(minX, std::min(maxX, targetCamX));
        // targetCamY = std::max(minY, std::min(maxY, targetCamY));
    }

    // -------------------------------------------------------------------------
    // Toggles smooth-follow mode. Enabling it initialises the internal target
    // to the current camera position so there is no jump on the first frame.
    // -------------------------------------------------------------------------
    void setFollowMode(bool enabled) {
        followMode = enabled;
        if (enabled) {
            // Initialize target to current position
            targetCamX = camX;
            targetCamY = camY;
        }
    }

    // -------------------------------------------------------------------------
    // Returns true if smooth-follow mode is currently enabled.
    // -------------------------------------------------------------------------
    bool isFollowMode() {
        return followMode;
    }

    // -------------------------------------------------------------------------
    // Starts a smooth vertical scroll from newFromY to newToY over durationSec.
    // Disables follow mode for the duration.
    // Returns false without doing anything if a transition is already running.
    // -------------------------------------------------------------------------
    bool startTransitionY(float newFromY, float newToY, float durationSec) {
        if (transitioning)
            return false;

        transitioning = true;
        fromY = newFromY;
        toY = newToY;
        timer = 0.0f;
        duration = (durationSec <= 0.0f) ? 0.001f : durationSec;
        camY = fromY;
        justFinished = false;

        // Disable follow mode during transition
        followMode = false;

        return true;
    }

    // -------------------------------------------------------------------------
    // Returns true while a vertical scroll transition is in progress.
    // -------------------------------------------------------------------------
    bool isTransitioning() {
        return transitioning;
    }

    // -------------------------------------------------------------------------
    // Returns normalised progress of the active transition in [0, 1].
    // Returns 0 if no transition is running.
    // -------------------------------------------------------------------------
    float transitionProgress() {
        if (!transitioning) return 0.0f;
        return clamp01(timer / duration);
    }

    // -------------------------------------------------------------------------
    // One-shot query: returns true on the first call after a transition ends,
    // then resets so subsequent calls return false.
    // -------------------------------------------------------------------------
    bool consumeJustFinished() {
        bool v = justFinished;
        justFinished = false;
        return v;
    }

    // -------------------------------------------------------------------------
    // Begins a new screen shake. The shake fades linearly from full magnitude
    // to zero over durationSec seconds, oscillating at frequencyHz.
    // -------------------------------------------------------------------------
    void startShake(float magnitude, float durationSec, float frequencyHz) {
        if (magnitude <= 0.0f || durationSec <= 0.0f) {
            stopShake();
            return;
        }
        shakeMagnitude = magnitude;
        shakeDuration = durationSec;
        shakeTimeLeft = durationSec;
        shakeFrequencyHz = (frequencyHz <= 0.0f) ? 35.0f : frequencyHz;
        shakePhase = 0.0f;
    }

    // -------------------------------------------------------------------------
    // Immediately zeroes all shake state and offsets.
    // -------------------------------------------------------------------------
    void stopShake() {
        shakeOffsetX = shakeOffsetY = 0.0f;
        shakeTimeLeft = shakeDuration = 0.0f;
        shakeMagnitude = 0.0f;
        shakePhase = 0.0f;
    }

    // -------------------------------------------------------------------------
    // Returns true while a screen shake effect has remaining time.
    // -------------------------------------------------------------------------
    bool isShaking() {
        return shakeTimeLeft > 0.0f;
    }


} // namespace camera
