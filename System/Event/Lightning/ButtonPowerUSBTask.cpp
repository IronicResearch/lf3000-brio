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

#undef __STRICT_ANSI__
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
	
	//----------------------------------------------------------------------------
	// Check NAND driver status
	//----------------------------------------------------------------------------
	#define SYSFS_HOTSWAP  "/sys/devices/platform/lf1000-nand/cart_hotswap"	
	int CheckCartDriverStatus()
	{
		FILE *fp;
		int status;
		CDebugMPI debug(kGroupEvent);
		
		fp = fopen(SYSFS_HOTSWAP, "r");
		if(fp == NULL) {
			debug.DebugOut(kDbgLvlCritical, "can't open %s !\n", SYSFS_HOTSWAP);
			return -9;
		}
		
		if(fscanf(fp, "%d", &status) != 1) {
			debug.DebugOut(kDbgLvlCritical, "Wrong data format in %s !\n", SYSFS_HOTSWAP);
			fclose(fp);
			return -9;
		}
		fclose(fp);
		return status;
	}
	
	#define MTABFILE  "/etc/mtab"
	#define MTABFILE_LINEMAX   256
	int CheckCartFSStatus()
	{
		FILE *fp;
		int status = 0;
		CDebugMPI debug(kGroupEvent);
		char line[MTABFILE_LINEMAX];
		
		fp = fopen(MTABFILE, "r");
		if(fp == NULL) {
			debug.DebugOut(kDbgLvlCritical, "can't open %s !\n", MTABFILE);
			return -9;
		}
		
		while (fgets(line, MTABFILE_LINEMAX, fp) != NULL) {
			if(strstr(line, "/Cart vfat")) {
				status = 1;
				break;
			}
		}
		fclose(fp);
		return status;
	}
	
	#define BRIO_PATH_MAX 256
	int ScanCloseCartridgeFile(void) 
	{
		char linkpath[BRIO_PATH_MAX];
		char fpath[BRIO_PATH_MAX];
		char dirpath[64];
		struct stat stat_buf;
		DIR *dirp;
		struct dirent *ep;
		int ret, count=0;
		CDebugMPI debug(kGroupEvent);		

		sprintf(dirpath, "/proc/%d/fd", getpid());

		debug.DebugOut(kDbgLvlVerbose, "dir path = %s\n", dirpath);
		
		dirp = opendir(dirpath);

		while (dirp) {
		    errno = 0;
		    if ((ep = readdir(dirp)) != NULL) {
			
			sprintf(fpath, "%s/%s", dirpath, ep->d_name);
			    
			if(stat(fpath, &stat_buf) != 0) {
				debug.DebugOut(kDbgLvlCritical, "stat(%s) error, errno = %s\n", ep->d_name, strerror(errno));
			}
			
			if(S_IFLNK & stat_buf.st_mode) {
				
				memset(linkpath, 0, BRIO_PATH_MAX);
				if(readlink(fpath, linkpath, BRIO_PATH_MAX) == -1) {
					debug.DebugOut(kDbgLvlCritical, "read link error: %s\n", strerror(errno));
				} else {
					printf("link name %s\n", linkpath);
					if (strncmp(linkpath, "/LF/Cart/", 9) == 0) {
						debug.DebugOut(kDbgLvlValuable, "%s -> %s will be forced to close\n", ep->d_name, linkpath);
						ret = close(atoi(ep->d_name));
						if(ret != 0) {
							debug.DebugOut(kDbgLvlCritical, "Error closing file, %s\n", strerror(errno));
						}
					}
				}
			}
		    } else {
			if (errno == 0) {
			    closedir(dirp);
			    return 0;
			}
			printf("error reading directory %s: %s\n", dirpath, strerror(errno)); 
			closedir(dirp);
			return -1;
		    }
		}

		return -1;
	}
	
	int WaitForCartEvent(struct pollfd *event_fd) 
	{
		int event;
		struct input_event ev;
		int ret, size;
		U32 button;
		
		
		// block on driver state changes, or timeout after 1 sec
		ret = poll(event_fd, 1, 1000);
		
		if((ret >= 0)  && (event_fd[0].revents & POLLIN)) {

			size = read(event_fd[0].fd, &ev, sizeof(ev));
			for(int i = 0; i < size; i++) {
				if(ev.type == EV_SW) { // this is a switch change
					button = LinuxSwitchToBrio(ev.code);
					
					// only handle cartridge insertion here
					if(button != kCartridgeDetect) {
						continue;
					}
				}
				else {
					continue;
				}

				if(ev.value > 0) {
					event = CARTRIDGE_EVENT_INSERT;
				}
				else if(ev.value == 0) {
					event = CARTRIDGE_EVENT_REMOVE;					
				}
			}
		} else {
			event = CARTRIDGE_EVENT_NONE;
		}
		
		return event;
	}

}


//============================================================================
// Button Power USB task
//============================================================================
void* CEventModule::CartridgeTask( void* arg )
{
	bool cart_inserted;
	int cart_status, sys_ret;
	CEventModule* pThis = reinterpret_cast<CEventModule*>(arg);
	CDebugMPI debug(kGroupEvent);	
	struct pollfd	event_fd[1];
	tCartridgeData data;
	int event;

	event_fd[0].fd = open_input_device("LF1000 Keyboard");
	event_fd[0].events = POLLIN;
	if(event_fd[0].fd >= 0)
	{
		int sw = 0;
		
		// get initial state of switches
		pThis->debug_.Assert(ioctl(event_fd[0].fd, EVIOCGSW(sizeof(int)), &sw) >= 0,
				"CEventModule::CartridgeTask: reading switch state failed");

		cart_inserted = !!(sw & (1<<SW_TABLET_MODE));
		
		if(cart_inserted) {
			data.cartridgeState = CARTRIDGE_STATE_INSERTED;

			// check cart status from driver
			cart_status = CheckCartDriverStatus();
			if(cart_status == 1) {
				/* driver initialized successfully*/
				data.cartridgeState = CARTRIDGE_STATE_DRIVER_READY;
			} else if(cart_status == -1) {
				/* driver didn't initialize successfully*/
				data.cartridgeState = CARTRIDGE_STATE_REINSERT;			
			}
			
			// check cart status from file system
			if(CheckCartFSStatus() == 1) {
				data.cartridgeState = CARTRIDGE_STATE_READY;
			}
			
		} else {
			data.cartridgeState = CARTRIDGE_STATE_CLEAN;
			
			// check cart status from driver
			cart_status = CheckCartDriverStatus();		
			if(cart_status == 1) {
				/* driver initialized, this is due to AppManager crash*/
				data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
			}
			
			// file system still mounted, force to go through a file system clean cycle
			if(CheckCartFSStatus() == 1) {
				data.cartridgeState = CARTRIDGE_STATE_REMOVED;
			}
			
		}
	}
	else
	{
		pThis->debug_.DebugOut(kDbgLvlCritical, "CEventModule::CartridgeTask: Fatal Error, cannot open LF1000 Keyboard !!\n");
		return (void *)-1;
	}
	
	while (1)
	{
		SetCartridgeState(data);

		switch(data.cartridgeState) {
			case CARTRIDGE_STATE_INSERTED: {
			
				debug.DebugOut(kDbgLvlValuable, "Cartridge task: Cartridge inserted !!\n");
				
				// NAND driver request
				sys_ret = system("echo 1 > /sys/devices/platform/lf1000-nand/cart_hotswap");
				
				// check driver status 
				cart_status = CheckCartDriverStatus();
				if(cart_status == 1) {
					data.cartridgeState = CARTRIDGE_STATE_DRIVER_READY;
				} else if (cart_status == -1) {
					debug.DebugOut(kDbgLvlValuable, "Cartridge task: Can't read cart, please reinsert cart !\n");
					CCartridgeMessage cartridge_msg(data);
					pThis->PostEvent(cartridge_msg, kCartridgeEventPriority, 0);
					data.cartridgeState = CARTRIDGE_STATE_REINSERT;
				} else {
					int sw = 0;
					debug.DebugOut(kDbgLvlValuable, "Cartridge task: cartridge insertion can't be confirmed !\n");
					
					// get cartridge switche state again
					pThis->debug_.Assert(ioctl(event_fd[0].fd, EVIOCGSW(sizeof(int)), &sw) >= 0,
							"CEventModule::CartridgeTask: reading switch state failed");
					
					if(!(sw & (1<<SW_TABLET_MODE))) {
						// cartridge is removed somehow
						data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
						debug.DebugOut(kDbgLvlValuable, "Cartridge task: CARTRIDGE_STATE_FS_CLEAN !\n");
					} else {
						//cartridge is still inserted
						data.cartridgeState = CARTRIDGE_STATE_REINSERT;
						debug.DebugOut(kDbgLvlValuable, "Cartridge task: CARTRIDGE_STATE_REINSERT !\n");
					}
					
				}
				break;
			}
			
			case CARTRIDGE_STATE_DRIVER_READY: {

				// UBI attach, and cart mount
				sys_ret = system("/etc/init.d/cartridge start") >> 8;
				
				// check return status here
				printf("cartridge scripts return code %d\n", sys_ret);
				if(sys_ret == 11) {
					int sw = 0;
					debug.DebugOut(kDbgLvlCritical, "Cartridge task: Can't attach ubi !\n");
					
					// get cartridge switche state again
					pThis->debug_.Assert(ioctl(event_fd[0].fd, EVIOCGSW(sizeof(int)), &sw) >= 0,
							"CEventModule::CartridgeTask: reading switch state failed");
					
					if(!(sw & (1<<SW_TABLET_MODE))) {
						// cartridge is removed somehow
						data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
						debug.DebugOut(kDbgLvlValuable, "Cartridge task: CARTRIDGE_STATE_FS_CLEAN !\n");
					} else {
						//cartridge is still inserted
						data.cartridgeState = CARTRIDGE_STATE_REINSERT;
						debug.DebugOut(kDbgLvlValuable, "Cartridge task: CARTRIDGE_STATE_REINSERT !\n");
					}
					
				} else if (sys_ret == 21) {
					
					int sw = 0;
					debug.DebugOut(kDbgLvlCritical, "Cartridge task: Can't mount cartridge FS !\n");
					
					// get cartridge switche state again
					pThis->debug_.Assert(ioctl(event_fd[0].fd, EVIOCGSW(sizeof(int)), &sw) >= 0,
							"CEventModule::CartridgeTask: reading switch state failed");
					
					if(!(sw & (1<<SW_TABLET_MODE))) {
						// cartridge is removed somehow
						data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
						debug.DebugOut(kDbgLvlValuable, "Cartridge task: CARTRIDGE_STATE_FS_CLEAN !\n");
					} else {
						//cartridge is still inserted
						data.cartridgeState = CARTRIDGE_STATE_REINSERT;
						debug.DebugOut(kDbgLvlValuable, "Cartridge task: CARTRIDGE_STATE_REINSERT !\n");
					}
					
				} else if(sys_ret == 0) {
					// send CARTRIDGE_STATE_READY message
					data.cartridgeState = CARTRIDGE_STATE_READY;
					CCartridgeMessage cartridge_msg(data);
					pThis->PostEvent(cartridge_msg, kCartridgeEventPriority, 0);
					pThis->debug_.DebugOut(kDbgLvlValuable, "CEventModule::CartridgeTask: CARTRIDGE_STATE_READY\n");
					debug.DebugOut(kDbgLvlValuable, "Cartridge task: Cartridge ready !!\n");					
				}
				break;	
			}
			
			case CARTRIDGE_STATE_READY: {

				event = WaitForCartEvent(event_fd);
				if(event == CARTRIDGE_EVENT_REMOVE) {
					data.cartridgeState = CARTRIDGE_STATE_REMOVED;
					CCartridgeMessage cartridge_msg(data);
					pThis->PostEvent(cartridge_msg, kCartridgeEventPriority, 0);
					
				} else if (event == CARTRIDGE_EVENT_INSERT){
					pThis->debug_.DebugOut(kDbgLvlCritical, "CEventModule::CartridgeTask: State machine error: current state ready, received insert event\n");
				}
				break;
			}
			
			case CARTRIDGE_STATE_REMOVED: {
				
				debug.DebugOut(kDbgLvlValuable, "Cartridge task: Cartridge removed, close all open cart files\n");
				ScanCloseCartridgeFile();
				
				// wait a bit, while appManager release resouces, find out more here !!!!
				// sleep(1);
				sys_ret = system("/etc/init.d/cartridge stop") >> 8;
				
				#if 1
				// check return status here
				if(sys_ret == 11) {
					debug.DebugOut(kDbgLvlCritical, "Cartridge task: Can't detach ubi !\n");
					
					//what to do? pretend normal.
					data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
				} else if (sys_ret == 12) {
					debug.DebugOut(kDbgLvlCritical, "Cartridge task: no dev/ubi2, can't detach !\n");
					
					//what to do? pretend normal.
					data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
					
				}else if (sys_ret == 21) {
					debug.DebugOut(kDbgLvlCritical, "Cartridge task: Can't unmount cartridge FS !\n");
					
					sys_ret = system("/etc/init.d/cartridge fstop") >> 8;
										
					// what to do?  pretend normal
					if(sys_ret != 0) {
						data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
					} else {
						data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
					}
					
				} else if(sys_ret == 0) {
					data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
				} else {
					debug.DebugOut(kDbgLvlCritical, "Unknown sys_ret = %d  !\n", sys_ret);
					data.cartridgeState = CARTRIDGE_STATE_FS_CLEAN;
				}
				#else
					data.cartridgeState = CARTRIDGE_STATE_UNKNOWN;
				#endif
				
				break;
			}

			case CARTRIDGE_STATE_FS_CLEAN: {
				// NAND driver request
				sys_ret = system("echo 0 > /sys/devices/platform/lf1000-nand/cart_hotswap");

				// send CARTRIDGE_STATE_REINSERT message
				data.cartridgeState = CARTRIDGE_STATE_CLEAN;
				CCartridgeMessage cartridge_msg(data);
				pThis->PostEvent(cartridge_msg, kCartridgeEventPriority, 0);
				debug.DebugOut(kDbgLvlValuable, "Cartridge task: CARTRIDGE_STATE_CLEAN\n");					
				break;
			}
			
			case CARTRIDGE_STATE_CLEAN: {
				event = WaitForCartEvent(event_fd);
				if(event == CARTRIDGE_EVENT_INSERT) {
					data.cartridgeState = CARTRIDGE_STATE_INSERTED;
				} else if(event == CARTRIDGE_EVENT_REMOVE) {
					pThis->debug_.DebugOut(kDbgLvlCritical, "CEventModule::CartridgeTask: State machine warning: current state clean, received remove event\n");
				}
				break;
			}
			
			case CARTRIDGE_STATE_REINSERT: {
				event = WaitForCartEvent(event_fd);
				if(event == CARTRIDGE_EVENT_REMOVE) {
					data.cartridgeState = CARTRIDGE_STATE_REMOVED;
				} else if (event == CARTRIDGE_EVENT_INSERT){
					pThis->debug_.DebugOut(kDbgLvlCritical, "CEventModule::CartridgeTask: State machine error: current state ready, received insert event\n");
				}
				break;
			}
			
			case CARTRIDGE_STATE_UNKNOWN:
			default: 	{
				event = WaitForCartEvent(event_fd);
				if(event == CARTRIDGE_EVENT_REMOVE) {
					data.cartridgeState = CARTRIDGE_STATE_REMOVED;
				} else if (event == CARTRIDGE_EVENT_INSERT){
					pThis->debug_.DebugOut(kDbgLvlCritical, "CEventModule::CartridgeTask: State machine error: current state unknown, received insert event\n");
				}
				break;
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
				case KEY_BATTERY:  power_data.powerState = kPowerCritical;    break;
				case KEY_POWER:    power_data.powerState = kPowerShutdown;    break;
				case KEY_MINUS:    power_data.powerState = kPowerLowBattery;  break;
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
