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

} // namespace camera

#endif // CAMERA_HPP
