#include "Particle.h"

Particle::Particle()
{
}

Particle::Particle( ci::Vec2f loc, ci::Vec2f dir, float vel, float size )
{
	mLoc = loc;
	mDir = dir;
	mVel = vel;
	mSize = size;
}