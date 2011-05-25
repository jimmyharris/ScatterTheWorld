#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Perlin.h"
#include "cinder/qtime/MovieWriter.h"
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

using namespace cinder;
using namespace cinder::app;
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
    void startRecording(ImageSourceRef testFrame);
    void stopRecording();

    // Graphic Variables
    gl::Texture colorTexture;
    Vec2f handCoords;
    Vec2f handVel;
    Perlin mPerlin;
    Area mViewPort;

    bool mDrawParticles;
    bool mDrawImage;
    bool mStopRecording;
    bool mStartRecording;
    bool mRecording;
    bool mCentralGravity,mAllowPerlin;

    // Kinect Callbacks
    KinectCursor *mKinectController;
    volatile bool mInSession,mPushed,wasInSession;

    void OnPush();
    void OnPointUpdate(Vec3f HandPosition);
    void OnFocusStartDetected();
    void OnSessionStart();
    void OnSessionEnd();

    //Particle System
    ParticleController mParticleController;
  
    // Movie Writer
    std::string moviePath;
    qtime::MovieWriter	mMovieWriter;

    // Fbo Support
    gl::Fbo kinectFrame;
    void drawFbo();
    Vec2f getFboCenter();
};

void ScatterTheWorldApp::startRecording(ImageSourceRef testFrame)
{
  mStartRecording = false;
  qtime::MovieWriter::Format format;
  format.setCodec(qtime::MovieWriter::CODEC_H264);
  format.setQuality(.99f);
  mMovieWriter = qtime::MovieWriter(getHomeDirectory() + "/Desktop/TestMovie.mov",getWindowWidth(),getWindowHeight(),format);
  mRecording = true;
}

void ScatterTheWorldApp::stopRecording()
{
  mStopRecording = false;
  if (mRecording) {
    mMovieWriter.finish();
    mRecording = false;
  }
}

void ScatterTheWorldApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize( WIDTH, HEIGHT );
  settings->setFrameRate(60.0f);
  // settings->setFullScreen();
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

  gl::Fbo::Format csaaFormat;
  csaaFormat.setSamples(4);
  csaaFormat.setCoverageSamples(8);
  kinectFrame = gl::Fbo(KINECT_WIDTH, KINECT_HEIGHT, csaaFormat);
  //kinectFrame = gl::Fbo(KINECT_WIDTH, KINECT_HEIGHT);

  mDrawParticles  = true;
  mDrawImage      = false;
  mPushed         = false;
  mCentralGravity = false;
  mAllowPerlin    = false;
  mRecording      = false;
  mStartRecording = false;
  mStopRecording  = false;
  moviePath       = std::string("");
}

void ScatterTheWorldApp::update()
{
  mKinectController->update();
  if (mDrawImage) {
    colorTexture.update(mKinectController->getImageChannel8u(),Area(0,0,KINECT_WIDTH,KINECT_HEIGHT));
  }

  mParticleController.update(mKinectController->getImageChannel32f(),handCoords);

	if( mPushed )
		mParticleController.addParticles( NUM_PARTICLES_TO_SPAWN, handCoords, handVel );

  mParticleController.repulseParticles();
  
  if( mCentralGravity )
    mParticleController.pullToCenter(kinectFrame.getSize()/2);
  
  if( mAllowPerlin )
    mParticleController.applyPerlin( mPerlin );

  if (!wasInSession && mInSession) {
    mParticleController.mParticles.clear();
    mParticleController.mSize = 0;
  }
  wasInSession = mInSession;
}

void ScatterTheWorldApp::drawFbo()
{
  kinectFrame.bindFramebuffer();
  gl::setMatricesWindow( kinectFrame.getSize(),false);
  gl::clear( Color( 0, 0, 0 ) );
  if (mDrawImage) {
    gl::color( Color(1,1,1) );
    gl::draw(colorTexture,kinectFrame.getBounds()); 
  }
  if (mDrawParticles) {
    gl::color( Color(1,1,1) );
    mParticleController.draw();
  }
  if (mInSession) {
    gl::color( Color(1,0,0) );
    gl::drawSolidCircle(handCoords, 2.0f, 20);
    gl::color( Color(1,1,1) );
  }
  kinectFrame.unbindFramebuffer();
}

void ScatterTheWorldApp::draw()
{
  drawFbo();
  if (mStartRecording) {
    startRecording(copyWindowSurface());
  }
  // clear out the window with black
  gl::clear( Color( 0, 0, 0 ) );
  gl::setMatricesWindow( getWindowSize(),true);
  // gl::draw(kinectFrame.getTexture(), Rectf(0.0f,0.0f, getWindowWidth(),getWindowHeight()));
  gl::draw(kinectFrame.getTexture(), getWindowBounds());
  gl::drawString( toString(mParticleController.mParticles.size()) + " Particles", Vec2f(32.0f, 32.0f));
  gl::drawString( notify.str(), Vec2f(32.0f, getWindowHeight()-32.0f));
  if( mRecording ){
		mMovieWriter.addFrame( copyWindowSurface() );
    gl::drawString( "Recording...", Vec2f(getWindowWidth()-52.0f, getWindowHeight()-32.0f));
	}
  gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
  if (mStopRecording) {
    stopRecording();
  }

}

void ScatterTheWorldApp::keyDown( KeyEvent event )
{
  if( event.getChar() == '1' ){
    mDrawImage = ! mDrawImage;
  } else if( event.getChar() == '2' ){
    mDrawParticles = ! mDrawParticles;
  }
  
  if (event.getChar() == 'f') {
    setFullScreen(!isFullScreen());
  }

  if (event.getChar() == 's') {
    if (mRecording) {
      mStopRecording = true;
    } else  {
      mStartRecording = true;
    }
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
  notify.str("");
  notify << "I think I can see you!";
}

void ScatterTheWorldApp::OnSessionStart()
{
  notify.str("");
  notify << "Hello!";
  mInSession = true;
}

void ScatterTheWorldApp::OnSessionEnd()
{
  handCoords = Vec2f(CENTER_X,CENTER_Y);
  notify.str("");
  notify << "GoodBye!";
  mInSession = false;
  mPushed = false;
}

CINDER_APP_BASIC( ScatterTheWorldApp, RendererGl )

