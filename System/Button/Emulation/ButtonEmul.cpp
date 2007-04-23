//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		ButtonEmul.cpp
//
// Description:
//		Implements the underlying Button Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <X11/X.h>
#include <X11/Xlib.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#include <ButtonPriv.h>
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
	Display *gXDisplay = NULL;

	//------------------------------------------------------------------------
	U32 KeySymToButton( KeySym keysym )
	{
		switch( keysym )
		{
			case XK_Left:			return kButtonLeftKey;
			case XK_KP_Left:		return kButtonLeftKey;
			case XK_Right:			return kButtonRightKey;
			case XK_KP_Right:		return kButtonRightKey;
			case XK_Up:				return kButtonUpKey;
			case XK_KP_Up:			return kButtonUpKey;
			case XK_Down:			return kButtonDownKey;
			case XK_KP_Down:		return kButtonDownKey;
			case XK_a:				return kButtonAKey;
			case XK_KP_Insert:		return kButtonAKey;
			case XK_KP_0:			return kButtonAKey;
			case XK_b:				return kButtonBKey;
			case XK_KP_Delete:		return kButtonBKey;
			case XK_KP_Decimal:		return kButtonBKey;
			case XK_KP_Add:			return kButtonVolumeUp;
			case XK_KP_Subtract:	return kButtonVolumeDown;
		}
		return 0;
	}
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void* EmulationButtonTask(void*)
{
	if( gXDisplay == NULL )
		return NULL;

	CDebugMPI	dbg(kGroupButton);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationButtonTask: Started\n");

	Window focus;
	int revert;

	CEventMPI		eventmgr;
	tButtonData		data;
 	data.buttonState = data.buttonTransition = 0;

	Window win = (Window)EmulationConfig::Instance().GetLcdDisplayWindow();
	XSelectInput(gXDisplay, win, KeyPress | KeyRelease);
	for( bool bDone = false; !bDone; )	// FIXME/tp: when break out???
	{
		XEvent	event;
		XNextEvent(gXDisplay, &event);
		KeySym keysym = XLookupKeysym(reinterpret_cast<XKeyEvent *>(&event), 0);
		U32 mask = KeySymToButton(keysym);
		data.buttonTransition = mask;
		if( mask != 0 )
		{
			if( event.type == KeyPress )
				data.buttonState |= mask;
			else
				data.buttonState &= ~mask;
			gLastState = data.buttonState;
			CButtonMessage	msg(data);
			eventmgr.PostEvent(msg, kButtonEventPriority);
		}
	}
	return NULL;
}

//----------------------------------------------------------------------------
void CButtonModule::InitModule()
{
	if( !kInUnitTest ) 
	{
		gXDisplay = XOpenDisplay(kDefaultDisplay);
		if (gXDisplay == NULL)
			dbg_.DebugOut(kDbgLvlCritical, "Emulation XOpenDisplay() failed, buttons disabled!\n");
		tErrType	status = kModuleLoadFail;
		CKernelMPI	kernel;
	
		if( kernel.IsValid() )
		{
			tTaskHndl		handle;
			tTaskProperties	properties;
			properties.pTaskMainArgValues = NULL;
			properties.TaskMainFcn = EmulationButtonTask;
			status = kernel.CreateTask(handle, properties);
		}
		dbg_.Assert( status == kNoErr, 
					"Button Emulation InitModule: background task creation failed" );
	}
}


//============================================================================
// Button state
//============================================================================
//----------------------------------------------------------------------------
tButtonData CButtonModule::GetButtonState() const
{
	tButtonData	data = { 0, 0 };

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
		    	 	data.buttonState |= KeySymToButton(keysym);
			}
		}
		data.buttonTransition = data.buttonState ^ gLastState;
		gLastState = data.buttonState;
	}
	return data;
}

LF_END_BRIO_NAMESPACE()
// EOF
