#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/Vector.h"
#include "cinder/ImageIo.h"
#include "cinder/Channel.h"
#include "cinder/Thread.h"

// OpenNI headers
#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnCppWrapper.h>
// NITE headers
#include <XnVSessionManager.h>
#include <XnVWaveDetector.h>
#include <XnVPushDetector.h>
#include <XnVBroadcaster.h>
#include <sstream>
using std::stringstream;

#include "ErrorChecking.h"
#include "KinectImageSources.h"


using namespace cinder;
using namespace cinder::app;
using namespace std;

#ifndef KINECTCURSOR_H

#define KINECTCURSOR_H

#define KINECT_WIDTH 640
#define KINECT_HEIGHT 480
#define AS_KINECT_CURSOR(pointer) ((KinectCursor *)pointer)



class KinectCursor {
  public:

    // Kinect Listerer Interface. (YAY! Multiple Inheritance!)
    class KinectListener {
      public:
        virtual void OnPush() {}
        virtual void OnPointUpdate(Vec3f HandPosition) {}
        virtual void OnFocusStartDetected() {}
        virtual void OnSessionStart() {};
        virtual void OnSessionEnd() {};
        std::mutex mListinerMutex;
        std::stringstream notify;
    };


    // Constructors
    KinectCursor(KinectListener *listiner) : _CursorListiner(listiner), _SessionGenerator(NULL)
    {
      setup();
    }

    // Destructor
    virtual ~KinectCursor() {
      if(_SessionGenerator) 
      {
        delete _SessionGenerator; 
      }
      _Context.Shutdown();
    }

    void EndSession() {
      ((XnVSessionManager *)_SessionGenerator)->EndSession();
    }

    // Prototypes
    void update();
    void setup();
    volatile bool         image_ready;

    ImageSourceRef getColorImage();
    Channel8u getImageChannel8u();
    Channel32f getImageChannel32f();
    std::mutex            pImageMutex;

  protected:
    KinectListener*  _CursorListiner;
    
  
  private: 

    xn::Context           _Context;
    XnVSessionGenerator*  _SessionGenerator;
    xn::ImageGenerator    _ImageGen;
    xn::DepthGenerator    _DepthGen;
    XnVBroadcaster        _Broadcaster;
    XnVPushDetector       _PushCtrl;
    XnVWaveDetector       _WaveCtrl;
    std::mutex            _listener_mutex;
    ImageSourceRef        _colorImage;


    static void XN_CALLBACK_TYPE OnWaveCB(void* currentInstance);
    static void XN_CALLBACK_TYPE OnPushCB(XnFloat fVelocity, XnFloat fAngle, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnPointUpdate(const XnVHandPointContext* pContext, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnFocusStartDetected(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnSessionStart(const XnPoint3D& ptFocusPoint, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnSessionEnd(void* CurrentInstance);
};

class ThreadedKinectCursor : public KinectCursor {
public:
  ThreadedKinectCursor(KinectListener *listiner): _shutDown(false), KinectCursor(listiner)
  {
  }
  ~ThreadedKinectCursor()
  {
    _shutDown = true;
    _thread->join();
  }
  void startThreadedUpdate()
  {
    _thread = std::shared_ptr<boost::thread>(new boost::thread(&ThreadedKinectCursor::_threadedUpdate, this));
  }
private:
  std::shared_ptr<boost::thread> _thread;
  volatile bool _shutDown;
  void _threadedUpdate()
  {
    while (!_shutDown) {
      update();
    }
  }
};

#endif /* end of include guard: SCATTERTHEWORLD_H */
