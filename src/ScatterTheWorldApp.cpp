#include "ScatterTheWorldApp.h"


ScatterTheWorldApp* gCurrentApp;



//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

// Callback for wave detection
void XN_CALLBACK_TYPE OnWaveCB(void* cxt)
{
  printf("Wave!\n");
  ((XnVSessionManager *)CURRENT_APP->pSessionGenerator)->EndSession();
}

void XN_CALLBACK_TYPE ScatterTheWorldApp::OnPushCB(XnFloat fVelocity, XnFloat fAngle,void* cxt)
{
  ((ScatterTheWorldApp *)cxt)->TestValidity();
  ((ScatterTheWorldApp *)cxt)->_TestValidity();
  printf("Push!\n");
}

// callback for a new position of any hand
void XN_CALLBACK_TYPE OnPointUpdate(const XnVHandPointContext* pContext, void* cxt)
{
  XnPoint3D perspectivePoint = pContext->ptPosition;
  gCurrentApp->pDepthGen.ConvertRealWorldToProjective(1, &perspectivePoint, &perspectivePoint);
  gCurrentApp->handCoords = Vec2f(perspectivePoint.X, perspectivePoint.Y);
  #ifdef DEBUG
    printf("%d: (%f,%f,%f) [%f]\n", pContext->nID, perspectivePoint.X, perspectivePoint.Y, perspectivePoint.Z, pContext->fTime);
  #endif
}

void ScatterTheWorldApp::OnFocusStartDetected(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress)
{
#ifdef DEBUG
  printf("Session progress (%6.2f,%6.2f,%6.2f) - %6.2f [%s]\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z, fProgress,  strFocus);
#endif
}

// callback for session start
void ScatterTheWorldApp::OnSessionStart(const XnPoint3D& ptFocusPoint)
{
#ifdef DEBUG
  printf("Session started. Please wave (%6.2f,%6.2f,%6.2f)...\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z);
#endif
}

// Callback for session end
void ScatterTheWorldApp::OnSessionEnd()
{
  handCoords = Vec2f(CENTER_X,CENTER_Y);
#ifdef DEBUG
  printf("Session ended. Please perform focus gesture to start session\n");
#endif
}
  
ImageSourceRef ScatterTheWorldApp::getColorImage()
{
  //Get an ImageRef from the Active MetaData
  xn::ImageMetaData metaData;
  pImageGen.GetMetaData(metaData);
  return ImageSourceRef( new ImageSourceKinectColor(metaData));
}

void ScatterTheWorldApp::prepareSettings(Settings* settings)
{
  settings->setWindowSize(WIDTH, HEIGHT);
  settings->setFrameRate(60.0f);
}

void ScatterTheWorldApp::setup()
{
  gCurrentApp = this;
  pSessionGenerator = NULL;

  handCoords = Vec2f(CENTER_X,CENTER_Y);

  xn::EnumerationErrors errors;
  XnStatus rc = pContext.InitFromXmlFile(getResourcePath("ScatterTheWorld.xml").c_str(),&errors);
  CHECK_ERRORS(rc,errors,"InitFromXmlFile")
  CHECK_RC(rc,"XML Config")

  pSessionGenerator = new XnVSessionManager();
  rc = ((XnVSessionManager*)pSessionGenerator)->Initialize(&pContext, "Wave", "RaiseHand");
  CHECK_RC(rc,"Session Manager initialization")

  rc = pContext.FindExistingNode(XN_NODE_TYPE_IMAGE,pImageGen);
  CHECK_RC(rc,"Find Image Generator.")
  rc = pContext.FindExistingNode(XN_NODE_TYPE_DEPTH,pDepthGen);
  CHECK_RC(rc,"Find Depth Generator.")

  pContext.StartGeneratingAll();

  pSessionGenerator->RegisterSession((XnVSessionListener*)this);

  pWaveCtrl.RegisterWave(NULL, OnWaveCB);
  pWaveCtrl.RegisterPointUpdate(NULL, OnPointUpdate);
  pPushCtrl.RegisterPush(this, ScatterTheWorldApp::OnPushCB);
  pBroadcaster.AddListener(&pWaveCtrl);
  pBroadcaster.AddListener(&pPushCtrl);
  pSessionGenerator->AddListener(&pBroadcaster);

  // Initialize our Video texture.
  gl::Texture::Format format;
	colorTexture = gl::Texture( KINECT_WIDTH, KINECT_HEIGHT, format );

  printf("Please perform focus gesture to start session\n");

}

void ScatterTheWorldApp::mouseDown( MouseEvent event )
{
}

void ScatterTheWorldApp::update()
{
  pContext.WaitAndUpdateAll();
  ((XnVSessionManager*)pSessionGenerator)->Update(&pContext);
  colorTexture.update(getColorImage());
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
  delete pSessionGenerator;

  pContext.Shutdown();
}

CINDER_APP_BASIC( ScatterTheWorldApp, RendererGl )
