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

#include <dlfcn.h>
#include <tslib.h>

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
	#define FLAGS_NOTSLIB	"/flags/notslib"
	
	const tEventPriority	kButtonEventPriority	= 0;
	const tEventPriority	kPowerEventPriority	= 0;
	const tEventPriority	kUSBDeviceEventPriority	= 0;
	const tEventPriority	kUSBSocketEventPriority	= 128;
	const tEventPriority	kTouchEventPriority	= 0;
	const tEventPriority	kCartridgeEventPriority	= 0;	

	volatile bool 			g_threadRun2_ = true;
	volatile bool 			g_threadRunning2_ = false;

	bool use_tslib = false;

	//============================================================================
	// Local state and utility functions
	//============================================================================
	
	#define MAX_DEVNODES	8
	
	//----------------------------------------------------------------------------
	// find and open an input device by name
	//----------------------------------------------------------------------------

	// Fill char dev[20] and return 0 or -1 for failure.
	int find_input_device(const char *input_name, char *dev)
	{
		struct dirent *dp;
		char name[32+1];
		DIR *dir;
		int fd, i;

		dir = opendir("/dev/input/");
		if (!dir)
			return -1;
	
		while ((dp = readdir(dir)) != NULL) {
			if (dp->d_name && !strncmp(dp->d_name, "event", 5)) {
				sprintf(dev, "/dev/input/%s", dp->d_name);
				fd = open(dev, O_RDONLY);
				if (fd < 0)
					continue;
				int i=ioctl(fd, EVIOCGNAME(32), name);
				close (fd);
				if (i < 0)
					continue;
				if (!strcmp(name, input_name)) {
					closedir(dir);
					return 0;
				}
			}
		}
		closedir(dir);
		return -1;
	}

	int open_input_device(const char *input_name)
	{
		char dev[20];
		if (find_input_device (input_name, dev) < 0)
			return -1;
		return open(dev, O_RDONLY);
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
	
	while (pThis->bThreadRun_)
	{
		// block on driver state changes, or timeout after 1 sec
		// use timeout for cancellation point
 		ret = poll(event_fd, 1, 1000);
        pthread_testcancel();
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
						data.cartridgeState = (eCartridgeState_)app_msg.payload;
						CCartridgeMessage cartridge_msg(data);
						pThis->PostEvent(cartridge_msg, kCartridgeEventPriority, 0);
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
	
	// Determine if tslib is used or not

	int (*ts_config)(struct tsdev *);
	int (*ts_fd)(struct tsdev *);
	struct tsdev *(*ts_open)(const char *dev_name, int nonblock);
	int (*ts_read)(struct tsdev *, struct ts_sample *, int);

	use_tslib = false;
	struct stat st;
	if(stat(FLAGS_NOTSLIB, &st) == 0) /* file exists */
	{
		// NO TSLIB was picked
		pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: Did not find " FLAGS_NOTSLIB "\n");
	}
	else
	{
		// YES TSLIB was picked
		// Hack: do dlopen ourselves so we can assert RTLD_GLOBAL flag, or plugins will
		// not find symbols defined in tslib
		void *handle;
		handle = dlopen("/usr/lib/libts.so", RTLD_NOW | RTLD_GLOBAL);
		if (!handle)
		{
			pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: can't dlopen libts.so\n");
		}
		else
		{
			// Resolve the 4 symbols we want
			ts_open = (struct tsdev *(*)(const char *, int))
				dlsym(handle, "ts_open");
			ts_config = (int (*)(struct tsdev *)) dlsym(handle, "ts_config");
			ts_fd = (int (*)(struct tsdev *)) dlsym(handle, "ts_fd");
			ts_read = (int (*)(struct tsdev *, struct ts_sample *, int)) dlsym(handle, "ts_read");
			if (!ts_open || !ts_config || !ts_fd || !ts_read)
			{
				pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: can't resolve symbols in tslib.so\n");
			}
			else
			{
				pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: Use tslib\n");				
				use_tslib = true;
			}
		}
	}

	// init touchscreen driver
	int touch_index = -1;
	tTouchData touch_data;
	struct tsdev *tsl = NULL;

	while (use_tslib) // Use while instead of "if" so I can "break"
	{
		// Find input event device and hand it to tslib
		char dev[20];
		if (find_input_device("LF1000 touchscreen interface", dev) < 0) {
			pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: Can't find touchscreen event device in /dev/input\n");
			perror ("Can't find touchscreen event device in /dev/input");
			use_tslib = false;
			break;
		}
		tsl = ts_open(dev, 1);
		if (!tsl) {
			pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: ts_open failed\n");
			perror("ts_open");
			use_tslib = false;
			break;
		}
		if (ts_config(tsl)) {
			pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: ts_config failed\n");
			perror("ts_config");
			use_tslib = false;
			break;
		}

		// Success; get back fd for poll to know when to ts_read
		event_fd[last_fd].fd = ts_fd (tsl);
		event_fd[last_fd].events = POLLIN;

		// Always break, since this is just an if in disguise.
		break;
	}
	if (!use_tslib)
	{
		pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: Falling back on LF1000 touchscreen interface\n");
		event_fd[last_fd].fd = open_input_device("LF1000 touchscreen interface");
		event_fd[last_fd].events = POLLIN;
	}
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
	while (pThis->bThreadRun_)
	{
		// block on driver state changes only
		// use timeout for cancellation point
		ret = poll(event_fd, last_fd, 1000);
		pthread_testcancel();
		
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
					pThis->debug_.DebugOut(kDbgLvlValuable, "%s: vbus=%d\n", __FUNCTION__, vbus);
					if((vbus == 1)) {
						usb_data.USBDeviceState |= kUSBDeviceConnected;
						CUSBDeviceMessage usb_msg(usb_data);
						pThis->PostEvent(usb_msg, kUSBDeviceEventPriority, 0);
					} else if((vbus == 0)) {
						usb_data.USBDeviceState = 0; // clear all cached state
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
							CUSBDeviceMessage usb_priority_msg(usb_data, kUSBDevicePriorityStateChange);
							pThis->PostEvent(usb_priority_msg, kUSBDeviceEventPriority, 0);
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
				// Using tslib?
				if (use_tslib)
				{
					// Use tslib; drain up to 64 events at once
					struct ts_sample samp[64];
					int i, ret = ts_read(tsl, samp, 64);
					for (i=0; i<ret; i++)
					{
						touch_data.time.seconds      = samp[i].tv.tv_sec;
						touch_data.time.microSeconds = samp[i].tv.tv_usec;
						touch_data.touchX = samp[i].x;
						touch_data.touchY = samp[i].y;
						// touch_data.touchP = samp[i].pressure;

						// Note: We may wish to send 2 points if pressure==0 from tslib:
						// One as the last point before pen up, and a 2nd with only
						// the pen up information
#if 1
						// Don't do anything fancy... Just send it
						touch_data.touchState = samp[i].pressure > 0 ? 1 : 0;
						CTouchMessage touch_msg(touch_data);
						pThis->PostEvent(touch_msg, kTouchEventPriority, 0);
#else
						// Send any pen-up point twice, once still down...
						touch_data.touchState = 1;
						CTouchMessage touch_msg(touch_data);
						pThis->PostEvent(touch_msg, kTouchEventPriority, 0);
						if (samp[i].pressure == 0)
						{
							// Send another with pen up
							touch_data.touchState = 0;
							CTouchMessage touch_msg(touch_data);
							pThis->PostEvent(touch_msg, kTouchEventPriority, 0);
						}
#endif
					}
				}
				else
				{
					// Use raw input event system
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
	}
	
	for(last_fd--; last_fd >=0; --last_fd)
		close(event_fd[last_fd].fd);
	g_threadRunning2_ = false;
}
	
LF_END_BRIO_NAMESPACE()

// EOF
