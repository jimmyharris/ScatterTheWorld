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
        std::stringstream notify;
    };

    // Constructors
    KinectCursor(KinectListener *listiner) : mCursorListiner(listiner), mSessionGenerator(NULL), mSetupDidComplete(false), mIsValid(false) {}

    // Destructor
    virtual ~KinectCursor() {
      if(mSessionGenerator) 
      {
        delete mSessionGenerator; 
      }
      mContext.Shutdown();
    }

    void EndSession() {
      ((XnVSessionManager *)mSessionGenerator)->EndSession();
    }

    void threadedSetup()
    {
      mSetupThread = std::shared_ptr<boost::thread>(new boost::thread(&KinectCursor::mThreadedSetup, this));
    }

    // Prototypes
    void update();
    void setup();

    ImageSourceRef getColorImage();
    Channel8u getImageChannel8u();
    Channel32f getImageChannel32f();

    inline bool setupIsComplete()
    {
      return mSetupDidComplete;
    }

    inline bool isValid()
    {
      return mIsValid;
    }

  protected:
    KinectListener*  mCursorListiner;
  
  private: 

    xn::Context                    mContext;
    XnVSessionGenerator*           mSessionGenerator;
    xn::ImageGenerator             mImageGen;
    xn::DepthGenerator             mDepthGen;
    XnVBroadcaster                 mBroadcaster;
    XnVPushDetector                mPushCtrl;
    XnVWaveDetector                mWaveCtrl;
    ImageSourceRef                 mColorImage;
    volatile bool                  mSetupDidComplete,mIsValid;
    XnStatus                       mRc;
    std::shared_ptr<boost::thread> mSetupThread;

    void mThreadedSetup() {
      setup();
      mSetupDidComplete = true;
      mIsValid = (mRc == XN_STATUS_OK);
    }

    static void XN_CALLBACK_TYPE OnWaveCB(void* currentInstance);
    static void XN_CALLBACK_TYPE OnPushCB(XnFloat fVelocity, XnFloat fAngle, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnPointUpdate(const XnVHandPointContext* pContext, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnFocusStartDetected(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnSessionStart(const XnPoint3D& ptFocusPoint, void* CurrentInstance);
    static void XN_CALLBACK_TYPE OnSessionEnd(void* CurrentInstance);
};
#endif /* KINECTCURSOR_H */

