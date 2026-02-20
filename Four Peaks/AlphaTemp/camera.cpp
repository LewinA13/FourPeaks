#include "camera.hpp"
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

    void apply() {
        AEGfxSetCamPosition(camX + shakeOffsetX, camY + shakeOffsetY);
    }

    void setY(float y) {
        camY = y;
        targetCamY = y;
        transitioning = false;
        justFinished = false;
    }

    float getY() {
        return camY;
    }

    float screenHeight() {
        return screenH;
    }

    float screenWidth() {
        return screenW;
    }

    void followPlayer(float playerX, float playerY, float dt) {
        if (!followMode) return;
        if (transitioning) return; // Don't follow during transitions

        // Set target camera position (camera centered on player)
        targetCamX = playerX;
        targetCamY = playerY;

        // Example: Keep camera within stage bounds
        // targetCamX = std::max(minX, std::min(maxX, targetCamX));
        // targetCamY = std::max(minY, std::min(maxY, targetCamY));
    }

    void setFollowMode(bool enabled) {
        followMode = enabled;
        if (enabled) {
            // Initialize target to current position
            targetCamX = camX;
            targetCamY = camY;
        }
    }

    bool isFollowMode() {
        return followMode;
    }

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

    bool isTransitioning() {
        return transitioning;
    }

    float transitionProgress() {
        if (!transitioning) return 0.0f;
        return clamp01(timer / duration);
    }

    bool consumeJustFinished() {
        bool v = justFinished;
        justFinished = false;
        return v;
    }

    //screenshake
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

    void stopShake() {
        shakeOffsetX = shakeOffsetY = 0.0f;
        shakeTimeLeft = shakeDuration = 0.0f;
        shakeMagnitude = 0.0f;
        shakePhase = 0.0f;
    }

    bool isShaking() {
        return shakeTimeLeft > 0.0f;
    }


} // namespace camera
