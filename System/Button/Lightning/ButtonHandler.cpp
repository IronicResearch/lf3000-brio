//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		ButtonHandler.cpp
//
// Description:
//		Implements the underlying Button Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <ButtonPriv.h>
#include <AudioMPI.h>
#include <DisplayMPI.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>
#include <Utility.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>


LF_BEGIN_BRIO_NAMESPACE()



//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	U32					gLastState;
	int 				button_fd;
	char				*dev_name;
	tTaskHndl			handleButtonTask;
	tButtonData	data;
}

// Linux keyboard to Brio button mapping
U32 LinuxKeyToBrio(U16 code)
{
	switch(code) {
		case KEY_UP: 		return kButtonUp;
		case KEY_DOWN:		return kButtonDown;
		case KEY_RIGHT:		return kButtonRight;
		case KEY_LEFT:		return kButtonLeft;
		case KEY_A:			return kButtonA;
		case KEY_B:			return kButtonB;
		case KEY_L:			return kButtonLeftShoulder;
		case KEY_R:			return kButtonRightShoulder;
		case KEY_M:			return kButtonMenu;
		case KEY_H:			return kButtonHint;
		case KEY_P:			return kButtonPause;
		case KEY_X:			return kButtonBrightness;
	}
	return 0;
}

// Linux keyboard to Brio switches mapping
U32 LinuxSwitchToBrio(U16 code)
{
	switch(code) {
		case SW_HEADPHONE_INSERT:	return kHeadphoneJackDetect;
		case SW_TABLET_MODE:		return kCartridgeDetect;
	}
	return 0;
}

//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void *LightningButtonTask(void*)
{
	struct input_event ev[1];
	CEventMPI 	eventmgr;
	CDebugMPI	dbg(kGroupButton);
	CKernelMPI	kernel;
	CDisplayMPI dispmgr;
	
	/*
	 * 'Brighten' screen by adjusting LCD brightness and LCD backlight.  As
	 * 'N' levels are desired, then array will have 2(N-1) entries, first
	 * increasing then increasing in value.
	 */
	
	#define	SCREEN_BRIGHT_LEVELS	(2 * 4)	// have 5 distinct levels
	
	/* 
	 * lcdBright and lcdBacklight arrays describe a complete cycle of screen
	 * adjustment levels, so array index always increments, mod number of
	 * array entries.  First array entry is the middle of range; this is
	 * the default setting.
	 */
	S8 lcdBright[SCREEN_BRIGHT_LEVELS]    =
			{   0,   0,   0,   0,   0,   0,   0,   0};
	S8 lcdBacklight[SCREEN_BRIGHT_LEVELS] = 
			{ -11,  31,  72,  31, -11, -53, -94, -53};
	
	S8 brightness = lcdBright[0];
	S8 backlight  = lcdBacklight[0];
	int brightIndex = 1;				// index of next value to retrieve
	int sw = 0;
	U32 button;
	
	dbg.SetDebugLevel(kDbgLvlVerbose);
	
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	

	data.buttonState = 0;
	data.buttonTransition = 0;

	// get initial state of switches
	if(ioctl(button_fd, EVIOCGSW(sizeof(int)), &sw) == 0) {
		if(sw & (1<<SW_HEADPHONE_INSERT)) {
			data.buttonState |= kHeadphoneJackDetect;
			data.buttonTransition |= kHeadphoneJackDetect;
		}
		if(sw & (1<<SW_TABLET_MODE)) {
			data.buttonState |= kCartridgeDetect;
			data.buttonTransition |= kCartridgeDetect;
		}
		CButtonMessage msg(data);
		eventmgr.PostEvent(msg, kButtonEventPriority);
	}

	while(1) {

		// FIXME Pace thread at time intervals relavant for button presses
		kernel.TaskSleep(1);

		data.buttonTransition = 0;

		int num_events = read(button_fd, &ev, sizeof(ev));
		dbg.Assert( num_events >= 0, "CButtonModule::LightningButtonTask: button read failed" );

		// ev[n].value describes the transition:
		//  0 = released
		//  1 = pushed
		//  2 = repeat (ie: held down)
		//  Note: we do not support repeats but a button whose initial state is 2 should be
		//  treated as 'pushed' in order to not miss it.  This handles the case of a user
		//  pushing and holding a button before we initialize

		for(int i = 0; i < num_events; i++) {
			if(ev[i].type == EV_KEY) { // this is a key press
				button = LinuxKeyToBrio(ev[i].code);
				if(button == 0) {
					//dbg.DebugOut(kDbgLvlVerbose, "%s: unknown key code: %d type: %d\n", __FUNCTION__, ev[i].code, ev[i].type);
					continue;
				}
				//repeat of known-pressed button
				if(ev[i].value == 2 && (data.buttonState & button))
					continue;
			}
			else if(ev[i].type == EV_SW) { // this is a switch change
				button = LinuxSwitchToBrio(ev[i].code);
				if(button == 0) {
					dbg.DebugOut(kDbgLvlVerbose, "%s: unknown switch code\n", __FUNCTION__);
					continue;
				}
			}
			else {
				continue;
			}

			if(ev[i].value > 0 && (!(data.buttonState & button))) {
				data.buttonTransition |= button;
				data.buttonState |= button;
			}
			else if(ev[i].value == 0 && (data.buttonState & button)) {
				data.buttonTransition |= button;
				data.buttonState &= ~button;
			}
		}

		// Special internal handling for Brightness button
		if (data.buttonTransition & data.buttonState & kButtonBrightness)
		{
			brightness = lcdBright[brightIndex];
			backlight  = lcdBacklight[brightIndex];
			// wrap around index at array end
			brightIndex++;
			brightIndex = brightIndex % SCREEN_BRIGHT_LEVELS;
			dispmgr.SetBrightness(0, brightness);
			dispmgr.SetBacklight(0, backlight);
		}
		
		// Special internal handling for Headphone jack plug/unplug
		if (data.buttonTransition & kHeadphoneJackDetect)
		{
			CAudioMPI audioMPI;
			bool state_SpeakerEnabled = (0 == (data.buttonState & kHeadphoneJackDetect));
			audioMPI.EnableSpeakerDSP(state_SpeakerEnabled);
		}

		if(data.buttonTransition != 0) {
			CButtonMessage msg(data);
			eventmgr.PostEvent(msg, kButtonEventPriority);
		}
	}
	return NULL;
}

//----------------------------------------------------------------------------

void CButtonModule::InitModule()
{
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;
	
	// instantiate event manager, needed to resolve symbol
	// '_ZN8LeapFrog4Brio63_GLOBAL__N__ZNK8LeapFrog4Brio12CEventModule16
	// GetModuleVersionEv5pinstE'
	CEventMPI	eventmgr;
	
	dbg_.DebugOut(kDbgLvlVerbose, "Button Init\n");

	data.buttonState = 0;
	data.buttonTransition = 0;

	// Need valid file descriptor open before starting task thread 
	dev_name = GetKeyboardName();
	dbg_.Assert(dev_name != NULL, "CButtonModule::InitModule: no keyboard to use");
	button_fd = open(dev_name, O_RDONLY);
	dbg_.Assert(button_fd >= 0, "CButtonModule::InitModule: cannot open %s", dev_name);

	if( kernel.IsValid() )
	{
		tTaskProperties	properties;
		properties.pTaskMainArgValues = NULL;
		properties.TaskMainFcn = LightningButtonTask;
		status = kernel.CreateTask(handleButtonTask, properties);
	}
	dbg_.Assert( status == kNoErr, 
				"CButtonModule::InitModule: background task creation failed" );
}

//----------------------------------------------------------------------------
void CButtonModule::DeinitModule()
{
	CKernelMPI	kernel;

	dbg_.DebugOut(kDbgLvlVerbose, "ButtonModule::DeinitModule: Button Deinit\n");

	// Terminate button handler thread, and wait before closing driver
	kernel.CancelTask(handleButtonTask);
	kernel.TaskSleep(1);	
	close(button_fd);
}

//============================================================================
// Button state
//============================================================================
//----------------------------------------------------------------------------
tButtonData CButtonModule::GetButtonState() const
{
	return data;
}

LF_END_BRIO_NAMESPACE()
// EOF
