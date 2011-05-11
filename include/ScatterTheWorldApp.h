#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

// OpenNI headers
#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnCppWrapper.h>
// NITE headers
#include <XnVSessionManager.h>
#include <XnVWaveDetector.h>
#include <XnVPushDetector.h>
#include <XnVBroadcaster.h>

#include "ErrorChecking.h"
#include "KinectImageSources.h"

#define KINECT_WIDTH 640
#define KINECT_HEIGHT 480
#define WIDTH 640
#define HEIGHT 480

#define CENTER_X 320
#define CENTER_Y 240

using namespace ci;
using namespace ci::app;
using namespace std;

#define CURRENT_APP ((ScatterTheWorldApp *)ci::app::App::get())

#ifndef SCATTERTHEWORLDAPP_H

#define SCATTERTHEWORLDAPP_H

class ScatterTheWorldApp : public AppBasic, public XnVSessionListener {
  public:

    // Cinder CallBacks.
    void prepareSettings(Settings* settings);
    void setup();
    void mouseDown( MouseEvent event );
    void update();
    void draw();
    void shutdown();
    void OnFocusStartDetected(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress);
    void OnSessionStart(const XnPoint3D& ptFocusPoint);
    void OnSessionEnd();

    void TestValidity() {printf("This is ctx!\n");}
    ImageSourceRef getColorImage();

    // Graphic Variables
    gl::Texture colorTexture;
    Vec2f handCoords;

    // Kinect Variables
    xn::Context            pContext;
    XnVSessionGenerator*   pSessionGenerator;
    xn::ImageGenerator     pImageGen;
    xn::DepthGenerator     pDepthGen;
    XnVBroadcaster         pBroadcaster;
    XnVPushDetector        pPushCtrl;
    XnVWaveDetector        pWaveCtrl;

  private:
    void _TestValidity() {printf("We Are In High Cotton!\n");}
    static void XN_CALLBACK_TYPE OnPushCB(XnFloat fVelocity, XnFloat fAngle,void* cxt);
};



#endif /* end of include guard: SCATTERTHEWORLD_H */
