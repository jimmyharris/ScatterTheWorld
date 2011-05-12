#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Perlin.h"
#include "cinder/Utilities.h"

#include "KinectCursor.h"
#include "ParticleController.h"

#include <sstream>
using std::stringstream;

#define WIDTH 640
#define HEIGHT 480

#define CENTER_X 320
#define CENTER_Y 240

#define RESOLUTION 10
#define NUM_PARTICLES_TO_SPAWN 15

using namespace ci;
using namespace ci::app;
using namespace std;

class ScatterTheWorldApp : public AppBasic, public KinectCursor::KinectListener {
  public:

    // Cinder CallBacks.
    void prepareSettings(Settings* settings);
    void setup();
    void update();
    void draw();
    void shutdown();
    void keyDown(KeyEvent event);

    // Graphic Variables
    gl::Texture colorTexture;
    Vec2f handCoords;
    Vec2f handVel;
    Perlin mPerlin;

    bool mDrawParticles;
    bool mDrawImage;
    bool mCentralGravity,mAllowPerlin;

    // Kinect Callbacks
    KinectCursor *mKinectController;
    bool mInSession,mPushed;

    void OnPush();
    void OnPointUpdate(Vec3f HandPosition);
    void OnFocusStartDetected();
    void OnSessionStart();
    void OnSessionEnd();

    //Particle System
    ParticleController mParticleController;

};


void ScatterTheWorldApp::prepareSettings(Settings* settings)
{
  settings->setWindowSize(WIDTH, HEIGHT);
  settings->setFrameRate(60.0f);
}

void ScatterTheWorldApp::setup()
{

  handCoords = Vec2f(CENTER_X,CENTER_Y);
  handVel = Vec2f::zero();
  mInSession = false;
  //Setup the kinect. (This takes a while I wish I could do this in a different thread)
  mKinectController = new KinectCursor((KinectListener *)this);
  // Initialize our Video texture.
  gl::Texture::Format format;
  colorTexture = gl::Texture( KINECT_WIDTH, KINECT_HEIGHT, format );

  mDrawParticles  = true;
  mDrawImage      = false;
  mPushed         = false;
  mCentralGravity = false;
  mAllowPerlin    = false;

}


void ScatterTheWorldApp::update()
{
  mKinectController->update();
  if (mDrawImage) {
    colorTexture.update(mKinectController->getImageChannel8u(),Area(0,0,640,480));
  }

	if( mPushed )
		mParticleController.addParticles( NUM_PARTICLES_TO_SPAWN, handCoords, handVel );
  
  mParticleController.repulseParticles();
  
  if( mCentralGravity )
    mParticleController.pullToCenter();
  
  if( mAllowPerlin )
    mParticleController.applyPerlin( mPerlin );
  
  mParticleController.update(mKinectController->getImageChannel32f(),handCoords);

}

void ScatterTheWorldApp::draw()
{
  // clear out the window with black
  gl::clear( Color( 0, 0, 0 ) );
  if (mDrawImage) {
    gl::color( Color(1,1,1) );
    gl::draw(colorTexture,getWindowBounds()); 
  }
  if (mDrawParticles) {
    gl::color( Color(1,1,1) );
    mParticleController.draw();
  }
  if (mInSession) {
    gl::color( Color(1,0,0) );
    gl::drawSolidCircle(handCoords, 5.0f, 20); 
  }
  gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 32.0f));
  gl::drawString( toString(mParticleController.mParticles.size()) + " Particles", Vec2f(32.0f, 52.0f));

}

void ScatterTheWorldApp::keyDown( KeyEvent event )
{
  if( event.getChar() == '1' ){
    mDrawImage = ! mDrawImage;
  } else if( event.getChar() == '2' ){
    mDrawParticles = ! mDrawParticles;
  }

  if( event.getChar() == 'g' ){
    mCentralGravity = ! mCentralGravity;
  } else if( event.getChar() == 'p' ){
    mAllowPerlin = ! mAllowPerlin;
  }
}

void ScatterTheWorldApp::shutdown()
{
  delete mKinectController;
}


void ScatterTheWorldApp::OnPush()
{
  mPushed = !mPushed;
  printf("I pushed!\n");
}
void ScatterTheWorldApp::OnPointUpdate(Vec3f HandPosition)
{
  Vec2f newPosition = Vec2f(HandPosition.x, HandPosition.y);
  handVel = newPosition - handCoords;
  handCoords.x = HandPosition.x;
  handCoords.y = HandPosition.y;
}
void ScatterTheWorldApp::OnFocusStartDetected()
{
  printf("I think I can see you!\n");
}
void ScatterTheWorldApp::OnSessionStart()
{
  printf("Hello!\n");
  mInSession = true;
}
void ScatterTheWorldApp::OnSessionEnd()
{
  handCoords = Vec2f(CENTER_X,CENTER_Y);
  printf("GoodBye!\n");
  mInSession = false;
}




CINDER_APP_BASIC( ScatterTheWorldApp, RendererGl )
