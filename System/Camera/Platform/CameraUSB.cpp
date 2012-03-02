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
static bool SetUSBHost(bool enable)
{
#if !defined(EMULATION) && defined(LF1000)
	// USB host power option on Madrid only
	if (GetPlatformName() != "Madrid")
		return false;
	// Set USB host enable via GPIO
	int fd = open("/dev/gpio", O_RDWR | O_SYNC);
	if (fd > -1) {
		int r;
		union gpio_cmd c;
		c.outvalue.port  = 2;
		c.outvalue.pin 	 = 0;
		c.outvalue.value = (enable) ? 0 : 1;
		r = ioctl(fd, GPIO_IOCSOUTVAL, &c);
		close(fd);
		CDebugMPI dbg(kGroupCamera);
		dbg.DebugOut(kDbgLvlImportant, "%s: enable=%d, success=%d\n", __FUNCTION__, enable, (r == 0));
		return (r == 0) ? true : false;
	}
#endif
	return false;
}

//----------------------------------------------------------------------------

//============================================================================
// Local event listener
//============================================================================
const tEventType LocalCameraEvents[] = {kAllUSBDeviceEvents};

Boolean EnumCameraCallback(const CPath& path, void* pctx)
{
	CUSBCameraModule* pObj	= (CUSBCameraModule*)pctx;
	U32 id					= FindDevice(path);

	// FIXME: Generalize camera device enumeration
	#define USB_WEB_CAM_ID	0x046d08d7	// Logitech

	if (id == USB_WEB_CAM_ID) {
		pObj->sysfs = path;
		return false; // stop
	}
	if (id == USB_CAM_ID) {
		pObj->sysfs = path;
		return false; // stop
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
	running = true;

	tEventType event_type = msg.GetEventType();
	if(event_type == kUSBDevicePriorityStateChange)
	{
		const CUSBDeviceMessage& usbmsg = dynamic_cast<const CUSBDeviceMessage&>(msg);
		tUSBDeviceData usbData = usbmsg.GetUSBDeviceState();

		/* a device was inserted or removed */
		if(usbData.USBDeviceState & kUSBDeviceHotPlug)
		{
			/* enumerate sysfs to see if camera entry exists */
			pMod->sysfs.clear();
			EnumFolder(USB_DEV_ROOT, EnumCameraCallback, kFoldersOnly, pMod);

			if(!pMod->sysfs.empty())
			{
				struct tCaptureMode QVGA = {kCaptureFormatMJPEG, 320, 240, 1, 15};

				/* camera present or added */
				pMod->dbg_.DebugOut(kDbgLvlImportant, "CameraModule::CameraListener::Notify: USB camera detected %s\n", pMod->sysfs.c_str());

				pMod->kernel_.LockMutex(pMod->mutex_);
				pMod->valid = pMod->InitCameraInt(&QVGA);
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

					SetUSBHost(true);
				}

			}
		}
	}
	running = false;
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
	SetUSBHost(true);

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
}

//----------------------------------------------------------------------------
CUSBCameraModule::~CUSBCameraModule()
{
	event_.UnregisterEventListener(listener_);
	delete listener_;

	// Disable USB host port
	SetUSBHost(false);
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
