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
  _this->_CursorListiner->OnPush();
}

// callback for a new position of any hand
void XN_CALLBACK_TYPE KinectCursor::OnPointUpdate(const XnVHandPointContext* pContext, void* CurrentInstance)
{
  KinectCursor *_this = ((KinectCursor*)CurrentInstance);
  XnPoint3D perspectivePoint = pContext->ptPosition;
  _this->_DepthGen.ConvertRealWorldToProjective(1, &perspectivePoint, &perspectivePoint);
  #ifdef DEBUG
    printf("%d: (%f,%f,%f) [%f]\n", pContext->nID, perspectivePoint.X, perspectivePoint.Y, perspectivePoint.Z, pContext->fTime);
  #endif
  _this->_CursorListiner->OnPointUpdate(Vec3f(perspectivePoint.X, perspectivePoint.Y, perspectivePoint.Z));
}

void XN_CALLBACK_TYPE KinectCursor::OnFocusStartDetected(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* CurrentInstance)
{
  KinectCursor *_this = ((KinectCursor*)CurrentInstance);
#ifdef DEBUG
  printf("Session progress (%6.2f,%6.2f,%6.2f) - %6.2f [%s]\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z, fProgress,  strFocus);
#endif
  _this->_CursorListiner->OnFocusStartDetected();
}

// callback for session start
void XN_CALLBACK_TYPE KinectCursor::OnSessionStart(const XnPoint3D& ptFocusPoint, void* CurrentInstance)
{
  KinectCursor * _this = ((KinectCursor*)CurrentInstance);
#ifdef DEBUG
  printf("Session started. Please wave (%6.2f,%6.2f,%6.2f)...\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z);
#endif
  _this->_CursorListiner->OnSessionStart();
}

// Callback for session end
void XN_CALLBACK_TYPE KinectCursor::OnSessionEnd(void* CurrentInstance)
{
  KinectCursor * _this = ((KinectCursor*)CurrentInstance);
#ifdef DEBUG
  printf("Session ended. Please perform focus gesture to start session\n");
#endif
  _this->_CursorListiner->OnSessionEnd();
}
  
ImageSourceRef KinectCursor::getColorImage()
{
  //Get an ImageRef from the Active MetaData
  xn::ImageMetaData metaData;
  _ImageGen.GetMetaData(metaData);
  return ImageSourceRef( new ImageSourceKinectColor(metaData));
}

Channel8u KinectCursor::getImageChannel8u()
{
  xn::ImageMetaData metaData;
  _ImageGen.GetMetaData(metaData);
  return Channel8u(ImageSourceRef( new ImageSourceKinectColor(metaData)));
}

Channel32f KinectCursor::getImageChannel32f()
{
  xn::ImageMetaData metaData;
  _ImageGen.GetMetaData(metaData);
  return Channel32f(ImageSourceRef( new ImageSourceKinectColor(metaData)));
}


void KinectCursor::setup()
{
  xn::EnumerationErrors errors;
  XnStatus rc = _Context.InitFromXmlFile(App::getResourcePath("ScatterTheWorld.xml").c_str(),&errors);
  CHECK_ERRORS(rc,errors,"InitFromXmlFile")
  CHECK_RC(rc,"XML Config")

  _SessionGenerator = new XnVSessionManager();
  rc = ((XnVSessionManager*)_SessionGenerator)->Initialize(&_Context, "Wave", "RaiseHand");
  CHECK_RC(rc,"Session Manager initialization")

  rc = _Context.FindExistingNode(XN_NODE_TYPE_IMAGE,_ImageGen);
  CHECK_RC(rc,"Find Image Generator.")
  rc = _Context.FindExistingNode(XN_NODE_TYPE_DEPTH,_DepthGen);
  CHECK_RC(rc,"Find Depth Generator.")

  _Context.StartGeneratingAll();

 
  _SessionGenerator->RegisterSession(this, KinectCursor::OnSessionStart, KinectCursor::OnSessionEnd, KinectCursor::OnFocusStartDetected);

  _WaveCtrl.RegisterWave(this, KinectCursor::OnWaveCB);
  _WaveCtrl.RegisterPointUpdate(this, KinectCursor::OnPointUpdate);
  _PushCtrl.RegisterPush(this, KinectCursor::OnPushCB);
  _Broadcaster.AddListener(&_WaveCtrl);
  _Broadcaster.AddListener(&_PushCtrl);
  _SessionGenerator->AddListener(&_Broadcaster);

  printf("Please perform focus gesture to start session\n");

}


void KinectCursor::update()
{
  _Context.WaitAndUpdateAll();
  ((XnVSessionManager*)_SessionGenerator)->Update(&_Context);
}


