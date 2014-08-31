#include <SystemTypes.h>
#include <USBDeviceMPI.h>
#include <Utility.h>

#include <CameraPriv.h>

#if !defined(EMULATION) && defined(LF1000)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/lf1000/gpio_ioctl.h>
#endif

LF_BEGIN_BRIO_NAMESPACE()
//==============================================================================
// Defines
//==============================================================================
#define USB_DEV_ROOT				"/sys/class/usb_device/"
#define V4L_DEV_ROOT				"/sys/class/video4linux/"

//============================================================================
// CCameraModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CUSBCameraModule::GetModuleVersion() const
{
	return kUSBCameraModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CUSBCameraModule::GetModuleName() const
{
	return &kUSBCameraModuleName;
}


//----------------------------------------------------------------------------

//============================================================================
// Local event listener
//============================================================================
const tEventType LocalCameraEvents[] = {kAllUSBDeviceEvents};

static tMutex				pidlock = PTHREAD_MUTEX_INITIALIZER;
static int    				badpid = 0;

Boolean EnumCameraCallback(const CPath& path, void* pctx)
{
	CUSBCameraModule* pObj	= (CUSBCameraModule*)pctx;
	U32 id					= FindDevice(path);

	// Look for USB Video class device (UVC = 0xEF)
	CPath file = path + "/device/bDeviceClass";
	FILE* fp = fopen(file.c_str(), "r");
	if (fp) {
		int devclass = 0;
		fscanf(fp, "%x", &devclass);
		fclose(fp);
		if (devclass == 0xEF) {
			pObj->sysfs = path;
			return false; // stop
		}
	}

	// FIXME: Generalize camera device enumeration
	#undef  USB_CAM_ID
	#define USB_CAM_ID    	0x046d0000	// Logitech
	#define USB_WEB_CAM_ID	0x0ac80000	// Richtek
	#define USB_EXT_CAM_ID	0x18710000	// GI

	// FIXME: Mask off product ID variants in USB camera samples
	id &= 0xFFFF0000;

	if (id == USB_WEB_CAM_ID) {
		pObj->sysfs = path;
		return false; // stop
	}
	if (id == USB_EXT_CAM_ID) {
		pObj->sysfs = path;
		return false; // stop
	}
	if (id == USB_CAM_ID) {
		pObj->sysfs = path;
		return false; // stop
	}
	return true; // continue
}

#ifndef MAX_PATH
#define MAX_PATH 255
#endif
Boolean EnumVideoCallback(const CPath& path, void* pctx)
{
	CUSBCameraModule* pObj	= (CUSBCameraModule*)pctx;
	CPath file = pObj->sysfs + "/device/product";
	CPath name = path + "/name";
	CPath devclass = path + "/device/bInterfaceClass";
	char srcbuf[MAX_PATH] = "\0";
	char dstbuf[MAX_PATH] = "\0";
	FILE* fp = fopen(file.c_str(), "r");
	if (fp) {
		fscanf(fp, "%s", &srcbuf[0]);
		fclose(fp);
	}
	fp = fopen(name.c_str(), "r");
	if (fp) {
		fscanf(fp, "%s", &dstbuf[0]);
		fclose(fp);
	}
	if (strncmp(srcbuf, dstbuf, MAX_PATH) == 0) {
		pObj->devname = dstbuf;
		pObj->devpath = path;
		return false; // stop
	}
	fp = fopen(devclass.c_str(), "r");
	if (fp) {
		int interfaceclass = 0;
		fscanf(fp, "%x", &interfaceclass);
		fclose(fp);
		if (interfaceclass == 0x0E) {
			pObj->devname = dstbuf;
			pObj->devpath = path;
			return false; // stop
		}
	}

	return true; // continue
}

CUSBCameraModule::CameraListener::CameraListener(CUSBCameraModule* mod):
			IEventListener(LocalCameraEvents, ArrayCount(LocalCameraEvents)),
			pMod(mod), running(false)
			{}

CUSBCameraModule::CameraListener::~CameraListener()
{
	/*
	 * There is a potential race here if a CameraModule object is deleted just
	 * as the Notify() method is invoked by the either the EventDispatch thread or
	 * ButtonPowerUSB thread.  The proper fix would be to delay destruction until
	 * these threads have join()ed, but that is not possible, because:
	 *  a.) those threads are join()ed only when there are no outstanding clients
	 *  b.) this listener object itself counts as an outstanding client
	 *
	 *  This hack seems to make the race unlikely.
	 */
	while(running)
		pMod->kernel_.TaskSleep(1);
}

tEventStatus CUSBCameraModule::CameraListener::Notify(const IEventMessage& msg)
{
	// FIXME HACK to block AppServer contention for camera device
	if (getpid() == badpid)
		return kEventStatusOK;

	pMod->kernel_.LockMutex(pidlock);
	running = true;

	tEventType event_type = msg.GetEventType();
	if(event_type == kUSBDevicePriorityStateChange || event_type == kUSBDeviceStateChange)
	{
		const CUSBDeviceMessage& usbmsg = dynamic_cast<const CUSBDeviceMessage&>(msg);
		tUSBDeviceData usbData = usbmsg.GetUSBDeviceState();
		bool connected = (event_type == kUSBDeviceStateChange) && (usbData.USBDeviceState & kUSBDeviceConnected);
		bool disconnect = (event_type == kUSBDeviceStateChange) && (usbData.USBDeviceState == 0);
		bool hotplugged = (event_type == kUSBDevicePriorityStateChange) && (usbData.USBDeviceState & kUSBDeviceHotPlug);
		pMod->dbg_.DebugOut(kDbgLvlImportant, "CameraModule::CameraListener::Notify: pid=%d, usb=%08x\n", getpid(), (unsigned)usbData.USBDeviceState);

		/* a device was inserted or removed */
		if (connected || disconnect || hotplugged)
		{
			/* enumerate sysfs to see if camera entry exists */
			pMod->sysfs.clear();
			EnumFolder(USB_DEV_ROOT, EnumCameraCallback, kFoldersOnly, pMod);

			// Find matching V4L device entry
			pMod->devname.clear();
			pMod->devpath.clear();
			EnumFolder(V4L_DEV_ROOT, EnumVideoCallback, kFoldersOnly, pMod);

			if(!pMod->sysfs.empty() && !pMod->devpath.empty() && !pMod->valid)
			{
				struct tCaptureMode QVGA = {kCaptureFormatMJPEG, 320, 240, 1, 15};

				/* camera present or added */
				pMod->dbg_.DebugOut(kDbgLvlImportant, "CameraModule::CameraListener::Notify: USB camera detected %s\n", pMod->sysfs.c_str());
				pMod->dbg_.DebugOut(kDbgLvlImportant, "CameraModule::CameraListener::Notify: V4L device detected %s (%s)\n", pMod->devpath.c_str(), pMod->devname.c_str());

				pMod->kernel_.LockMutex(pMod->mutex_);
				// FIXME: Hacks for Glasgow support
				if (GetPlatformName() == "GLASGOW") {
					pMod->devnode = CPath("/dev") + pMod->devpath.substr(pMod->devpath.rfind('/'), pMod->devpath.length());
					pMod->camCtx_.file = pMod->devnode.c_str();
					QVGA.pixelformat = kCaptureFormatRAWYUYV;
				}
				pMod->valid = pMod->InitCameraInt(&QVGA);
				if (getpid() == badpid)
					pMod->valid = pMod->DeinitCameraInt(true);
				pMod->kernel_.UnlockMutex(pMod->mutex_);
			}
			else
			{
				if(pMod->valid)
				{
					/* camera removed */
					pMod->dbg_.DebugOut(kDbgLvlImportant, "CameraModule::CameraListener::Notify: USB camera removed %d\n", pMod->valid);

					/* set this to inform capture threads */
					pMod->valid = false;
					pMod->StopVideoCapture(pMod->camCtx_.hndl);
					pMod->StopAudioCapture(pMod->micCtx_.hndl);

					/* must uninitialize camera HW to allow 'modprobe -r' */
					pMod->kernel_.LockMutex(pMod->mutex_);
					pMod->DeinitCameraInt();
					pMod->kernel_.UnlockMutex(pMod->mutex_);
				}
				else
				{
					/* camera absent upon initialization*/
					pMod->dbg_.DebugOut(kDbgLvlImportant, "CameraModule::CameraListener::Notify: USB camera missing %d\n", pMod->valid);
					pMod->usbHost_ = CUsbHost::Instance();
				}

			}
		}
	}
	running = false;
	pMod->kernel_.UnlockMutex(pidlock);
	return kEventStatusOK;
}


//============================================================================
// Ctor & dtor
//============================================================================
CUSBCameraModule::CUSBCameraModule()
{
	tCaptureMode QVGA = {kCaptureFormatMJPEG, 320, 240, 1, 15};
	tUSBDeviceData		usb_data;

	camCtx_.mode = QVGA;
	sysfs.clear();

	// Enable USB host port
	usbHost_ = CUsbHost::Instance();

	// FIXME: Check for AppServer process instance which should *not* own the camera device
	char buf[256] = "\0";
	if (readlink("/proc/self/exe", buf, sizeof(buf)) != -1) {
		if (strstr(buf, "AppServer") != NULL) {
			badpid = getpid();
			dbg_.DebugOut(kDbgLvlImportant, "CUSBCameraModule::ctor: pid=%d for %s\n", badpid, buf);
		}
	}

	/* hotplug subsystem calls this handler upon device insertion/removal */
	listener_ = new CameraListener(this);
	event_.RegisterEventListener(listener_);

	/* spoof a hotplug event to force enumerate sysfs and see if camera is present */
	usb_data = GetCurrentUSBDeviceState();
	usb_data.USBDeviceState &= kUSBDeviceConnected;
	usb_data.USBDeviceState |= kUSBDeviceHotPlug;

	CUSBDeviceMessage usb_msg(usb_data, kUSBDevicePriorityStateChange);
	/* Event is synchronous and handled in-thread */
	valid = false;
	event_.PostEvent(usb_msg, 0, listener_);

	/* local listener handles hardware initialization as appropriate */

	// Wait for USB host power to enable drivers on Madrid
	if (GetPlatformName() == "Madrid") {
		int counter = 500;
		while (!valid && --counter > 0)
			kernel_.TaskSleep(10);
	}

	// Emulate framebuffer support
	camCtx_.fi = new struct fb_fix_screeninfo;
	camCtx_.vi = new struct fb_var_screeninfo;
}

//----------------------------------------------------------------------------
CUSBCameraModule::~CUSBCameraModule()
{
	delete camCtx_.vi;
	delete camCtx_.fi;

	event_.UnregisterEventListener(listener_);
	delete listener_;

	// Disable USB host port
	usbHost_.reset();
}

//----------------------------------------------------------------------------
tErrType CUSBCameraModule::SetCurrentCamera(tCameraDevice device)
{
	return kNoErr;
}


LF_END_BRIO_NAMESPACE()

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CUSBCameraModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version;	/* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CUSBCameraModule;
		return sinst;
	}

	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
		(void )ptr;	/* Prevent unused variable warnings. */
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG
