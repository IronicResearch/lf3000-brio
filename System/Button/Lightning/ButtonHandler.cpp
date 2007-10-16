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
	int			brightness = dispmgr.GetBrightness(0);
	const int	kBrightInc = 8;
	const int	kBrightMax = 127 - kBrightInc;
	const int	kBrightMin = -128 + kBrightInc;
	bool		isBrightening = (brightness < kBrightMax) ? true : false;
	
	dbg.SetDebugLevel(kDbgLvlVerbose);
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
			if (isBrightening)
			{
				brightness += kBrightInc;
				if (brightness >= kBrightMax)
					isBrightening = false;
			}
			else
			{
				brightness -= kBrightInc;
				if (brightness <= kBrightMin)
					isBrightening = true;
			}
			dispmgr.SetBrightness(0, brightness);
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
