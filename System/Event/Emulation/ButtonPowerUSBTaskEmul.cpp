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
#include <AccelerometerTypes.h>

// linux
#include <linux/input.h>
#include <poll.h>

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
	const tEventPriority	kAccelerometerEventPriority	= 255;
	
	volatile bool 			g_threadRun2_ = true;
	volatile bool 			g_threadRunning2_ = false;

	//Maximum number of input drivers to discover
	#define NUM_INPUTS	2

	#define INPUT_KEYBOARD		"gpio-keys"
	#define INPUT_ACLMTR		"Accelerometer"

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

	//----------------------------------------------------------------------------
	// Linux keyboard to Brio button mapping
	//----------------------------------------------------------------------------
	U32 LinuxKeyToBrio(U16 code)
	{
		switch(code) {
			case KEY_UP:		return kButtonUp;
			case KEY_DOWN:		return kButtonDown;
			case KEY_RIGHT:		return kButtonRight;
			case KEY_LEFT:		return kButtonLeft;
			case KEY_A:			return kButtonA;
			case KEY_B:			return kButtonB;
			case KEY_L:			return kButtonLeftShoulder;
			case KEY_R:			return kButtonRightShoulder;
			case KEY_M:			return kButtonMenu;
			case KEY_H:			return kButtonHint;
			case KEY_P:			return kButtonPause;
			case KEY_X:			return kButtonBrightness;
			case KEY_VOLUMEDOWN: return kButtonVolumeDown;
			case KEY_VOLUMEUP:	return kButtonVolumeUp;
			case KEY_ESC:		return kButtonEscape;
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
	struct pollfd	event_fd[NUM_INPUTS];
 
	// Init accelerometer driver and data
	int aclmtr_index = 0;
	tAccelerometerData aclmtr_data = {0, 0, 0, {0, 0}};
	event_fd[aclmtr_index].fd = open_input_device(INPUT_ACLMTR);
	event_fd[aclmtr_index].events = POLLIN;
	if (event_fd[aclmtr_index].fd < 0 )
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open: %s\n", INPUT_ACLMTR);
	}

	int button_index = 1;
	tButtonData2 button_data;
	button_data.buttonState = 0;
	button_data.buttonTransition = 0;
	event_fd[button_index].fd = open_input_device(INPUT_KEYBOARD);
	event_fd[button_index].events = POLLIN;
	if (event_fd[button_index].fd < 0 )
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open: %s\n", INPUT_KEYBOARD);
	}





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
	int ret;
	struct input_event ev;
	U32 button;
	int size = 0;
	while (pThis->bThreadRun_)
	{
		// block on driver state changes only
		// use timeout for cancellation point
		ret = poll(event_fd, NUM_INPUTS, 1000);
		pthread_testcancel();

		if( ret >= 0 ) {
			// button driver event?
			if(button_index >= 0 && event_fd[button_index].revents & POLLIN) {
				button_data.buttonTransition = 0;
				size = read(event_fd[button_index].fd, &ev, sizeof(ev));
				for(int i = 0; i < size; i++) {
					if(ev.type == EV_KEY) { // this is a key press
						button = LinuxKeyToBrio(ev.code);
						if(button == 0) {
							pThis->debug_.DebugOut(kDbgLvlValuable, "%s: unknown key code: %d type: %d\n", __FUNCTION__, ev.code, ev.type);
							continue;
						}
						//repeat of known-pressed button
						if(ev.value == 2 && (button_data.buttonState & button))
							continue;
					}
					else if(ev.type == EV_SW) { // this is a switch change
						// not implemented for emulator
					}
					else {
						continue;
					}

					if(ev.value > 0 && (!(button_data.buttonState & button))) {
						button_data.buttonTransition |= button;
						button_data.buttonState |= button;
					}
					else if(ev.value == 0 && (button_data.buttonState & button)) {
						button_data.buttonTransition |= button;
						button_data.buttonState &= ~button;
					}
				}

				if(button_data.buttonTransition != 0) {
					button_data.time.seconds      = ev.time.tv_sec;
					button_data.time.microSeconds = ev.time.tv_usec;
					CButtonMessage button_msg(button_data, true);
					pThis->PostEvent(button_msg, kButtonEventPriority, 0);
				}
			}

			// Accelerometer driver event?
			if (aclmtr_index >= 0 && event_fd[aclmtr_index].revents & POLLIN)
			{
				read(event_fd[aclmtr_index].fd, &ev, sizeof(ev));

				switch (ev.type) {
				case EV_ABS:
					switch (ev.code) {
					case ABS_X:
						aclmtr_data.accelX = ev.value;
						break;
					case ABS_Y:
						aclmtr_data.accelY = ev.value;
						break;
					case ABS_Z:
						aclmtr_data.accelZ = ev.value;
						break;
					case ABS_MISC: // orientation change
						CAccelerometerMessage aclmtr_msg(ev.value);
						pThis->PostEvent(aclmtr_msg, kAccelerometerEventPriority, 0);
						break;
					}
					break;
				case EV_SYN:
					aclmtr_data.time.seconds       = ev.time.tv_sec;
					aclmtr_data.time.microSeconds  = ev.time.tv_usec;
					CAccelerometerMessage aclmtr_msg(aclmtr_data);
					pThis->PostEvent(aclmtr_msg, kAccelerometerEventPriority, 0);
					break;
				}
			}

		}


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
						XAutoRepeatOn(gXDisplay);
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
					XAutoRepeatOn(gXDisplay);
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

//----------------------------------------------------------------------------

//============================================================================
// Platform-specific initializer
//============================================================================

void CEventModule::InitModule(void)
{

}

//----------------------------------------------------------------------------
void CEventModule::DeInitModule(void)
{

}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()
// EOF
