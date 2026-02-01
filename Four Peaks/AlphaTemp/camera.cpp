#include "camera.hpp"
#include <AEEngine.h>

namespace camera
{
    namespace
    {
        float camY = 0.0f;

        float fromY = 0.0f;
        float toY = 0.0f;
        float timer = 0.0f;
        float duration = 0.5f;

        bool transitioning = false;
        bool justFinished = false;

        float screenH = 900.0f; // will be updated in init()

        float clamp01(float t)
        {
            if (t < 0.0f) return 0.0f;
            if (t > 1.0f) return 1.0f;
            return t;
        }

        float smoothStep(float t)
        {
            // 0..1 smooth ease
            return t * t * (3.0f - 2.0f * t);
        }

        float lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }
    }

    void init()
    {
        screenH = AEGfxGetWinMaxY() - AEGfxGetWinMinY();
        camY = 0.0f;
        transitioning = false;
        justFinished = false;
    }

    void update(float dt)
    {
        justFinished = false;

        if (!transitioning)
            return;

        timer += dt;
        float t = clamp01(timer / duration);
        float s = smoothStep(t);

        camY = lerp(fromY, toY, s);

        if (t >= 1.0f)
        {
            transitioning = false;
            camY = toY;
            justFinished = true;
        }
    }

    void apply()
    {
        AEGfxSetCamPosition(0.0f, camY);
    }

    void setY(float y)
    {
        camY = y;
        transitioning = false;
        justFinished = false;
    }

    float getY()
    {
        return camY;
    }

    float screenHeight()
    {
        return screenH;
    }

    bool startTransitionY(float newFromY, float newToY, float durationSec)
    {
        if (transitioning)
            return false;

        transitioning = true;
        fromY = newFromY;
        toY = newToY;
        timer = 0.0f;
        duration = (durationSec <= 0.0f) ? 0.001f : durationSec;

        camY = fromY;
        justFinished = false;
        return true;
    }

    bool isTransitioning()
    {
        return transitioning;
    }

    float transitionProgress()
    {
        if (!transitioning) return 0.0f;
        return clamp01(timer / duration);
    }

    bool consumeJustFinished()
    {
        bool v = justFinished;
        justFinished = false;
        return v;
    }
}
