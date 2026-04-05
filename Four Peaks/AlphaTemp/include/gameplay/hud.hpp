// ----------------------------------------------------------------------------
// Done By: Justin
// ----------------------------------------------------------------------------

#pragma once

namespace hud
{
    // draw melon and the counter
// ---------------------------------------------------------------------------
// Draw Melon Counter
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void drawMelonCounter(int melonCount);

    // draw death counter on screen
// ---------------------------------------------------------------------------
// Draw Death Counter
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void drawDeathCounter(int deathCount);

    // draw formatted run timer on screen
// ---------------------------------------------------------------------------
// Draw Run Timer
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void drawRunTimer(float totalSeconds);

    // draw artifact icons on the hud
// ---------------------------------------------------------------------------
// Draw Artifacts Hud
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void drawArtifactsHud(const bool collected[4]);
}
