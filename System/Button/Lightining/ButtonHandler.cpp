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
LF_BEGIN_BRIO_NAMESPACE()



//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	U32			gLastState;
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
static void* TempButtonTask(void*)
{
	CDebugMPI	dbg(kGroupButton);
	dbg.DebugOut(kDbgLvlVerbose, "TempButtonTask: Started\n");

	CEventMPI		eventmgr;
	tButtonData		data;
 	data.buttonState = data.buttonTransition = 0;

	for( bool bDone = false; !bDone; )	// FIXME/tp: when break out???
	{
;//			CButtonMessage	msg(data);
//			eventmgr.PostEvent(msg, kButtonEventPriority);
	}
	return NULL;
}

//----------------------------------------------------------------------------
void CButtonModule::InitModule()
{
	//TODO: Setup interrupt handler here
}


//============================================================================
// Button state
//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonModule::GetButtonState(tButtonData& data) const
{
	data.buttonState = data.buttonTransition = 0;
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()
// EOF
