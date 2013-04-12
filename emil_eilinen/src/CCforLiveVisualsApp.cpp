#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CCforLiveVisualsApp : public AppBasic {
  public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
	void mouseUp( MouseEvent event );
	void mouseMove( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();

	float mMathPI;

	int mWindowWidth;
	int mWindowHeight;
	Color mBgColor;

	bool mIsPressed;
	Vec2i mMouseLoc;
	Vec2f mMouseVel;

	int mTime;

	int					mBPM;
	int					mLastBPMResetTime;
	int					mGeneralDir;

	list<Particle>		mParticles;
	float				mTmpDirAngle;
	float				mParticleSize;
	float				mParticleVelocity;

	float				mLineAngle;
};

void CCforLiveVisualsApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 800, 600 );
	settings->setFrameRate( 60.0f );
}

void CCforLiveVisualsApp::setup()
{
	mBPM = 120;
	mLastBPMResetTime = getElapsedSeconds()*1000;
	Rand::randomize();
	mBgColor = Color( 1, 0, 0 );
	mGeneralDir = 1;
	mIsPressed = false;
	mMathPI = 3.14159265358979323846;

	mLineAngle = 0;

	for( int i=0; i<100; i++ )
	{
		mParticleSize = 20;
		mParticleVelocity = 5;

		// create a particle (location, direction, velocity, size)
		mTmpDirAngle = Rand::randFloat( 360.0f );
		mParticles.push_back( Particle( Vec2i( mMouseLoc ),
										Vec2f( cos(mTmpDirAngle), sin(mTmpDirAngle) ), // direction
										mParticleVelocity, // velocity
										mParticleSize ) );
	}
}

void CCforLiveVisualsApp::mouseDown( MouseEvent event )
{
	mIsPressed = true;
}

void CCforLiveVisualsApp::mouseUp( MouseEvent event )
{
	mIsPressed = false;
}

void CCforLiveVisualsApp::mouseMove( MouseEvent event )
{
	mMouseVel = ( event.getPos() - mMouseLoc );
	mMouseLoc = event.getPos();
}

void CCforLiveVisualsApp::mouseDrag( MouseEvent event )
{
	mouseMove( event );
}

void CCforLiveVisualsApp::keyDown( KeyEvent event )
{
	if( event.getChar() == '1' ){
		mIsPressed = true;
	} else if( event.getChar() == '2' ){
		mIsPressed = false;
	}
	
	if( event.getChar() == 'f' ){
		if ( isFullScreen() ) {
			setFullScreen( false );
		} else {
			setFullScreen( true );
		}
	} else if( event.getChar() == 'g' ){
	} else if( event.getChar() == 'h' ){
	} else if( event.getChar() == 'i' ){
	}


	if( event.getCode() == KeyEvent::KEY_UP ) {
		if (mBPM < 236) { mBPM += 5;	} 
		else { mBPM = 60; }
	}
	if( event.getCode() == KeyEvent::KEY_DOWN ) {
		if (mBPM > 64) { mBPM -= 5;	} 
		else { mBPM = 240; }
	}
	if( event.getCode() == KeyEvent::KEY_LEFT ) {
		if (mBPM > 60) { mBPM -= 1;	} 
		else { mBPM = 240; }
	}
	if( event.getCode() == KeyEvent::KEY_RIGHT ) {
		if (mBPM < 240) { mBPM += 1;	} 
		else { mBPM = 60; }
	}
	if( event.getCode() == KeyEvent::KEY_SPACE ) {
		mLastBPMResetTime = getElapsedSeconds()*1000;
	}

	if( event.getCode() == KeyEvent::KEY_ESCAPE ) {
		quit();
	}
}

void CCforLiveVisualsApp::update()
{
	mTime = getElapsedSeconds()*1000 - mLastBPMResetTime;

	mBgColor = Color( mTime%1000/1000, mTime%1000/1000, mTime%1000/1000 );

	if ( ( mTime%(1000*60/mBPM) ) <= 25 ) {
		for( list<Particle>::iterator i = mParticles.begin(); i != mParticles.end(); ++i ){
			(*i).mLoc = mMouseLoc;
			mTmpDirAngle = Rand::randFloat( 360.0f );
			(*i).mDir = Vec2f( cos(mTmpDirAngle), sin(mTmpDirAngle) );

			(*i).mSize = 30;
		}
	}

	for( list<Particle>::iterator i = mParticles.begin(); i != mParticles.end(); ++i ){
			// move particle towards mouse
			float mMouseDirX = (*i).mLoc.x-mMouseLoc.x;
			float mMouseDirY = (*i).mLoc.y-mMouseLoc.y;
			Vec2f mMouseDir = Vec2f( cos(mMouseDirX), sin(mMouseDirY) );
			// (*i).mDir += (mMouseDir-(*i).mDir);

			// update location
			(*i).mLoc += mGeneralDir*10 * (*i).mDir;

			// decelerate moving particles (the more deceleration kicks in the faster they are)
			/*
			if ( (*i).mVel > mParticleVelocity ) { (*i).mVel -= 0.05f; }
			if ( (*i).mVel > mParticleVelocity*2 ) { (*i).mVel -= 0.05f; }
			if ( (*i).mVel > mParticleVelocity*3 ) { (*i).mVel -= 0.05f; }
			if ( (*i).mVel > mParticleVelocity*4 ) { (*i).mVel -= 0.05f; }
			if ( (*i).mVel > mParticleVelocity*5 ) { (*i).mVel -= 0.05f; }
			if ( (*i).mVel > mParticleVelocity*6 ) { (*i).mVel -= 0.05f; }
			*/

			(*i).mSize -= 0.5;
		}
}

void CCforLiveVisualsApp::draw()
{
	if (mIsPressed == true) {
		gl::clear( Color( 0.1, 0.1, 0.1 ) );
	}

	if (mIsPressed == false) {
		gl::clear( mBgColor );
	}

	gl::color( Color( 0, 0, 1 ) );
	glLineWidth(10);
	mLineAngle += 0.01;
	gl::drawLine( Vec2f( cos( mLineAngle )*getWindowWidth()/2+mMouseLoc.x, sin( mLineAngle )*getWindowHeight()/2+mMouseLoc.y ), mMouseLoc );
	gl::drawLine( Vec2f( cos( mLineAngle+mMathPI/2 )*getWindowWidth()/2+mMouseLoc.x, sin( mLineAngle+mMathPI/2 )*getWindowHeight()/2+mMouseLoc.y ), mMouseLoc );
	gl::drawLine( Vec2f( cos( mLineAngle+mMathPI )*getWindowWidth()/2+mMouseLoc.x, sin( mLineAngle+mMathPI )*getWindowHeight()/2+mMouseLoc.y ), mMouseLoc );
	gl::drawLine( Vec2f( cos( mLineAngle+3*mMathPI/2 )*getWindowWidth()/2+mMouseLoc.x, sin( mLineAngle+3*mMathPI/2 )*getWindowHeight()/2+mMouseLoc.y ), mMouseLoc );

	gl::color( 1, 1, 0 );
	for( list<Particle>::iterator i = mParticles.begin(); i != mParticles.end(); ++i ){
		gl::drawSolidCircle( (*i).mLoc, (*i).mSize );
	}
}

CINDER_APP_BASIC( CCforLiveVisualsApp, RendererGl )