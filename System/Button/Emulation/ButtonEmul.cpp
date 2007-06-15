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
			case XK_Left:			return kButtonLeft;
			case XK_KP_Left:		return kButtonLeft;
			case XK_Right:			return kButtonRight;
			case XK_KP_Right:		return kButtonRight;
			case XK_Up:				return kButtonUp;
			case XK_KP_Up:			return kButtonUp;
			case XK_Down:			return kButtonDown;
			case XK_KP_Down:		return kButtonDown;
			
			case XK_a:				return kButtonA;
			case XK_KP_0:			return kButtonA;
			
			case XK_b:				return kButtonB;
			case XK_KP_1:			return kButtonB;
			case XK_KP_Decimal:		return kButtonB;
			
			case XK_Delete:			return kButtonLeftShoulder;
//			case XK_KP_Delete:		return kButtonLeftShoulder;
			
			case XK_Page_Down:		return kButtonRightShoulder;
//			case XK_KP_Page_Down:	return kButtonRightShoulder;

			case XK_m:				return kButtonMenu;
			case XK_Insert:			return kButtonMenu;
			case XK_KP_Insert:		return kButtonMenu;

			case XK_h:				return kButtonHint;
			case XK_Home:			return kButtonHint;
			case XK_KP_Home:		return kButtonHint;

			case XK_p:				return kButtonPause;
			case XK_Page_Up:		return kButtonPause;
			case XK_KP_Page_Up:		return kButtonPause;
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

//----------------------------------------------------------------------------
void CButtonModule::DeinitModule()
{
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
