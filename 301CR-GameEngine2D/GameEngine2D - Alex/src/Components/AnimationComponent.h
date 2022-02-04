#ifndef ANIMATIONCOMPONENT_H
#define ANIMATIONCOMPONENT_H

#include <SDL.h>

struct AnimationComponent {

	int numFrames;
	int currentFrames;
	int frameSpeedRate;
	bool isLoop;
	int startTime;

	AnimationComponent(int numFrames = 1, int frameSpeedRate = 1, bool isLoop = true) {

		this->numFrames = numFrames;
		this->currentFrames = 1;
		this->frameSpeedRate = frameSpeedRate;
		this->isLoop = isLoop;
		this->startTime = SDL_GetTicks();
	}
};

#endif // !ANIMATIONCOMPONENT_H