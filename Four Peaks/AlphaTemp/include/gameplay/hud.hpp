// ----------------------------------------------------------------------------
// Done By: Justin
// ----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Copyright (C) 2026 Team Game++ (Four Peaks)
// All rights reserved.
//
// This file is part of the Four Peaks project. All code, design, and original
// assets are the work of LewinA and team members unless otherwise stated.
//
// Audio assets are sourced from Soundly and used under appropriate licensing.
//
// Reproduction, distribution, or modification of this file or its contents,
// in wwwwwhole or in part, without prior written permission is strictly prohibited.
//
//---------------------------------------------------------------------------

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
