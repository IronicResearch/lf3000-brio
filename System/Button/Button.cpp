//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Button.cpp
//
// Description:
//		Implements the underlying Button Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#include <ButtonMPI.h>
#include <ButtonPriv.h>
#include <EmulationConfig.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>
LF_BEGIN_BRIO_NAMESPACE()


const CURI	kModuleURI	= "Button URI";

#define BIT(c, x)   ( c[x/8]&(1<<(x%8)) )


//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	const char* kDefaultDisplay = ":0";
	U32			gLastState;

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
// CButtonModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonModule::GetModuleVersion(tVersion &version) const
{
	version = kButtonModuleVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CButtonModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kButtonModuleName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CButtonModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
}

//============================================================================
// ButtonTask
//============================================================================
//----------------------------------------------------------------------------
// FIXME/tp: Move to emulation-only section/file
void* ButtonTask(void*)
{
	Display  *disp = XOpenDisplay(kDefaultDisplay);
	Window focus;
	int revert;

	const tEventPriority kPriorityTBD = 0;

printf("Started ButtonTask()\n");

	CEventMPI		eventmgr;
	tButtonData		data;
 	data.buttonState = data.buttonTransition = 0;

	Window win = (Window)EmulationConfig::Instance().GetLcdDisplayWindow();
	XSelectInput(disp, win, KeyPress | KeyRelease);
	for( bool bDone = false; !bDone; )	// FIXME/tp: when break out???
	{
		XEvent	event;
		XNextEvent(disp, &event);
		KeySym keysym = XLookupKeysym((XKeyEvent *) &event, 0);
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
			eventmgr.PostEvent(msg, kPriorityTBD);
		}
	}
}


//============================================================================
// Ctor & dtor
//============================================================================
CButtonModule::CButtonModule()
{
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;

	if( kernel.IsValid() )
	{
		const CURI		*pTaskURI = NULL;
		tTaskProperties	pProperties;
		pProperties.pTaskMainArgValues = NULL;
		tTaskHndl		*pHndl;
		pProperties.TaskMainFcn = ButtonTask;
		status = kernel.CreateTask( NULL, &pProperties, pHndl );	//FIXME/tp: param order
	}
	if( status != kNoErr )
		;// FIXME error message
}

//----------------------------------------------------------------------------
CButtonModule::~CButtonModule()
{
}

//----------------------------------------------------------------------------
Boolean	CButtonModule::IsValid() const
{
	return true;
}


//============================================================================
// Button state
//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonModule::GetButtonState(tButtonData& data) const
{
	data.buttonState = data.buttonTransition = 0;

	char keys[32];
	Display  *disp = XOpenDisplay(kDefaultDisplay);	
	XQueryKeymap(disp, keys);
	
	for (int i=0; i<32*8; i++)
	{
    	if (BIT(keys, i))
		{
	     	KeySym keysym = XKeycodeToKeysym(disp, i, 0);
	     	if( keysym )
	    	 	data.buttonState |= KeySymToButton(keysym);
		}
	}
	data.buttonTransition = data.buttonState ^ gLastState;
	gLastState = data.buttonState;
	
	
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================

LF_USING_BRIO_NAMESPACE()

static CButtonModule*	sinst = NULL;
//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance(tVersion version)
{
	if( sinst == NULL )
		sinst = new CButtonModule;
	return sinst;
}
	
//------------------------------------------------------------------------
extern "C" void DestroyInstance(ICoreModule* ptr)
{
//		assert(ptr == sinst);
	delete sinst;
	sinst = NULL;
}


// EOF
