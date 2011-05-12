#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "KinectCursor.h"

#define WIDTH 640
#define HEIGHT 480

#define CENTER_X 320
#define CENTER_Y 240

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

    // Graphic Variables
    gl::Texture colorTexture;
    Vec2f handCoords;

    // Kinect Callbacks
    KinectCursor *mKinectController;

    void OnPush();
    void OnPointUpdate(Vec3f HandPosition);
    void OnFocusStartDetected();
    void OnSessionStart();
    void OnSessionEnd();

};


void ScatterTheWorldApp::prepareSettings(Settings* settings)
{
  settings->setWindowSize(WIDTH, HEIGHT);
  settings->setFrameRate(60.0f);
}

void ScatterTheWorldApp::setup()
{

  handCoords = Vec2f(CENTER_X,CENTER_Y);

  //Setup the kinect. (This takes a while I wish I could do this in a different thread)
  mKinectController = new KinectCursor((KinectListener *)this);
  // Initialize our Video texture.
  gl::Texture::Format format;
	colorTexture = gl::Texture( KINECT_WIDTH, KINECT_HEIGHT, format );

  printf("Please wave to start session\n");
}


void ScatterTheWorldApp::update()
{
  mKinectController->update();
  colorTexture.update(mKinectController->getColorImage());
}

void ScatterTheWorldApp::draw()
{
  // clear out the window with black
  gl::clear( Color( 0, 0, 0 ) );

  gl::color( Color(1,1,1) );
  gl::draw(colorTexture,getWindowBounds()); 

  gl::color( Color(1,0,0) );
  gl::drawSolidCircle(handCoords, 10.0f, 20);
 
}

void ScatterTheWorldApp::shutdown()
{
  delete mKinectController;
}


void ScatterTheWorldApp::OnPush()
{
  printf("I pushed!\n");
}
void ScatterTheWorldApp::OnPointUpdate(Vec3f HandPosition)
{
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
}
void ScatterTheWorldApp::OnSessionEnd()
{
  handCoords = Vec2f(CENTER_X,CENTER_Y);
  printf("GoodBye!\n");
}




CINDER_APP_BASIC( ScatterTheWorldApp, RendererGl )
