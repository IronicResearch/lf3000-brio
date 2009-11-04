//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		ButtonPowerUSBTask.cpp
//
// Description:
//		Implements the consolidated Button, Power, USBDevice task.
//
//============================================================================

#include <SystemTypes.h>
#include <StringTypes.h>
#include <SystemErrors.h>

#include <KernelMPI.h>
#include <DebugMPI.h>
#include <EventListener.h>
#include <EventListenerPriv.h>
#include <EventMessage.h>
#include <EventMPI.h>
#include <EventPriv.h>
#include <PowerMPI.h>

#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/prctl.h>

#undef __STRICT_ANSI__
#include <linux/input.h>
#include <ButtonTypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <poll.h>
#include <PowerTypes.h>
#include <USBDeviceTypes.h>
#include <Utility.h>
#include <TouchTypes.h> 
#include <errno.h>



LF_BEGIN_BRIO_NAMESPACE() 

// Maximum cartridge x icons allowed before exiting AppManager
#define MAX_CART_X_ICONS	3

//Maximum number of input drivers to discover
#define NUM_INPUTS	5

namespace
{
	//============================================================================
	// Local variables and defines
	//============================================================================

	/* socket on which monitord reports USB events */
	#define USB_SOCK        "/tmp/usb_events_socket"
	/* socket on which monitord reports Power events */
	#define POWER_SOCK      "/tmp/power_events_socket"
	
	#define EVENT_POWER		1
	#define EVENT_BATTERY 	2
	
	#define APP_MSG_GET_USB         0
	#define APP_MSG_SET_USB         1
	#define APP_MSG_GET_POWER       2
	#define APP_MSG_SET_POWER       3
	
	#define SYSFS_LUN0_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun0/enabled"
	#define SYSFS_LUN1_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun1/enabled"
	#define SYSFS_VBUS_PATH "/sys/devices/platform/lf1000-usbgadget/vbus"
	#define SYSFS_WD_PATH 	"/sys/devices/platform/lf1000-usbgadget/watchdog_seconds"
	#define FLAGS_VBUS_PATH "/flags/vbus"
	
	const tEventPriority	kButtonEventPriority	= 0;
	const tEventPriority	kPowerEventPriority	= 0;
	const tEventPriority	kUSBDeviceEventPriority	= 0;
	const tEventPriority	kUSBSocketEventPriority	= 128;
	const tEventPriority	kTouchEventPriority	= 0;
	const tEventPriority	kCartridgeEventPriority	= 0;	

	volatile bool 			g_threadRun2_ = true;
	volatile bool 			g_threadRunning2_ = false;


	//============================================================================
	// Local state and utility functions
	//============================================================================
	
	#define MAX_DEVNODES	8
	
	//----------------------------------------------------------------------------
	// find and open an input device by name
	//----------------------------------------------------------------------------
	int open_input_device(char *input_name)
	{
		char dev[20];
		char name[32];
		int fd, i;
	
		for(i = 0; i < MAX_DEVNODES; i++) {
			sprintf(dev, "/dev/input/event%d", i);
			fd = open(dev, O_RDONLY);
			if(fd < 0) {
				return 1;
			}
	
			if(ioctl(fd, EVIOCGNAME(32), name) < 0) {
				close(fd);
				return 1;
			}
	
			if(!strcmp(name, input_name)) {
				return fd;
			}
			else { /* not what we want, check another */
				close(fd);
				fd = -1;
			}
		}
		
		return -1;
	}
	
	//----------------------------------------------------------------------------
	// Linux keyboard to Brio button mapping
	//----------------------------------------------------------------------------
	U32 LinuxKeyToBrio(U16 code)
	{
		switch(code) {
			case KEY_UP: 		return kButtonUp;
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
		}
		return 0;
	}
	
	//----------------------------------------------------------------------------
	// Linux keyboard to Brio switches mapping
	//----------------------------------------------------------------------------
	U32 LinuxSwitchToBrio(U16 code)
	{
		switch(code) {
			case SW_HEADPHONE_INSERT:	return kHeadphoneJackDetect;
			case SW_TABLET_MODE:		return kCartridgeDetect;
		}
		return 0;
	}

	//----------------------------------------------------------------------------
	// Emergency exit for poweroff timeouts
	//----------------------------------------------------------------------------
	void PoweroffTimer(tTimerHndl hndl)
	{
		CPowerMPI power;
		CDebugMPI debug(kGroupEvent);
		debug.DebugOut(kDbgLvlCritical, "%s: timeout!\n", __FUNCTION__);
		power.Shutdown();
		exit(kKernelExitShutdown);
	}
}

#define CART_SOCK        "/tmp/cart_events_socket"
//============================================================================
// Button Power USB task
//============================================================================
void* CEventModule::CartridgeTask( void* arg )
{
	CEventModule*	pThis = reinterpret_cast<CEventModule*>(arg);
	struct app_message app_msg;		
	CDebugMPI debug(kGroupEvent);	
	struct pollfd	event_fd[2];
	tCartridgeData data;
	int ret;
	

	event_fd[0].fd = CreateListeningSocket(CART_SOCK);;
	event_fd[0].events = POLLIN;
	if(event_fd[0].fd < 0)	{
		debug.DebugOut(kDbgLvlCritical, "CartridgeTask: Fatal Error, cannot Create socket at %s !!\n", CART_SOCK);
		return (void *)-1;
	}
	
	while (1)
	{
		// block on driver state changes, or timeout after 1 sec
		ret = poll(event_fd, 1, 1000);
		if((ret >= 0)  && (event_fd[0].revents & POLLIN)) {
			struct sockaddr_un addr;
			socklen_t size;
			int fdsock;
			
			size= sizeof(struct sockaddr_un);
			fdsock = accept(event_fd[0].fd, (struct sockaddr *)&addr, &size);
			if (fdsock >= 0) {
				int r;
				do {
					r = recv(fdsock, &app_msg, sizeof(app_message), 0);
					if (r == sizeof(app_message)) {
						if(app_msg.payload==CARTRIDGE_STATE_REMOVED) {
							debug.DebugOut(kDbgLvlValuable, "CartridgeTask: CARTRIDGE_STATE_REMOVED message received from socket !!\n");
							
							data.cartridgeState = CARTRIDGE_STATE_REMOVED;
							CCartridgeMessage cartridge_msg(data);
							pThis->PostEvent(cartridge_msg, kCartridgeEventPriority, 0);
						}
					}
				}while (r > 0);
				close(fdsock);
			}
		}
	}
				

	close(event_fd[0].fd);
	
	return 0;
}


//----------------------------------------------------------------------------

//============================================================================
// Button Power USB task
//============================================================================
/* static */ void* CEventModule::ButtonPowerUSBTask( void* arg )
{
	CEventModule*	pThis = reinterpret_cast<CEventModule*>(arg);
	struct app_message app_msg;		
	
	struct pollfd	event_fd[NUM_INPUTS];
	int last_fd = 0;
	int size;

	int button_index = -1;
	tButtonData2 button_data;

	// init button driver and state
	button_data.buttonState = 0;
	button_data.buttonTransition = 0;
	event_fd[last_fd].fd = open_input_device("LF1000 Keyboard");
	event_fd[last_fd].events = POLLIN;
	if(event_fd[last_fd].fd >= 0)
	{
		button_index = last_fd++;

		int sw = 0;
		
		// get initial state of switches
		pThis->debug_.Assert(ioctl(event_fd[button_index].fd, EVIOCGSW(sizeof(int)), &sw) >= 0,
				"CEventModule::ButtonPowerUSBTask: reading switch state failed");
		// get and report the current headphone state as a transition
		if(sw & (1<<SW_HEADPHONE_INSERT)) {
			button_data.buttonState |= kHeadphoneJackDetect;
			button_data.buttonTransition |= kHeadphoneJackDetect;
		}
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlCritical, "CEventModule::ButtonPowerUSBTask: cannot open LF1000 Keyboard\n");
	}
	SetButtonState(button_data);
	
	int power_index = -1;
	struct tPowerData	power_data;

	// init power driver and state
	power_data.powerState = GetCurrentPowerState();
	event_fd[last_fd].fd = open_input_device("Power Button");
	event_fd[last_fd].events = POLLIN;
	if(event_fd[last_fd].fd >= 0)
	{
		power_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open Power Button\n");
	}
	
	int usb_index = -1;
	int vbus;
	tUSBDeviceData	usb_data;	

	// init USB driver and state
	usb_data = GetCurrentUSBDeviceState();
	vbus = usb_data.USBDeviceState;
	event_fd[last_fd].fd = open_input_device("LF1000 USB");
	event_fd[last_fd].events = POLLIN;
	if(event_fd[last_fd].fd >= 0)
	{
		usb_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open LF1000 USB\n");
	}
	
	// init USB socket
	int socket_index = -1;
	event_fd[last_fd].fd = CreateListeningSocket(USB_SOCK);
	event_fd[last_fd].events = POLLIN;
	if(event_fd[last_fd].fd >= 0)
	{
		socket_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open socket %s\n", USB_SOCK);
	}
	
	// init touchscreen driver
	int touch_index = -1;
	tTouchData touch_data;
	
	event_fd[last_fd].fd = open_input_device("LF1000 touchscreen interface");
	event_fd[last_fd].events = POLLIN;
	if(event_fd[last_fd].fd >= 0)
	{
		touch_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open LF1000 touchscreen interface\n");
	}
	
	struct input_event ev;
	U32 button;
	int ret;
	g_threadRunning2_ = true;
	while (g_threadRun2_)
	{
		// block on driver state changes, or timeout after 1 sec
		ret = poll(event_fd, last_fd, 1000);
		
		if(ret >= 0) {
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
						button = LinuxSwitchToBrio(ev.code);
						
						// only handle headphone JackDetect here
						if(button != kHeadphoneJackDetect) {
							if(button == 0) {
								pThis->debug_.DebugOut(kDbgLvlValuable, "%s: unknown switch code\n", __FUNCTION__);
							}
							continue;
						}
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
					SetButtonState(button_data);
					CButtonMessage button_msg(button_data);
					pThis->PostEvent(button_msg, kButtonEventPriority, 0);
				}
			}
			
			// power driver event ?
			if(power_index >= 0 && event_fd[power_index].revents & POLLIN) {
				size = read(event_fd[power_index].fd, &ev, sizeof(ev));
				switch(ev.code)
				{	/* map Linux Keys to Brio Messages */
				/* Critical Low Battery */
				case KEY_BATTERY:
					power_data.powerState = kPowerCritical;
					break;
				/* Low Battery */
				case KEY_MINUS:
					power_data.powerState = kPowerLowBattery;
					break;
				/* Battery */
				case KEY_EQUAL:
					power_data.powerState = kPowerBattery;
					break;
				/* Battery */
				case KEY_UP:
					power_data.powerState = kPowerExternal;
					break;
				/* External Power */
				case KEY_POWER:
					power_data.powerState = kPowerShutdown;
					break;
				}
				// power state changed?
				CPowerMessage power_msg(power_data);
				pThis->PostEvent(power_msg, kPowerEventPriority, 0);
				if (power_data.powerState == kPowerShutdown) {
					// Arm poweroff timer on powerdown event
					CPowerMPI power;
					tTimerProperties props = {TIMER_RELATIVE_SET, {0, 0, 0, 0}};
					tTimerHndl timer = pThis->kernel_.CreateTimer(PoweroffTimer, props, NULL);
					int msec = power.GetShutdownTimeMS();
					props.timeout.it_value.tv_sec = msec / 1000;
					props.timeout.it_value.tv_nsec = (msec % 1000) * 1000000;
					pThis->kernel_.StartTimer(timer, props);
				}
			}

			// USB driver event ?
			if(usb_index >= 0 && event_fd[usb_index].revents & POLLIN) {
				size = read(event_fd[usb_index].fd, &ev, sizeof(ev));
				if(ev.type == EV_SW && ev.code == SW_LID)
				{
					vbus = !!ev.value;
					if((vbus == 1)) {
						usb_data.USBDeviceState |= kUSBDeviceConnected;
						CUSBDeviceMessage usb_msg(usb_data);
						pThis->PostEvent(usb_msg, kUSBDeviceEventPriority, 0);
					} else if((vbus == 0)) {
						usb_data.USBDeviceState &= ~(kUSBDeviceConnected);
						CUSBDeviceMessage usb_msg(usb_data);
						pThis->PostEvent(usb_msg, kUSBDeviceEventPriority, 0);
					}
				}
			}

			// USB socket event ?
			if (socket_index >= 0 && event_fd[socket_index].revents & POLLIN) {
				struct sockaddr_un addr;
				socklen_t size = sizeof(struct sockaddr_un);
				int fdsock = accept(event_fd[socket_index].fd, (struct sockaddr *)&addr, &size);
				if (fdsock >= 0) {
					int r;
					do {
						r = recv(fdsock, &app_msg, sizeof(app_message), 0);
						if (r == sizeof(app_message)) {
							usb_data.USBDeviceState &= kUSBDeviceConnected;
							usb_data.USBDeviceState |= app_msg.payload;
							if (app_msg.payload == 0)
								usb_data.USBDeviceState = 0;
							CUSBDeviceMessage usb_msg(usb_data);
							pThis->PostEvent(usb_msg, kUSBSocketEventPriority, 0);
							SetCachedUSBDeviceState(usb_data);
						}
					}
					while (r > 0);
					close(fdsock);
				}
			}
			
			// Touch driver event ?
			if(touch_index >= 0 && event_fd[touch_index].revents & POLLIN) {
				size = read(event_fd[touch_index].fd, &ev, sizeof(ev));
				switch(ev.type)
				{
				case EV_SYN:
					touch_data.time.seconds      = ev.time.tv_sec;
					touch_data.time.microSeconds = ev.time.tv_usec;
					{
						CTouchMessage touch_msg(touch_data);
						pThis->PostEvent(touch_msg, kTouchEventPriority, 0);
					}
					break;
					
				case EV_KEY:
					touch_data.touchState = ev.value;
					break;
					
				case EV_ABS:
					switch(ev.code)
					{
					case ABS_X:
						touch_data.touchX = ev.value;
						break;
					case ABS_Y:
						touch_data.touchY = ev.value;
						break;
					}
					break;
				}
			}
		}
	}
	
	for(; last_fd >=0; --last_fd)
		close(event_fd[last_fd].fd);
	g_threadRunning2_ = false;
}
	
LF_END_BRIO_NAMESPACE()

// EOF
