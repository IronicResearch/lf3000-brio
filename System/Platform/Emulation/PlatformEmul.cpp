//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PlatformEmul.cpp
//
// Description:
//		Implements the underlying Platform Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <X11/X.h>
#include <X11/Xlib.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#include <PlatformPriv.h>
#include <EmulationConfig.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>
LF_BEGIN_BRIO_NAMESPACE()


#define BIT(c, x)   ( c[x/8]&(1<<(x%8)) )


//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	const char* kDefaultDisplay = ":0";
	U32			gLastState;
	Display*	gXDisplay = NULL;
	bool		gDone = false;

	//------------------------------------------------------------------------
	U32 KeySymToPlatform( KeySym keysym )
	{
		switch( keysym )
		{
			case XK_F1:		return kPlatformUsbConnect;
			case XK_F2:		return kPlatformUsbDisconnect;
		}
		return kPlatformNull;
	}
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void* EmulationPlatformTask(void*)
{
	if( gXDisplay == NULL )
		return NULL;

	CDebugMPI	dbg(kGroupPlatform);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPlatformTask: Started\n");

	CEventMPI			eventmgr;
	tPlatformData			data;
	data.platformState = kPlatformNull;

	U32 dw = EmulationConfig::Instance().GetLcdDisplayWindow();
	if (dw == 0)
	{
		dbg.DebugOut(kDbgLvlImportant, 
				"EmulationPlatformTask: no display window created, disabling platform emulation\n");
		XAutoRepeatOn(gXDisplay);
		return NULL;
	}
	Window win = static_cast<Window>(dw);
	
	XSelectInput(gXDisplay, win, KeyPress | KeyRelease | ClientMessage);
	while( !gDone )
	{
		XEvent	event;
		XNextEvent(gXDisplay, &event);
		KeySym keysym = XLookupKeysym(reinterpret_cast<XKeyEvent *>(&event), 0);
		U32 mask = KeySymToPlatform(keysym);
		gLastState = data.platformState;
		CPlatformMessage	msg(data);
		eventmgr.PostEvent(msg, kPlatformEventPriority);
	}
	XAutoRepeatOn(gXDisplay);
	return NULL;
}

//----------------------------------------------------------------------------
void CPlatformModule::InitModule()
{
	if( !kInUnitTest ) 
	{
		gXDisplay = XOpenDisplay(kDefaultDisplay);
		if (gXDisplay == NULL)
			dbg_.DebugOut(kDbgLvlCritical, "CPlatformModule::InitModule(): Emulation XOpenDisplay() failed, platform emulation disabled!\n");
		XAutoRepeatOff(gXDisplay);
		tErrType	status = kModuleLoadFail;
		CKernelMPI	kernel;
	
		if( kernel.IsValid() )
		{
			tTaskHndl	handle;
			tTaskProperties	properties;
			properties.pTaskMainArgValues = NULL;
			properties.TaskMainFcn = EmulationPlatformTask;
			status = kernel.CreateTask(handle, properties);
		}
		dbg_.Assert( status == kNoErr, 
					"CPlatformModule::InitModule(): Platform Emulation InitModule: background task creation failed" );
	}
}

//----------------------------------------------------------------------------
void CPlatformModule::DeinitModule()
{
	gDone = true;
	XAutoRepeatOn(gXDisplay);
}

//============================================================================
// Platform state
//============================================================================
//----------------------------------------------------------------------------
tPlatformData CPlatformModule::GetPlatformState() const
{
	tPlatformData data = { kPlatformNull } ;

	if( gXDisplay != NULL )
	{
		char keys[32];
		XQueryKeymap(gXDisplay, keys);
		
		for (int i=0; i<32*8; i++)
		{
	    	if (BIT(keys, i))
			{
		     	KeySym keysym = XKeycodeToKeysym(gXDisplay, i, 0);
		     	if( keysym )
		    	 	data.platformState = KeySymToPlatform(keysym);
			}
		}
		gLastState = data.platformState;
	}
	return data;
}

LF_END_BRIO_NAMESPACE()
// EOF
