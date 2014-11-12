#ifndef STUBMainState_H_
#define STUBMainState_H_

#include <DebugMPI.h>
#include <GameStateHandler.h>
#include <GameStateBase.h>
#include <DisplayMPI.h>
#include <ButtonMPI.h>
#include <EventMPI.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <CameraMPI.h>

#include "STUBApp.h"

#include <Vision/VNVisionMPI.h>

#include <BrioOpenGLConfig.h>
#include <GLES2/gl2.h>


using namespace LeapFrog::Brio;

class STUBMainState : public CGameStateBase
{
public:
	STUBMainState();
	~STUBMainState();

	virtual void Suspend();
	virtual void Resume();
	virtual void Enter(CStateData* userData);
	virtual void Update(CGameStateHandler* sh);
	virtual void Exit();

protected:
	LeapFrog::Brio::CDebugMPI			debugMPI_;
	int DoTouchEventLoop();
	int DoKeyEventLoop();

	static STUBMainState *				mState;

	LeapFrog::Brio::U16 				currentSelection_;

	LeapFrog::Brio::CTouchEventQueue 	m_touch_event_queue;
	LeapFrog::Brio::CButtonEventQueue 	m_button_event_queue;

	//SAMPLE CODE
	void initSample();
	void initCamera(int cw, int ch);
	void initVision();
	void initOGL();

	void drawSample();
	void checkGLError();
	void test();

	//////////////////////
	LeapFrog::Brio::CDisplayMPI			mDisplayMPI;
	LeapFrog::Brio::CCameraMPI			mCameraMPI;
	int		screenWidth, screenHeight;

	LF::Vision::VNVisionMPI			mVisionMPI;
	LF::Vision::VNAlgorithm*        m_visionAlgorithm;

	std::vector<const LF::Vision::VNHotSpot*> hot_spots_;

	/////////////////
	LeapFrog::Brio::tVideoHndl mVideoHandle;
	LeapFrog::Brio::tVideoSurf mVideoSurf;
	LeapFrog::Brio::tVidCapHndl mVideoCaptureHandle;
	LeapFrog::Brio::tDisplayHandle mDisplayHandle;

	LeapFrog::Brio::U8*	mDisplayBuffers[3];
	unsigned char* camBuffer_;
	GLuint mFragShader;
	GLuint mVertexShader;
	GLuint mProgramObject;
	GLuint mYUVTexture[3];

	/////////////
	LeapFrog::Brio::BrioOpenGLConfig *mBrioOpenGLConfig;

	class HotSpotListener : public IEventListener {
	public:
	  HotSpotListener();
	  tEventStatus Notify( const IEventMessage& msg );
	};
	HotSpotListener				m_hotspot_listener;

	bool bRotate;
};



#endif // STUBMainState_H_

