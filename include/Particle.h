#pragma once
#include "cinder/Channel.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Perlin.h"
#include <vector>

class Particle {
 public:
	Particle();
	Particle( cinder::Vec2f, cinder::Vec2f );
	void pullToCenter(cinder::Vec2i center);
	void applyPerlin( const cinder::Perlin &perlin );
	void update( const cinder::Channel32f &channel, const cinder::Vec2i &mouseLoc );
	void draw();
	
	cinder::Vec2f	mLoc;
	cinder::Vec2f	mVel;
	cinder::Vec2f	mAcc;
	
	float		mDecay;
	float		mRadius, mRadiusDest;
	float		mScale;
	
	float		mMass;

	int			mAge;
	int			mLifespan;
	bool		mIsDead;
};
