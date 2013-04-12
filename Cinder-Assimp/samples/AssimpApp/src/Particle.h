#pragma once
#include "cinder/Vector.h"
#include <vector>

class Particle {
public:
	Particle();
	Particle( ci::Vec2f loc, ci::Vec2f dir, float vel, float size );

	ci::Vec2f	mLoc;
	ci::Vec2f	mDir;
	float		mVel;
	float		mSize;
};