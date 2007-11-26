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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/lf1000/buttons.h>

LF_BEGIN_BRIO_NAMESPACE()



//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	U32					gLastState;
	int 				button_fd;
	struct button_event current_be;
	tTaskHndl			handleButtonTask;
	tButtonData	data;
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void *LightningButtonTask(void*)
{
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
			{ -11,  30,  72,  30, -11, -52, -93, -52};
	
	S8 brightness = lcdBright[0];
	S8 backlight  = lcdBacklight[0];
	int brightIndex = 1;				// index of next value to retrieve
	
#if !defined SET_DEBUG_LEVEL_DISABLE
	dbg.SetDebugLevel(kDbgLvlVerbose);
#endif
	
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {

		// Pace thread at time intervals relavant for button presses
		kernel.TaskSleep(1);

		int size = read(button_fd, &current_be, sizeof(struct button_event));
		dbg.Assert( size >= 0, "CButtonModule::LightningButtonTask: button read failed" );
		
		data.buttonState = current_be.button_state;
		data.buttonTransition = current_be.button_trans;
		CButtonMessage msg(data);
		eventmgr.PostEvent(msg, kButtonEventPriority);
		
		// Special internal handling for Brightness button
		if (data.buttonTransition & data.buttonState & kButtonBrightnessKey)
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
//printf("ButtonHandler: buttonState=$%X buttonTransition=%X kHeadphoneJackDetect=$%X \n", 
 //   (unsigned int) data.buttonState, (unsigned int) data.buttonTransition, (unsigned int)kHeadphoneJackDetect);
		if (data.buttonTransition & kHeadphoneJackDetect)
		{
        CAudioMPI audioMPI;
        bool state_SpeakerEnabled = (0 == (data.buttonState & kHeadphoneJackDetect));

//printf("ButtonHandler: kHeadphoneJackDetect state_SpeakerEnabled = %d\n", state_SpeakerEnabled);
        audioMPI.SetSpeakerEqualizer(state_SpeakerEnabled);
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
	button_fd = open( "/dev/buttons", B_O_RDWR);
	dbg_.Assert(button_fd != -1, "CButtonModule::InitModule: cannot open /dev/buttons");

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
