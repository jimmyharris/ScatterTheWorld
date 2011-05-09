#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

// OpenNI headers
#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnCppWrapper.h>
// NITE headers
#include <XnVSessionManager.h>
#include "XnVMultiProcessFlowClient.h"
#include <XnVWaveDetector.h>

#include "ErrorChecking.h"

#define KINECT_WIDTH 640
#define KINECT_HEIGHT 480
#define WIDTH 640
#define HEIGHT 480

using namespace ci;
using namespace ci::app;
using namespace std;

XnPoint3D perspectivePoint;
Vec2f handCoords;
xn::DepthGenerator pDepthGen;


//----------------------------
// Image Sources
//----------------------------
class ImageSourceKinectColor : public ImageSource
{
  public:
    ImageSourceKinectColor(xn::ImageMetaData &newData) : ImageSource()
    {
      _myMetaData.InitFrom(newData);
      _width = _myMetaData.XRes();
      _height = _myMetaData.YRes();
      mData = _myMetaData.WritableData();
      setSize( _width, _height );
      setColorModel( ImageIo::CM_RGB );
      setChannelOrder( ImageIo::RGB );
      setDataType( ImageIo::UINT8 );
    }
    ImageSourceKinectColor( uint8_t *buffer, int width, int height )
      : ImageSource(), mData( buffer ), _width(width), _height(height)
    {
      setSize( _width, _height );
      setColorModel( ImageIo::CM_RGB );
      setChannelOrder( ImageIo::RGB );
      setDataType( ImageIo::UINT8 );
    }

    ~ImageSourceKinectColor()
    {
      // mData is actually a ref. It's released from the device.
      /*if( mData ) {
        delete[] mData;
        mData = NULL;
        }*/
    }

    virtual void load( ImageTargetRef target )
    {
      ImageSource::RowFunc func = setupRowFunc( target );

      for( uint32_t row	 = 0; row < _height; ++row )
        ((*this).*func)( target, row, mData + row * _width * 3 );
    }

  protected:
    uint32_t					_width, _height;
    uint8_t						*mData;
  private: 
    xn::ImageMetaData _myMetaData;
};

class ImageSourceKinectDepth : public ImageSource
{
  public:
    ImageSourceKinectDepth( uint16_t *buffer, int width, int height )
      : ImageSource(), mData( buffer ), _width(width), _height(height)
    {
      setSize( _width, _height );
      setColorModel( ImageIo::CM_GRAY );
      setChannelOrder( ImageIo::Y );
      setDataType( ImageIo::UINT16 );
    }

    ~ImageSourceKinectDepth()
    {
    }

    virtual void load( ImageTargetRef target )
    {
      ImageSource::RowFunc func = setupRowFunc( target );

      for( uint32_t row = 0; row < _height; ++row )
        ((*this).*func)( target, row, mData + row * _width );
    }

  protected:
    uint32_t					_width, _height;
    uint16_t					*mData;
};
//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE SessionProgress(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* UserCxt)
{
  printf("Session progress (%6.2f,%6.2f,%6.2f) - %6.2f [%s]\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z, fProgress,  strFocus);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& ptFocusPoint, void* UserCxt)
{
  printf("Session started. Please wave (%6.2f,%6.2f,%6.2f)...\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z);
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt)
{
  //handCoords = Vec3f(0.0f,0.0f,0.0f);
  handCoords = Vec2f(0.0f,0.0f);
  printf("Session ended. Please perform focus gesture to start session\n");
}
// Callback for wave detection
void XN_CALLBACK_TYPE OnWaveCB(void* cxt)
{
  printf("Wave!\n");
}
// callback for a new position of any hand
void XN_CALLBACK_TYPE OnPointUpdate(const XnVHandPointContext* pContext, void* cxt)
{
  perspectivePoint = pContext->ptPosition;
  pDepthGen.ConvertRealWorldToProjective(1, &perspectivePoint, &perspectivePoint);
  handCoords = Vec2f(perspectivePoint.X, perspectivePoint.Y);
  printf("%d: (%f,%f,%f) [%f]\n", pContext->nID, handCoords.x, handCoords.y, perspectivePoint.Z, pContext->fTime);
}

class ScatterTheWorldApp : public AppBasic {
  public:
    void prepareSettings(Settings* settings);
    void setup();
    void mouseDown( MouseEvent event );
    void update();
    void draw();
    void shutdown();


    xn::Context context;
    XnVSessionGenerator* pSessionGenerator;
    xn::ImageGenerator pImageGen;
    XnBool bRemoting;
    XnVWaveDetector wc;
    gl::Texture colorTexture;
  
    ImageSourceRef getColorImage(){
      //Get an ImageRef from the Active MetaData
      xn::ImageMetaData metaData;
      pImageGen.GetMetaData(metaData);
      return ImageSourceRef( new ImageSourceKinectColor(metaData));
    }
  
};

void ScatterTheWorldApp::prepareSettings(Settings* settings)
{
  settings->setWindowSize(WIDTH, HEIGHT);
}

void ScatterTheWorldApp::setup()
{
  bRemoting = FALSE;
  pSessionGenerator = NULL;
  xn::EnumerationErrors errors;
  XnStatus rc = context.InitFromXmlFile(getResourcePath("ScatterTheWorld.xml").c_str(),&errors);
  CHECK_ERRORS(rc,errors,"InitFromXmlFile")
  CHECK_RC(rc,"XML Config")

  pSessionGenerator = new XnVSessionManager();
  rc = ((XnVSessionManager*)pSessionGenerator)->Initialize(&context, "Click", "RaiseHand");
  CHECK_RC(rc,"Session Manager initialization")

  rc = context.FindExistingNode(XN_NODE_TYPE_IMAGE,pImageGen);
  CHECK_RC(rc,"Find Image Generator.")
  rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH,pDepthGen);
  CHECK_RC(rc,"Find Depth Generator.")

  context.StartGeneratingAll();

  pSessionGenerator->RegisterSession(NULL, &SessionStart, &SessionEnd, &SessionProgress);


  wc.RegisterWave(NULL, OnWaveCB);
  wc.RegisterPointUpdate(NULL, OnPointUpdate);
  pSessionGenerator->AddListener(&wc);

  gl::Texture::Format format;
	colorTexture = gl::Texture( WIDTH, HEIGHT, format );
  printf("Please perform focus gesture to start session\n");

}

void ScatterTheWorldApp::mouseDown( MouseEvent event )
{
}

void ScatterTheWorldApp::update()
{
  context.WaitAndUpdateAll();
  ((XnVSessionManager*)pSessionGenerator)->Update(&context);
  colorTexture.update(getColorImage());
}

void ScatterTheWorldApp::draw()
{
  // clear out the window with black
  gl::clear( Color( 0, 0, 0 ) );

  gl::draw(colorTexture,getWindowBounds()); 
  //gl::color(Color(1.0f, .0f, .0f));
  //gl::translate(handCoords);
  gl::drawSolidCircle(handCoords, 10.0f, 20);
 
}

void ScatterTheWorldApp::shutdown()
{
  delete pSessionGenerator;

  context.Shutdown();
}


CINDER_APP_BASIC( ScatterTheWorldApp, RendererGl )
