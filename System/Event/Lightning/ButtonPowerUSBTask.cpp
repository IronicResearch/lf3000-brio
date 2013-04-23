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
#include <AccelerometerTypes.h>
#include <errno.h>

#include <dlfcn.h>
#include <tslib.h>

LF_BEGIN_BRIO_NAMESPACE() 

// Maximum cartridge x icons allowed before exiting AppManager
#define MAX_CART_X_ICONS	3

//Maximum number of input drivers to discover
#define NUM_INPUTS	6

extern tButtonData2 gButtonData;
extern tMutex gButtonDataMutex;
extern tDpadOrientation gDpadOrientation;


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
	
	#define FLAGS_NOTSLIB	"/flags/notslib"

#if defined(LF1000)
	#define INPUT_KEYBOARD_LF1000	"LF1000 Keyboard"
#endif
	#define INPUT_KEYBOARD		"gpio-keys"
	#define INPUT_TOUCHSCREEN	"touchscreen interface"
	#define	INPUT_POWER			"Power Button"
	#define INPUT_USB			"USB"
	#define INPUT_ACLMTR		"Accelerometer"

	const tEventPriority	kButtonEventPriority	= 0;
	const tEventPriority	kPowerEventPriority	= 0;
	const tEventPriority	kUSBDeviceEventPriority	= 0;
	const tEventPriority	kUSBSocketEventPriority	= 128;
	const tEventPriority	kTouchEventPriority	= 0;
	const tEventPriority	kCartridgeEventPriority	= 0;	
	const tEventPriority	kAccelerometerEventPriority	= 255;

	volatile bool 			g_threadRun2_ = true;
	volatile bool 			g_threadRunning2_ = false;

	bool use_tslib = false;

	int (*ts_config)(struct tsdev *);
	int (*ts_fd)(struct tsdev *);
	struct tsdev *(*ts_open)(const char *dev_name, int nonblock);
	int (*ts_read)(struct tsdev *, struct ts_sample *, int);
	int (*ts_close)(struct tsdev *);

	//============================================================================
	// Local state and utility functions
	//============================================================================
	
	//----------------------------------------------------------------------------
	// Linux keyboard to Brio button mapping
	//----------------------------------------------------------------------------
	U32 LinuxKeyToBrioEmerald(U16 code)
	{
		switch(code) {
			case KEY_UP:
				switch(gDpadOrientation){
					case kDpadLandscape: return kButtonUp;
					case kDpadPortrait: return kButtonLeft;
					case kDpadLandscapeUpsideDown: return kButtonDown;
					case kDpadPortraitUpsideDown: return kButtonRight;
				}
				break;
			case KEY_DOWN:
				switch(gDpadOrientation){
					case kDpadLandscape: return kButtonDown;
					case kDpadPortrait: return kButtonRight;
					case kDpadLandscapeUpsideDown: return kButtonUp;
					case kDpadPortraitUpsideDown: return kButtonLeft;
				}
				break;
			case KEY_RIGHT:
				switch(gDpadOrientation){
					case kDpadLandscape: return kButtonRight;
					case kDpadPortrait: return kButtonUp;
					case kDpadLandscapeUpsideDown: return kButtonLeft;
					case kDpadPortraitUpsideDown: return kButtonDown;
				}
				break;
			case KEY_LEFT:
				switch(gDpadOrientation){
					case kDpadLandscape: return kButtonLeft;
					case kDpadPortrait: return kButtonDown;
					case kDpadLandscapeUpsideDown: return kButtonRight;
					case kDpadPortraitUpsideDown: return kButtonUp;
				}
				break;
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
	U32 LinuxKeyToBrioMadrid(U16 code)
	{
		switch(code) {
			case KEY_UP:
				switch(gDpadOrientation){
					case kDpadPortrait: return kButtonUp;
					case kDpadLandscape: return kButtonRight;
					case kDpadPortraitUpsideDown: return kButtonDown;
					case kDpadLandscapeUpsideDown: return kButtonLeft;
				}
				break;
			case KEY_DOWN:
				switch(gDpadOrientation){
					case kDpadPortrait: return kButtonDown;
					case kDpadLandscape: return kButtonLeft;
					case kDpadPortraitUpsideDown: return kButtonUp;
					case kDpadLandscapeUpsideDown: return kButtonRight;
				}
				break;
			case KEY_RIGHT:
				switch(gDpadOrientation){
					case kDpadPortrait: return kButtonRight;
					case kDpadLandscape: return kButtonDown;
					case kDpadPortraitUpsideDown: return kButtonLeft;
					case kDpadLandscapeUpsideDown: return kButtonUp;
				}
				break;
			case KEY_LEFT:
				switch(gDpadOrientation){
					case kDpadPortrait: return kButtonLeft;
					case kDpadLandscape: return kButtonUp;
					case kDpadPortraitUpsideDown: return kButtonRight;
					case kDpadLandscapeUpsideDown: return kButtonDown;
				}
				break;
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

	//----------------------------------------------------------------------------
	// Support for tslib touchscreen filter loading on demand
	//----------------------------------------------------------------------------
	bool LoadTSLib(CEventModule* pThis, tHndl* phandle, struct tsdev** ptsl)
	{
		static tHndl handle = 0;
		static struct tsdev* tsl = NULL;
		
		CKernelMPI kernel;

		if (use_tslib && handle && tsl) {
			*phandle = handle;
			*ptsl = tsl;
			return use_tslib;
		}

		handle = kernel.LoadModule("libts.so", RTLD_NOW | RTLD_GLOBAL);
		if (!handle)
		{
			pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: can't dlopen libts.so\n");
		}
		else
		{
			// Resolve the 4 symbols we want
			ts_open = (struct tsdev *(*)(const char *, int))
				kernel.RetrieveSymbolFromModule(handle, "ts_open");
			ts_config = (int (*)(struct tsdev *)) kernel.RetrieveSymbolFromModule(handle, "ts_config");
			ts_fd = (int (*)(struct tsdev *)) kernel.RetrieveSymbolFromModule(handle, "ts_fd");
			ts_read = (int (*)(struct tsdev *, struct ts_sample *, int)) kernel.RetrieveSymbolFromModule(handle, "ts_read");
			ts_close = (int (*)(struct tsdev *)) kernel.RetrieveSymbolFromModule(handle, "ts_close");
			if (!ts_open || !ts_config || !ts_fd || !ts_read || !ts_close)
			{
				pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: can't resolve symbols in tslib.so\n");
				kernel.UnloadModule(handle);
			}
			else
			{
				pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: Use tslib\n");
				use_tslib = true;
			}
		}

		while (use_tslib) // Use while instead of "if" so I can "break"
		{
			// Find input event device and hand it to tslib
			char dev[20];
			if (find_input_device(INPUT_TOUCHSCREEN, dev) < 0) {
				pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: Can't find touchscreen event device in /dev/input\n");
				perror ("Can't find touchscreen event device in /dev/input");
				kernel.UnloadModule(handle);
				use_tslib = false;
				break;
			}
			tsl = ts_open(dev, 1);
			if (!tsl) {
				pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: ts_open failed\n");
				perror("ts_open");
				kernel.UnloadModule(handle);
				use_tslib = false;
				break;
			}
			if (ts_config(tsl)) {
				pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: ts_config failed\n");
				perror("ts_config");
				ts_close(tsl);
				kernel.UnloadModule(handle);
				use_tslib = false;
				tsl = NULL;
				break;
			}

			// Success; get back fd for poll to know when to ts_read
			//event_fd[last_fd].fd = ts_fd (tsl);
			//event_fd[last_fd].events = POLLIN;

			// Always break, since this is just an if in disguise.
			break;
		}

		pThis->debug_.DebugOut(kDbgLvlValuable, "%s: use_tslib=%d, handle=%p, tsl=%p, fd=%d\n", __FUNCTION__, use_tslib, (void*)handle, tsl, (tsl) ? ts_fd(tsl) : 0);

		*phandle = handle;
		*ptsl = tsl;
		return use_tslib;
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
	struct stat stbuf;
	tMessageQueueHndl queue = kInvalidMessageQueueHndl;
	tErrType err = kNoErr;

	tMessageQueuePropertiesPosix props = {
	    0,                          	// msgProperties.blockingPolicy;
	    "/CartEventsIPC",			 	// msgProperties.nameQueue
	    B_S_IRWXU,                    	// msgProperties.mode
	    B_O_WRONLY|B_O_CREAT|B_O_TRUNC, // msgProperties.oflag
	    0,                          	// msgProperties.priority
	    0,                          	// msgProperties.mq_flags
	    8,                          	// msgProperties.mq_maxmsg
	    sizeof(CMessage),				// msgProperties.mq_msgsize
	    0                           	// msgProperties.mq_curmsgs
	};

	// FIXME: CreateListeningSocket() pre-emptively removes old socket
	if (stat(CART_SOCK, &stbuf) == 0) {
		debug.DebugOut(kDbgLvlCritical, "CartridgeTask: socket listener exists at %s\n", CART_SOCK);

		// Test if socket is still active
		event_fd[1].fd = CreateReportSocket(CART_SOCK);
		if (event_fd[1].fd < 0) {
			pThis->debug_.DebugOut(kDbgLvlCritical, "CartridgeTask: dead socket at %s, error %d: %s\n", CART_SOCK, errno, strerror(errno));
			goto server;
		}
		pThis->debug_.DebugOut(kDbgLvlImportant, "CartridgeTask: active socket at %s = %d\n", CART_SOCK, event_fd[1].fd);
		close(event_fd[1].fd);

		// Open inbound message queue for receiving cart messages from parent process
		props.mode = B_S_IRUSR;
		props.oflag = B_O_RDONLY;
		err = pThis->kernel_.OpenMessageQueue(queue, props, NULL);
		if (err != kNoErr) {
			pThis->debug_.DebugOut(kDbgLvlCritical, "CartridgeTask: inbound message queue failed\n");
			goto server;
		}

		while (pThis->bThreadRun_)
		{
			CMessage ipc_msg;
			ipc_msg.SetMessageSize(sizeof(ipc_msg));
			err = pThis->kernel_.ReceiveMessageOrWait(queue, &ipc_msg, sizeof(ipc_msg), 500);
			if (err == kNoErr) {
				data.cartridgeState = (eCartridgeState_)ipc_msg.GetMessageReserved();
				CCartridgeMessage cart_msg(data);
				pThis->PostEvent(cart_msg, kCartridgeEventPriority, 0);
			}
		}

		pThis->kernel_.CloseMessageQueue(queue, props);
		return (void *)0;
	}
	
server:
	event_fd[0].fd = CreateListeningSocket(CART_SOCK);;
	event_fd[0].events = POLLIN;
	if(event_fd[0].fd < 0)	{
		debug.DebugOut(kDbgLvlCritical, "CartridgeTask: Fatal Error, cannot Create socket at %s !!\n", CART_SOCK);
		return (void *)-1;
	}
	debug.DebugOut(kDbgLvlImportant, "CartridgeTask: created cart socket %s = %d\n", CART_SOCK, event_fd[0].fd);

	// Create outbound message queue for propagating cart messages to any child processes
	props.mode = B_S_IRWXU;
	props.oflag = B_O_WRONLY|B_O_CREAT|B_O_TRUNC;
	err = pThis->kernel_.OpenMessageQueue(queue, props, NULL);
	if (err != kNoErr)
		pThis->debug_.DebugOut(kDbgLvlCritical, "CartridgeTask: outbound message queue failed\n");
	
	while (pThis->bThreadRun_)
	{
		// block on driver state changes, or timeout after 1 sec
		// use timeout for cancellation point
 		ret = poll(event_fd, 1, 1000);
 		pthread_testcancel();
		if((ret > 0) && (event_fd[0].revents & (POLLERR | POLLHUP | POLLNVAL))) {
			pThis->debug_.DebugOut(kDbgLvlCritical, "CartridgeTask: socket error at %s\n", CART_SOCK);
			close(event_fd[0].fd);
			event_fd[0].fd = CreateListeningSocket(CART_SOCK);
			continue;
		}
		if((ret > 0) && (event_fd[0].revents & POLLIN)) {
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

						// Post cart message to child processes
						CMessage ipc_msg;
						ipc_msg.SetMessageSize(sizeof(ipc_msg));
						ipc_msg.SetMessagePriority(0);
						ipc_msg.SetMessageReserved((U8)app_msg.payload);
						if (queue != kInvalidMessageQueueHndl)
							pThis->kernel_.SendMessageOrWait(queue, ipc_msg, 500);
					}
				}while (r > 0);
				close(fdsock);
			}
		}
	}
				

	close(event_fd[0].fd);
	remove(CART_SOCK);
	pThis->kernel_.CloseMessageQueue(queue, props);
	
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

	CKernelMPI kernel_mpi;

	// init button driver and state
	U32 (*LinuxKeyToBrio)(U16 code) = 0;
	CString platform_name = GetPlatformFamily();
	if(platform_name == "LPAD" || platform_name == "RIO") {
		gDpadOrientation = kDpadPortrait;
		LinuxKeyToBrio = LinuxKeyToBrioMadrid;
	} else {
		gDpadOrientation = kDpadLandscape;
		LinuxKeyToBrio = LinuxKeyToBrioEmerald;
	}

	const tMutexAttr	attr = {0};
	kernel_mpi.InitMutex( gButtonDataMutex, attr );
	kernel_mpi.LockMutex(gButtonDataMutex);
	gButtonData.buttonState = 0;
	gButtonData.buttonTransition = 0;
	kernel_mpi.UnlockMutex(gButtonDataMutex);
	event_fd[last_fd].fd = open_input_device(INPUT_KEYBOARD);
	event_fd[last_fd].events = POLLIN;
#ifdef LF1000
	if (event_fd[last_fd].fd < 0)
		event_fd[last_fd].fd = open_input_device(INPUT_KEYBOARD_LF1000);
#endif
	if(event_fd[last_fd].fd >= 0)
	{
		button_index = last_fd++;

		int sw = 0;
		
		// get initial state of switches
		pThis->debug_.Assert(ioctl(event_fd[button_index].fd, EVIOCGSW(sizeof(int)), &sw) >= 0,
				"CEventModule::ButtonPowerUSBTask: reading switch state failed");
		// get and report the current headphone state as a transition
		if(sw & (1<<SW_HEADPHONE_INSERT)) {
			kernel_mpi.LockMutex(gButtonDataMutex);
			gButtonData.buttonState |= kHeadphoneJackDetect;
			gButtonData.buttonTransition |= kHeadphoneJackDetect;
			kernel_mpi.UnlockMutex(gButtonDataMutex);
		}
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlCritical, "CEventModule::ButtonPowerUSBTask: cannot open: %s\n", INPUT_KEYBOARD);
	}
	
	int power_index = -1;
	struct tPowerData	power_data;

	// init power driver and state
	power_data.powerState = GetCurrentPowerState();
	event_fd[last_fd].fd = open_input_device(INPUT_POWER);
	event_fd[last_fd].events = POLLIN;
	if(event_fd[last_fd].fd >= 0)
	{
		power_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open: %s\n", INPUT_POWER);
	}
	
	int usb_index = -1;
	int vbus;
	tUSBDeviceData	usb_data;	

	// init USB driver and state
	usb_data = GetCurrentUSBDeviceState();
	vbus = usb_data.USBDeviceState;
	event_fd[last_fd].fd = open_input_device(INPUT_USB);
	event_fd[last_fd].events = POLLIN;
	if(event_fd[last_fd].fd >= 0)
	{
		usb_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open: %s\n", INPUT_USB);
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

	use_tslib = false;
	struct stat st;
	tHndl handle;
	struct tsdev *tsl = NULL;
	if(stat(FLAGS_NOTSLIB, &st) == 0) /* file exists */
	{
		// NO TSLIB was picked
		pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: Found " FLAGS_NOTSLIB " -- tslib support disabled\n");
	}
	else
	{
		// YES TSLIB was picked
		pThis->kernel_.TaskSleep(100);
		use_tslib = LoadTSLib(pThis, &handle, &tsl);
	}

	// init touchscreen driver
	int touch_index = -1;
	int touch_slot = 0;
	tTouchData touch_data;
	tMultiTouchData mtd[2];
	memset(&mtd, 0, sizeof(mtd));

	if (use_tslib)
	{
		// Success; get back fd for poll to know when to ts_read
		event_fd[last_fd].fd = ts_fd (tsl);
		event_fd[last_fd].events = POLLIN;
	}
	if (!use_tslib)
	{
		pThis->debug_.DebugOut(kDbgLvlCritical, "ButtonPowerUSBTask: tslib: Falling back on touchscreen interface\n");
		event_fd[last_fd].fd = open_input_device(INPUT_TOUCHSCREEN);
		event_fd[last_fd].events = POLLIN;
	}
	if(event_fd[last_fd].fd >= 0)
	{
		touch_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open: %s\n", INPUT_TOUCHSCREEN);
	}
	
	// Init accelerometer driver and data
	int aclmtr_index = -1;
	tAccelerometerData aclmtr_data = {0, 0, 0, {0, 0}};

	event_fd[last_fd].fd = open_input_device(INPUT_ACLMTR);
	event_fd[last_fd].events = POLLIN;
	if (event_fd[last_fd].fd >= 0)
	{
		aclmtr_index = last_fd++;
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlImportant, "CEventModule::ButtonPowerUSBTask: cannot open: %s\n", INPUT_ACLMTR);
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
				kernel_mpi.LockMutex(gButtonDataMutex);
				gButtonData.buttonTransition = 0;
				size = read(event_fd[button_index].fd, &ev, sizeof(ev));
				for(int i = 0; i < size; i++) {
					if(ev.type == EV_KEY) { // this is a key press
						button = LinuxKeyToBrio(ev.code);
						if(button == 0) {
							pThis->debug_.DebugOut(kDbgLvlValuable, "%s: unknown key code: %d type: %d\n", __FUNCTION__, ev.code, ev.type);
							continue;
						}
						//repeat of known-pressed button
						if(ev.value == 2 && (gButtonData.buttonState & button))
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

					if(ev.value > 0 && (!(gButtonData.buttonState & button))) {
						gButtonData.buttonTransition |= button;
						gButtonData.buttonState |= button;
					}
					else if(ev.value == 0 && (gButtonData.buttonState & button)) {
						gButtonData.buttonTransition |= button;
						gButtonData.buttonState &= ~button;
					}
				}

				if(gButtonData.buttonTransition != 0) {
					gButtonData.time.seconds      = ev.time.tv_sec;
					gButtonData.time.microSeconds = ev.time.tv_usec;
					CButtonMessage button_msg(gButtonData);
					pThis->PostEvent(button_msg, kButtonEventPriority, 0);
				}
				kernel_mpi.UnlockMutex(gButtonDataMutex);
			}
			
			// power driver event ?
			if(power_index >= 0 && event_fd[power_index].revents & POLLIN) {
				size = read(event_fd[power_index].fd, &ev, sizeof(ev));
				// power events mimic key events, so ignore paired "release" states
				if (ev.value == 0)
					continue;
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
				/* External Power */
				case KEY_UP:
					power_data.powerState = kPowerExternal;
					break;
				/* Shutdown request */
				case KEY_POWER:
					power_data.powerState = kPowerShutdown;
					break;
				/* Battery is Rechargeable */
				case KEY_KPEQUAL:
					power_data.powerState = kPowerRechargeable;
					break;
				/* Battery is Charging */
				case KEY_PAGEUP:
					power_data.powerState = kPowerCharging;
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
					pThis->debug_.DebugOut(kDbgLvlCritical, "%s: vbus=%d\n", __FUNCTION__, vbus);
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
						touch_data.touchState = samp[i].pressure;
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

							// Multi-touch events requested?
							if (touch_msg.GetTouchMode() == kTouchModeMultiTouch) {
								mtd[touch_slot].td = touch_data;
								CTouchMessage mt_msg(mtd[touch_slot]);
								pThis->PostEvent(mt_msg, kTouchEventPriority, 0);
							}
							else
								pThis->PostEvent(touch_msg, kTouchEventPriority, 0);

							// Load tslib on demand?
							if (touch_msg.GetTouchMode() == kTouchModeDrawing) {
								use_tslib = LoadTSLib(pThis, &handle, &tsl);
								if (tsl != NULL)
									event_fd[touch_index].fd = ts_fd(tsl);
							}
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
						case ABS_PRESSURE:
							touch_data.touchState = ev.value;
							break;
						case ABS_MT_SLOT:
							if (ev.value >= 0 && ev.value <= 1)
								touch_slot = ev.value;
							mtd[touch_slot].td = touch_data;
							mtd[touch_slot].id = touch_slot;
							break;
						case ABS_MT_WIDTH_MAJOR:
							mtd[touch_slot].touchWidth = ev.value;
							break;
						case ABS_MT_TOUCH_MAJOR:
							mtd[touch_slot].touchHeight = ev.value;
							break;
						}
						break;
					}
				}
			}

			// Accelerometer driver event?
			if (aclmtr_index >= 0 && event_fd[aclmtr_index].revents & POLLIN)
			{
				read(event_fd[aclmtr_index].fd, &ev, sizeof(ev));

				switch (ev.type) {
				case EV_ABS:
					switch (ev.code) {
					case ABS_X: aclmtr_data.accelX = ev.value; break;
					case ABS_Y: aclmtr_data.accelY = ev.value; break;
					case ABS_Z: aclmtr_data.accelZ = ev.value; break;
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
	}
	kernel_mpi.DeInitMutex( gButtonDataMutex );

	// close tslib if in use
	if (use_tslib) {
		ts_close(tsl);
		CKernelMPI kernel;
		kernel.UnloadModule(handle);
	}
	
	for(last_fd--; last_fd >=0; --last_fd)
		close(event_fd[last_fd].fd);
	g_threadRunning2_ = false;
}

//----------------------------------------------------------------------------

//============================================================================
// Platform-specific initializer
//============================================================================

void CEventModule::InitModule(void)
{
	// Load tslib pre-emptively during module creation to serialize plugin loading
	struct stat stbf;
	if (stat("/flags/notslib", &stbf) != 0) {
		tHndl handle;
		struct tsdev* tsl = NULL;
		LoadTSLib(this, &handle, &tsl);
	}
}

//----------------------------------------------------------------------------
void CEventModule::DeInitModule(void)
{

}

//----------------------------------------------------------------------------
LF_END_BRIO_NAMESPACE()

// EOF
