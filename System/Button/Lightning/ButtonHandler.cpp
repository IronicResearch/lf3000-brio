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
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>

#include <stdio.h>
#include <unistd.h>

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
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
static void* LighteningButtonTask(void*)
{
	CEventMPI 	eventmgr;
	CDebugMPI	dbg(kGroupButton);
	CKernelMPI	kernel;

	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {

		// Pace thread at time intervals relavant for button presses
		kernel.TaskSleep(1);

		int size = read(button_fd, &current_be, sizeof(struct button_event));
		dbg.Assert( size >= 0, "button read failed" );
		
		tButtonData	data = { current_be.button_state, current_be.button_trans };
		CButtonMessage msg(data);
		eventmgr.PostEvent(msg, kButtonEventPriority);

	}
	return NULL;
}

//----------------------------------------------------------------------------
void CButtonModule::InitModule()
{
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;

	dbg_.DebugOut(kDbgLvlVerbose, "Button Init\n");

	// Need valid file descriptor open before starting task thread 
	button_fd = open( "/dev/gpio", O_RDWR);
	dbg_.Assert(button_fd != -1, "Lightening Button: cannot open /dev/gpio");

	if( kernel.IsValid() )
	{
		tTaskProperties	properties;
		properties.pTaskMainArgValues = NULL;
		properties.TaskMainFcn = LighteningButtonTask;
		status = kernel.CreateTask(handleButtonTask, properties);
	}
	dbg_.Assert( status == kNoErr, 
				"Lightening Button InitModule: background task creation failed" );
}

//----------------------------------------------------------------------------
void CButtonModule::DeinitModule()
{
	CKernelMPI	kernel;

	dbg_.DebugOut(kDbgLvlVerbose, "Button Deinit\n");

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
	tButtonData	data = { 0, 0 };
	data.buttonState = data.buttonTransition = 0;
	return data;
}

LF_END_BRIO_NAMESPACE()
// EOF
