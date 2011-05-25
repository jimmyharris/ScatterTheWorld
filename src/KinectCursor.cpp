#include "KinectCursor.h"

void XN_CALLBACK_TYPE KinectCursor::OnWaveCB(void* CurrentInstance)
{
  KinectCursor *_this = ((KinectCursor*)CurrentInstance);
  #ifdef DEBUG
    printf("Wave!\n");
  #endif
  _this->EndSession();
}

void XN_CALLBACK_TYPE KinectCursor::OnPushCB(XnFloat fVelocity, XnFloat fAngle, void* CurrentInstance)
{
  KinectCursor *_this = ((KinectCursor*)CurrentInstance);
  #ifdef DEBUG
    printf("Wave!\n");
  #endif
  _this->mCursorListiner->OnPush();
}

// callback for a new position of any hand
void XN_CALLBACK_TYPE KinectCursor::OnPointUpdate(const XnVHandPointContext* pContext, void* CurrentInstance)
{
  KinectCursor *_this = ((KinectCursor*)CurrentInstance);
  XnPoint3D perspectivePoint = pContext->ptPosition;
  _this->mDepthGen.ConvertRealWorldToProjective(1, &perspectivePoint, &perspectivePoint);
  #ifdef DEBUG
    printf("%d: (%f,%f,%f) [%f]\n", pContext->nID, perspectivePoint.X, perspectivePoint.Y, perspectivePoint.Z, pContext->fTime);
  #endif
  _this->mCursorListiner->OnPointUpdate(Vec3f(perspectivePoint.X, perspectivePoint.Y, perspectivePoint.Z));
}

void XN_CALLBACK_TYPE KinectCursor::OnFocusStartDetected(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* CurrentInstance)
{
  KinectCursor *_this = ((KinectCursor*)CurrentInstance);
#ifdef DEBUG
  printf("Session progress (%6.2f,%6.2f,%6.2f) - %6.2f [%s]\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z, fProgress,  strFocus);
#endif
  _this->mCursorListiner->OnFocusStartDetected();
}

// callback for session start
void XN_CALLBACK_TYPE KinectCursor::OnSessionStart(const XnPoint3D& ptFocusPoint, void* CurrentInstance)
{
  KinectCursor * _this = ((KinectCursor*)CurrentInstance);
#ifdef DEBUG
  printf("Session started. Please wave (%6.2f,%6.2f,%6.2f)...\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z);
#endif
  _this->mCursorListiner->OnSessionStart();
}

// Callback for session end
void XN_CALLBACK_TYPE KinectCursor::OnSessionEnd(void* CurrentInstance)
{
  KinectCursor * _this = ((KinectCursor*)CurrentInstance);
#ifdef DEBUG
  printf("Session ended. Please perform focus gesture to start session\n");
#endif
  _this->mCursorListiner->OnSessionEnd();
}
  
ImageSourceRef KinectCursor::getColorImage()
{
  //Get an ImageRef from the Active MetaData
  return mColorImage;
}

Channel8u KinectCursor::getImageChannel8u()
{
  return Channel8u(mColorImage);
}

Channel32f KinectCursor::getImageChannel32f()
{
  return Channel32f(mColorImage);
}

void KinectCursor::setup()
{
  xn::EnumerationErrors errors;
  mRc = mContext.InitFromXmlFile(App::getResourcePath("ScatterTheWorld.xml").c_str(),&errors);
  CHECK_ERRORS_AND_RETURN(mRc,errors,"InitFromXmlFile")
  CHECK_RC_AND_RETURN(mRc,"XML Config")

  mSessionGenerator = new XnVSessionManager();
  mRc = ((XnVSessionManager*)mSessionGenerator)->Initialize(&mContext, "Wave", "RaiseHand");
  CHECK_RC_AND_RETURN(mRc,"Session Manager initialization")

  mRc = mContext.FindExistingNode(XN_NODE_TYPE_IMAGE,mImageGen);
  CHECK_RC_AND_RETURN(mRc,"Find Image Generator.")
  mRc = mContext.FindExistingNode(XN_NODE_TYPE_DEPTH,mDepthGen);
  CHECK_RC_AND_RETURN(mRc,"Find Depth Generator.")

  mContext.StartGeneratingAll();
 
  mSessionGenerator->RegisterSession(this, KinectCursor::OnSessionStart, KinectCursor::OnSessionEnd, KinectCursor::OnFocusStartDetected);

  mWaveCtrl.RegisterWave(this, KinectCursor::OnWaveCB);
  mWaveCtrl.RegisterPointUpdate(this, KinectCursor::OnPointUpdate);
  mPushCtrl.RegisterPush(this, KinectCursor::OnPushCB);
  mBroadcaster.AddListener(&mWaveCtrl);
  mBroadcaster.AddListener(&mPushCtrl);
  mSessionGenerator->AddListener(&mBroadcaster);
  mCursorListiner->notify.str("");
  mCursorListiner->notify << "Please perform focus gesture to start session";
}

void KinectCursor::update()
{
  mContext.WaitAndUpdateAll();
  ((XnVSessionManager*)mSessionGenerator)->Update(&mContext);
  
  //Load Color Image Map.
  xn::ImageMetaData metaData;
  mImageGen.GetMetaData(metaData);
  mColorImage = ImageSourceRef(new ImageSourceKinectColor(metaData));
}



