#ifndef EGAPP_H_
#define EGAPP_H_

#include <AppInterface.h>
#include <DisplayMPI.h>
#include <EventMPI.h>
#include <FontMPI.h>
#include <ButtonEventQueue.h>
#include <TouchEventQueue.h>
#include <BlitBuffer.h>

class VtsApp : public CAppInterface
{
public:
	VtsApp(void *userData);
	~VtsApp();

	void Enter();
	void Update();
	void Exit();

	void Suspend();
	void Resume();

private:
	LeapFrog::Brio::CDisplayMPI mDisplayMPI;
	LeapFrog::Brio::tDisplayHandle mDisplayHandle;
	LeapFrog::Brio::CEventMPI mEventMPI;
	LeapFrog::Brio::CButtonEventQueue mButtonEventQueue;
	LeapFrog::Brio::CTouchEventQueue mTouchEventQueue;
	LeapFrog::Brio::CFontMPI mFontMPI;

	CBlitBuffer mScreenBlitBuffer;
};


#endif // EGAPP_H_

