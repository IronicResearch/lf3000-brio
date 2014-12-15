/******************************************************************************

 @Description  A very simple app that uses the Buttons

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ButtonMPI.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <TouchTypes.h>
#include <boost/scoped_ptr.hpp>
#ifdef EMULATION
#include <EmulationConfig.h>
#include <BrioOpenGLConfig.h>
#endif

LF_USING_BRIO_NAMESPACE()


//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

const tEventType kMyHandledTypes[] = { kAllButtonEvents, kAllTouchEvents };

#define NUM_BUTTONS 19
const char *button_names[NUM_BUTTONS] = {
	"kButtonNull",
	"kButtonUpKey",
	"kButtonDownKey",
	"kButtonRightKey",
	"kButtonLeftKey",
	"kButtonAKey",
	"kButtonBKey",
	"kButtonLeftShoulderKey",
	"kButtonRightShoulderKey",
	"kButtonMenuKey",
	"kButtonHintKey",
	"kButtonPauseKey",
	"kButtonBrightnessKey",
	"kHeadphoneJackDetectKey",
	"kCartridgeDetectKey",
	"kButtonVolumeDown",
	"kButtonVolumeUp",
	"kButtonEscape",
	"kButtonSync"
};

//----------------------------------------------------------------------------
class MyBtnEventListener : public IEventListener
{
 public:
	MyBtnEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)),dbg_(kMyApp) 
	{
		dbg_.SetDebugLevel(kDbgLvlVerbose);
		dbg_.DebugOut(kDbgLvlVerbose, "MyBtnEventListener setup...\n");		
	}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
		{
			int button;
			type_ = msg.GetEventType();
			
			// Touch event message?
			if (type_ == kTouchStateChanged)
			{
				const CTouchMessage& touchmsg = reinterpret_cast<const CTouchMessage&>(msg);
				tTouchData state = touchmsg.GetTouchState();
				dbg_.DebugOut(kDbgLvlVerbose, "kTouchStateChanged %d: %d,%d\n", 
						state.touchState, state.touchX, state.touchY);
				return kEventStatusOKConsumed;
			}

			// Button event message
			const CButtonMessage& btnmsg = reinterpret_cast<const CButtonMessage&>(msg);
			tButtonData state = btnmsg.GetButtonState();
			
			for(button=0; button<NUM_BUTTONS-1; button++) {
			  if (state.buttonTransition & 1<<button) {
				dbg_.DebugOut(kDbgLvlVerbose, "%s %s\n",
				       button_names[button+1],
				       state.buttonState & 1<<button ?
				       "press " : "release ");
			  }
			}
			dbg_.DebugOut(kDbgLvlVerbose, "\n");

			return kEventStatusOKConsumed;
		}
	tEventType	type_;
	tButtonData	data_;
private:
	CDebugMPI	dbg_;
};

int main()
{
#ifdef EMULATION
	// Need X window opened for capturing events on emulation target
	BrioOpenGLConfig   config;
#endif
	
	MyBtnEventListener handler;

	CDebugMPI dbg(kMyApp);

	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "ButtonDemo setup...timeout after 10 sec\n");

	CButtonMPI	btnmgr;
	
	if (btnmgr.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return -1;
	}
	
	dbg.DebugOut(kDbgLvlVerbose, "ButtonDemo loop: Press a button\n");
	for (int i = 0; i < 30; i++) {
		/* Just hang around waiting for handler to be called. */
		sleep(1);
	}

	dbg.DebugOut(kDbgLvlVerbose, "ButtonDemo done.\n");
	btnmgr.UnregisterEventListener(&handler);
	return 0;
}
