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

#include <string.h>

#include <linux/input.h>
#include <ButtonTypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <poll.h>
#include <PowerTypes.h>
#include <USBDeviceTypes.h>
#include <Utility.h>
#include <TouchTypes.h> 
LF_BEGIN_BRIO_NAMESPACE() 

//Maximum number of input drivers to discover
#define NUM_INPUTS 4

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
	const tEventPriority	kTouchEventPriority	= 0;
	
	volatile bool 			g_threadRun2_ = true;
	volatile bool 			g_threadRunning2_ = false;

	//============================================================================
	// Local state and utility functions
	//============================================================================
	
	//----------------------------------------------------------------------------
	static bool CartInitiallyInserted(void)
	{
		CDebugMPI dbg(kGroupButton);
		char buf[16];
		FILE *f = fopen("/sys/devices/platform/lf1000-nand/cartridge", "r");
	
		dbg.Assert(f != NULL,
				"CButtonModule::LightningButtonTask: cart read failed");
	
		dbg.Assert(fscanf(f, "%s\n", buf) >= 1,
				"CButtonModule::LightningButtonTask: cart read failed");
	
		fclose(f);
	
		if(!strncmp(buf, "none", 4)) // no cart inserted
			return false;
		return true;
	}
	
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
		// get the current state of the cartrdige
		if(sw & (1<<SW_TABLET_MODE))
			button_data.buttonState |= kCartridgeDetect;

		bool CartInitial = CartInitiallyInserted();
		if(CartInitial ^ !!(sw & (1<<SW_TABLET_MODE)))
			button_data.buttonTransition |= kCartridgeDetect;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open LF1000 Keyboard\n");
	}
	SetButtonState(button_data);
	
	int power_index = -1;
	struct tPowerData   current_pe;
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
							pThis->debug_.DebugOut(kDbgLvlVerbose, "%s: unknown key code: %d type: %d\n", __FUNCTION__, ev.code, ev.type);
							continue;
						}
						//repeat of known-pressed button
						if(ev.value == 2 && (button_data.buttonState & button))
							continue;
					}
					else if(ev.type == EV_SW) { // this is a switch change
						button = LinuxSwitchToBrio(ev.code);
						if(button == 0) {
							pThis->debug_.DebugOut(kDbgLvlVerbose, "%s: unknown switch code\n", __FUNCTION__);
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
			current_pe.powerState = GetCurrentPowerState();
			if(power_index >= 0 && event_fd[power_index].revents & POLLIN) {
				size = read(event_fd[power_index].fd, &ev, sizeof(ev));
				switch(ev.code)
				{
				case KEY_BATTERY:
					current_pe.powerState = kPowerCritical;
					break;
									
				case KEY_POWER:
					current_pe.powerState = kPowerShutdown;
					break;
				}
			}
			// power state changed? (polled for low-battery)
			if (power_data.powerState != current_pe.powerState &&
						power_data.powerState != kPowerShutdown) {
					power_data.powerState = current_pe.powerState;
					CPowerMessage power_msg(power_data);
					pThis->PostEvent(power_msg, kPowerEventPriority, 0);
			}
		
			// USB driver event ?
			if(usb_index >= 0 && event_fd[usb_index].revents & POLLIN) {
				size = read(event_fd[usb_index].fd, &ev, sizeof(ev));
				if(ev.type == EV_SW && ev.code == SW_LID)
				{
					vbus = !!ev.value;
					if((vbus == 1) && !(usb_data.USBDeviceState & kUSBDeviceConnected)) {
						usb_data.USBDeviceState |= kUSBDeviceConnected;
						CUSBDeviceMessage usb_msg(usb_data);
						pThis->PostEvent(usb_msg, kUSBDeviceEventPriority, 0);
					} else if((vbus == 0) && (usb_data.USBDeviceState & kUSBDeviceConnected)) {
						usb_data.USBDeviceState &= ~(kUSBDeviceConnected);
						CUSBDeviceMessage usb_msg(usb_data);
						pThis->PostEvent(usb_msg, kUSBDeviceEventPriority, 0);
					}
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
