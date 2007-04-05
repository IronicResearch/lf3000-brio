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
//
//============================================================================
#include <stdio.h>

#include <ButtonMPI.h>
#include <ButtonPriv.h>
#include <EmulationConfig.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>

const CURI	kModuleURI	= "Button FIXME";


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
	const tEventPriority kPriorityTBD = 0;
//	WINDOW* win = (WINDOW*)LeapFrog::Brio::EmulationConfig::Instance().GetLcdDisplayWindow();
//	CDebugMPI	debug;
//	debug.
printf("Started ButtonTask()\n");
/*
	sleep(3);
	cbreak();				// Line buffering disabled, pass everything to me
	keypad(win, TRUE);	// I need that nifty F1

	CEventMPI		event;
	tButtonData		d;
	int				ch;
    while((ch = wgetch(win)) != KEY_F(1))
    {
    	printf(".");
    	switch( ch )
    	{
			case KEY_LEFT:
				d.buttonState = d.buttonTransition = kButtonLeftKey;
				break;
			case KEY_RIGHT:
				d.buttonState = d.buttonTransition = kButtonRightKey;
				break;
			case KEY_UP:
				d.buttonState = d.buttonTransition = kButtonUpKey;
				break;
			case KEY_DOWN:
				d.buttonState = d.buttonTransition = kButtonDownKey;
				break;
			case 'a':
			case KEY_IL:
				d.buttonState = d.buttonTransition = kButtonAKey;
				break;
			case 'b':
			case KEY_DL:
				d.buttonState = d.buttonTransition = kButtonBKey;
				break;
			default:
 			   	d.buttonState = d.buttonTransition = 0;
				break;
		}
    	if( d.buttonState != 0 )
    	{
    		printf("Posting button event!\n");
			CButtonMessage	msg(d);
			event.PostEvent(msg, kPriorityTBD);
    	}
    }
    	*/
}


//============================================================================
// Ctor & dtor
//============================================================================
CButtonModule::CButtonModule()
{
	/*
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;

//FIXME/tp: typedef for casting
	typedef void* (*tTaskMainFcnPtr)(void*);
	if( kernel.IsValid() )
	{
		const CURI		*pTaskURI = NULL;
		tTaskProperties	pProperties = {0};
		thread_arg_t	threadArg;
		pProperties.pTaskMainArgValues = NULL;
		tTaskHndl		*pHndl;
		pProperties.TaskMainFcn = ButtonTask;
		status = kernel.CreateTask( NULL, &pProperties, pHndl );	//FIXME/tp: param order
	}
	if( status != kNoErr )
		;// FIXME error message
	*/
}
//static_cast<tTaskMainFcnPtr>
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
tErrType CButtonModule::GetButtonState(tButtonData& data)
{
	data.buttonState = 0;
	data.buttonTransition = 0;
	return kNoErr;
}


//============================================================================
// Instance management interface for the Module Manager
//============================================================================

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
