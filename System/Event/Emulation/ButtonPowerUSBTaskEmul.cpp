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

#include <EventPriv.h>
#include <EmulationConfig.h>
#include <CoreModule.h>
#include <DebugMPI.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>
#include <Utility.h>
#include <TouchTypes.h>
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
	
	const tEventPriority	kButtonEventPriority	= 0;
	const tEventPriority	kTouchEventPriority	= 0;
	const tEventPriority	kPowerEventPriority	= 0;
	
	volatile bool 			g_threadRun2_ = true;
	volatile bool 			g_threadRunning2_ = false;

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
			
			case XK_l:				return kButtonLeftShoulder;
			case XK_Delete:			return kButtonLeftShoulder;
//			case XK_KP_Delete:		return kButtonLeftShoulder;
			
			case XK_r:				return kButtonRightShoulder;
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
			case XK_Pause:			return kButtonPause;
		}
		return 0;
	}
}

void* CEventModule::CartridgeTask( void* arg )
{
	CEventModule*	pThis = reinterpret_cast<CEventModule*>(arg);
	while (pThis->bThreadRun_)
		pThis->kernel_.TaskSleep(10);
	return NULL;
}

//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void* CEventModule::ButtonPowerUSBTask(void* arg)
{
	CEventModule*	pThis = reinterpret_cast<CEventModule*>(arg);

	gXDisplay = XOpenDisplay(kDefaultDisplay);
	if (gXDisplay == NULL)
		pThis->debug_.Assert(kDbgLvlCritical, "CEventModule::ButtonPowerUSBTask(): Emulation XOpenDisplay() failed, buttons disabled!\n");
	XAutoRepeatOff(gXDisplay);

	tButtonData2		data;
 	data.buttonState = data.buttonTransition = 0;

	U32 dw = EmulationConfig::Instance().GetLcdDisplayWindow();
	while(!dw && pThis->bThreadRun_)
	{
		pThis->kernel_.TaskSleep(10);
		dw = EmulationConfig::Instance().GetLcdDisplayWindow();
	}
	Window win = static_cast<Window>(dw);
	
	struct tPowerData	power_data;
	power_data.powerState = GetCurrentPowerState();

	// Specify event mask here, not XSetWindowAttributes()
	if (win)
		XSelectInput(gXDisplay, win, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | StructureNotifyMask );
	g_threadRunning2_ = true;
	while (pThis->bThreadRun_)
	{
		XEvent	event;
		while(XPending(gXDisplay))
		{
			XNextEvent(gXDisplay, &event);
			switch(event.type)
			{
			case KeyPress:
			case KeyRelease:
				{
					KeySym keysym = XLookupKeysym(reinterpret_cast<XKeyEvent *>(&event), 0);
					if(keysym == XK_Escape)
					{
						power_data.powerState = kPowerShutdown;
						CPowerMessage power_msg(power_data);
						pThis->PostEvent(power_msg, kPowerEventPriority, 0);
						g_threadRun2_ = false;
					}
					U32 mask = KeySymToButton(keysym);
					data.buttonTransition = mask;
					if( event.type == KeyPress )
						data.buttonState |= mask;
					else
						data.buttonState &= ~mask;
					gLastState = data.buttonState;
					SetButtonState(data);
					CButtonMessage	msg(data);
					pThis->PostEvent(msg, kButtonEventPriority, 0);
				}
				break;
			case ButtonPress:
			case ButtonRelease:
			case MotionNotify:
				{
					tTouchData		td;
					td.touchState	= (event.type != ButtonRelease) ? 1 : 0;
					td.touchX		= event.xbutton.x;
					td.touchY		= event.xbutton.y;
					CTouchMessage	msg(td);
					pThis->PostEvent(msg, kTouchEventPriority, 0);
				}
				break;
				
			case DestroyNotify:
				{
					//printf("DestroyNotify Event %d\n", event.type);
					power_data.powerState = kPowerShutdown;
					CPowerMessage power_msg(power_data);
					pThis->PostEvent(power_msg, kPowerEventPriority, 0);
					g_threadRun2_ = false;
				}
				break;
				
			//default:
			//	printf("Unknown Event %d\n", event.type);
			}
		}
		pThis->kernel_.TaskSleep(10);
	}
	XAutoRepeatOn(gXDisplay);
	XCloseDisplay(gXDisplay);
	
	g_threadRunning2_ = false;
	return NULL;
}

LF_END_BRIO_NAMESPACE()
// EOF
