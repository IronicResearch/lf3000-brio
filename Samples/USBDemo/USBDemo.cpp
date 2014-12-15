/******************************************************************************

 @Description  A very simple app that monitors the USB Device Interface

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <USBDeviceMPI.h>
#include <EmulationConfig.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <boost/scoped_ptr.hpp>
#include <Utility.h>
#include <sys/socket.h>
#include <errno.h>

LF_USING_BRIO_NAMESPACE()


//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// USB Device MPI event handler
//============================================================================
const tEventType kMyHandledTypes[] = { kAllUSBDeviceEvents };

//----------------------------------------------------------------------------
class MyUsbEventListener : public IEventListener
{
 public:
	MyUsbEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)),dbg_(kMyApp) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
		{
			type_ = msg.GetEventType();
			const CUSBDeviceMessage& usbmsg = reinterpret_cast<const CUSBDeviceMessage&>(msg);
			tUSBDeviceData data = usbmsg.GetUSBDeviceState();
			dbg_.DebugOut(kDbgLvlCritical, "USB Device change: state=%d, driver=%d, support=%d\n", 
					static_cast<unsigned int>(data.USBDeviceState),
					static_cast<unsigned int>(data.USBDeviceDriver),
					static_cast<unsigned int>(data.USBDeviceSupports));
			return kEventStatusOKConsumed;
		}
	tEventType	type_;
private:
 	CDebugMPI	dbg_;
};

//----------------------------------------------------------------------------
void PostFakeSocketMessage(int code)
{
	struct app_message msg;
	int fdsock = CreateReportSocket(USB_SOCK);
	if (fdsock < 0)
		return;
	msg.type = APP_MSG_SET_USB;
	msg.payload = code;
	send(fdsock, &msg, sizeof(struct app_message), 0);
	close(fdsock);
}

//----------------------------------------------------------------------------
int main()
{
	CDebugMPI dbg(kMyApp);

#ifdef EMULATION
	dbg.Assert(false, "This sample only runs on the target platform!");
#endif
	
	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "USBDemo setup...timeout after 30 sec\n");

	CUSBDeviceMPI		usbmgr;
	MyUsbEventListener 	handler;

	if (usbmgr.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "Failed to register event handler.\n");
		return -1;
	}
	
	dbg.DebugOut(kDbgLvlVerbose, "USBDemo loop: Plug or unplug Lightning USB device\n");
	for (int i = 0; i < 30; i++) {
		/* Just hang around waiting for handler to be called. */
		sleep(1);
#ifdef USE_FAKE_MESSAGES
		PostFakeSocketMessage(i);
#endif
	}

	dbg.DebugOut(kDbgLvlVerbose, "USBDemo done.\n");
	usbmgr.UnregisterEventListener(&handler);
	return 0;
}
