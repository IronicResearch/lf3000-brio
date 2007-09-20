//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PowerEmul.cpp
//
// Description:
//		Implements the underlying Power Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <X11/X.h>
#include <X11/Xlib.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#include <PowerPriv.h>
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
	U32 KeySymToPower( KeySym keysym )
	{
		switch( keysym )
		{
			case XK_F1:		return kPowerExternal;
			case XK_F2:		return kPowerBattery;
			case XK_F3:		return kPowerLowBattery;
			case XK_F4:		return kPowerShutdown;
		}
		return kPowerNull;
	}
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void* EmulationPowerTask(void*)
{
	if( gXDisplay == NULL )
		return NULL;

	CDebugMPI	dbg(kGroupPower);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPowerTask: Started\n");

	CEventMPI			eventmgr;
	tPowerData			data;
	data.powerState = kPowerNull;

	U32 dw = EmulationConfig::Instance().GetLcdDisplayWindow();
	if (dw == 0)
	{
		dbg.DebugOut(kDbgLvlImportant, 
				"EmulationPowerTask: no display window created, disabling power emulation\n");
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
		U32 mask = KeySymToPower(keysym);
		gLastState = data.powerState;
		CPowerMessage	msg(data);
		eventmgr.PostEvent(msg, kPowerEventPriority);
	}
	XAutoRepeatOn(gXDisplay);
	return NULL;
}

//----------------------------------------------------------------------------
void CPowerModule::InitModule()
{
	if( !kInUnitTest ) 
	{
		gXDisplay = XOpenDisplay(kDefaultDisplay);
		if (gXDisplay == NULL)
			dbg_.DebugOut(kDbgLvlCritical, "CPowerModule::InitModule(): Emulation XOpenDisplay() failed, power emulation disabled!\n");
		XAutoRepeatOff(gXDisplay);
		tErrType	status = kModuleLoadFail;
		CKernelMPI	kernel;
	
		if( kernel.IsValid() )
		{
			tTaskHndl	handle;
			tTaskProperties	properties;
			properties.pTaskMainArgValues = NULL;
			properties.TaskMainFcn = EmulationPowerTask;
			status = kernel.CreateTask(handle, properties);
		}
		dbg_.Assert( status == kNoErr, 
					"CPowerModule::InitModule(): Power Emulation InitModule: background task creation failed" );
	}
}

//----------------------------------------------------------------------------
void CPowerModule::DeinitModule()
{
	gDone = true;
	XAutoRepeatOn(gXDisplay);
}

//============================================================================
// Power state
//============================================================================
//----------------------------------------------------------------------------
enum tPowerState CPowerModule::GetPowerState() const
{
	enum tPowerState state = kPowerNull;

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
		     		state = (tPowerState)KeySymToPower(keysym);
			}
		}
		gLastState = state;
	}
	return state;
}


//----------------------------------------------------------------------------
int CPowerModule::GetConserve() const
{
	enum tPowerState state = kPowerNull;

	CDebugMPI	dbg(kGroupPower);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPowerTask: GetConserve() not implemented.\n");
	
	return state;
}


//----------------------------------------------------------------------------
int CPowerModule::SetConserve(bool bConserve) const
{
	enum tPowerState state = kPowerNull;

	CDebugMPI	dbg(kGroupPower);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPowerTask: SetConserve() not implemented.\n");
	
	return state;
}


//----------------------------------------------------------------------------
int CPowerModule::Shutdown() const
{
	enum tPowerState state = kPowerNull;

	CDebugMPI	dbg(kGroupPower);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPowerTask: Shutdown() not implemented.\n");
	
	return state;
}


//----------------------------------------------------------------------------
int CPowerModule::GetShutdownTimeMS() const
{
	enum tPowerState state = kPowerNull;

	CDebugMPI	dbg(kGroupPower);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPowerTask: Shutdown() not implemented.\n");
	
	return state;
}

//----------------------------------------------------------------------------
int CPowerModule::SetShutdownTimeMS(int milliSeconds) const
{
	enum tPowerState state = kPowerNull;

	CDebugMPI	dbg(kGroupPower);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPowerTask: SetShutdownTimeMS() not implemented.\n");
	
	return state;
}

//----------------------------------------------------------------------------
int CPowerModule::Reset() const
{
	CDebugMPI	dbg(kGroupPower);
	dbg.DebugOut(kDbgLvlVerbose, "EmulationPowerTask: Reset() not implemented.\n");
	
	return -1;
}
LF_END_BRIO_NAMESPACE()
// EOF
