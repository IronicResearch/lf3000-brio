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

#include "buttons.h"

LF_BEGIN_BRIO_NAMESPACE()



//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	U32			gLastState;
	int button_fd;
	struct button_event current_be;
	CDebugMPI	dbg(kGroupButton);
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
static void* LighteningButtonTask(void*)
{
	CEventMPI eventmgr;
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {

		int size = read(button_fd, &current_be, sizeof(struct button_event));
		dbg.DebugOut(kDbgLvlVerbose, "Read data.\n");
		dbg.Assert( size >= 0, "button read failed" );
		
		tButtonData	data = { current_be.button, current_be.type };		
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
	dbg.SetDebugLevel(kDbgLvlVerbose);

	if( kernel.IsValid() )
	{
		tTaskHndl handle;
		tTaskProperties	properties;
		properties.pTaskMainArgValues = NULL;
		properties.TaskMainFcn = LighteningButtonTask;
		status = kernel.CreateTask(handle, properties);
	}
	dbg.Assert( status == kNoErr, 
				"Lightening Button InitModule: background task creation failed" );


	button_fd = open( "/dev/gpio", O_RDWR);
	dbg.Assert(button_fd != -1, "Lightening Button: cannot open /dev/gpio");

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
