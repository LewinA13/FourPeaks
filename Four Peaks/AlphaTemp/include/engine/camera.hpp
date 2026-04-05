// ----------------------------------------------------------------------------
// Done By: Arun
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
// in whole or in part, without prior written permission is strictly prohibited.
//
//---------------------------------------------------------------------------

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "AEEngine.h"

namespace camera {

	void init();
	void update(float dt);
	void apply();

	// Position control
	void setY(float y);
	float getY();

	// Camera follow player
	void followPlayer(float playerX, float playerY, float dt);
	void setFollowMode(bool enabled);
	bool isFollowMode();

	// Screen info
	float screenHeight();
	float screenWidth();

	// Transitions
	bool startTransitionY(float newFromY, float newToY, float durationSec);
	bool isTransitioning();
	float transitionProgress();
	bool consumeJustFinished();

	// screen shake magnitude
	void startShake(float magnitude, float durationSec, float frequencyHz = 35.0f);
	void stopShake();
	bool isShaking();

} // namespace camera

#endif // CAMERA_HPP
