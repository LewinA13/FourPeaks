#pragma once

namespace hud
{
    // draw melon and the counter
    void drawMelonCounter(int melonCount);

    // draw death counter on screen
    void drawDeathCounter(int deathCount);

    // draw formatted run timer on screen
    void drawRunTimer(float totalSeconds);

    // draw artifact icons on the hud
    void drawArtifactsHud(const bool collected[4]);
}