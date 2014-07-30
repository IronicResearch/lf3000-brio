#include "HWControllerMPIPIMPL.h"
#include <Hardware/HWController.h>
#include <HWControllerPIMPL.h>
#include <Hardware/HWControllerEventMessage.h>
#include <Timer.h>
#include <iostream> // AJL DEBUG
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

#undef ENABLE_PROFILING	// #define to enable profiling BT callbacks
#include <FlatProfiler.h>

#if defined(EMULATION)
const LeapFrog::Brio::tEventType
  kHWControllerListenerTypes[] = {LeapFrog::Brio::kAccelerometerDataChanged,
				  LeapFrog::Brio::kOrientationChanged,
				  LeapFrog::Brio::kButtonStateChanged,
				  LeapFrog::Brio::kTimerFiredEvent,
				  LF::Hardware::kHWAnalogStickDataChanged};
#else
const LeapFrog::Brio::tEventType
kHWControllerListenerTypes[] = {LeapFrog::Brio::kButtonStateChanged};
#endif

static const LeapFrog::Brio::tEventPriority kHWControllerDefaultEventPriority = 128; // async
static const LeapFrog::Brio::tEventPriority kHWControllerHighPriorityEvent = 0;  // immediate

static LeapFrog::Brio::tMutex lock = PTHREAD_MUTEX_INITIALIZER;

static LeapFrog::Brio::COneShotTimer* timer = NULL;
static const LeapFrog::Brio::tTimerProperties props = {TIMER_RELATIVE_SET,
										 	{{0, 0}, {1, 0}},
	                                    };

namespace LF {
namespace Hardware {

  HWControllerMPIPIMPL*
  HWControllerMPIPIMPL::Instance(void) {
    static HWControllerMPIPIMPL* sharedInstance = NULL;
    if (sharedInstance == NULL) {
      sharedInstance = new HWControllerMPIPIMPL();
    }
    return sharedInstance;
  }



  HWControllerMPIPIMPL::HWControllerMPIPIMPL(void) :
    IEventListener(kHWControllerListenerTypes, ArrayCount(kHWControllerListenerTypes)),
    debugMPI_(kGroupController) {
	    numControllers_ = 0;
	    listControllers_.clear();
	    mapControllers_.clear();
	    isScanning_ = false;

	    // Dynamically load Bluetooth client lib
		dll_ = dlopen(BTIO_LIB_NAME, RTLD_LAZY);
		if (dll_ != NULL) {
			pBTIO_Init_			= (pFnInit)dlsym(dll_, "BTIO_Init");
			pBTIO_Exit_			= (pFnExit)dlsym(dll_, "BTIO_Exit");
			pBTIO_SendCommand_ 	= (pFnSendCommand)dlsym(dll_, "BTIO_SendCommand");
			pBTIO_QueryStatus_	= (pFnQueryStatus)dlsym(dll_, "BTIO_QueryStatus");
			pBTIO_ScanDevices_	= (pFnScanForDevices)dlsym(dll_, "BTIO_ScanForDevices");
			pBTIO_PairWithRemoteDevice_ = (pFnPairWithRemoteDevice)dlsym(dll_, "BTIO_PairWithRemoteDevice");
			pBTIO_GetControllerVersion_ = (pFnGetControllerVersion)dlsym(dll_, "BTIO_GetControllerVersion");
			pBTIO_EnableBluetoothDebug_ = (pFnEnableBluetoothDebug)dlsym(dll_, "BTIO_EnableBluetoothDebug");
			pBTIO_DisconnectDevice_     = (pFnDisconnectDevice)dlsym(dll_, "BTIO_DisconnectDevice");

			// Connect to Bluetooth client service?
			handle_ = pBTIO_Init_(this);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetDeviceCallback, (void*)&DeviceCallback, sizeof(void*), NULL);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetInputCallback, (void*)&InputCallback, sizeof(void*), NULL);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetScanCallback, (void*)&ScanCallback, sizeof(void*), NULL);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetInputContext, this, sizeof(void*), NULL);
			FILE* fp = fopen("/flags/controllerlog", "r");
			if(fp)
			{
				pBTIO_EnableBluetoothDebug_(true, 3, 1, "ControllerLog.btsnoop");
				fclose(fp);
			}
		}
		else {
			debugMPI_.DebugOut(kDbgLvlImportant, "%s: dlopen failed to load %s, error=%s\n", __func__, BTIO_LIB_NAME, dlerror());
		}

		timer = new COneShotTimer(props);

		// this is so we can get the sync button on device and wii mote events in emulation
		RegisterSelfAsListener();

#ifdef ENABLE_PROFILING
		FlatProfilerInit(GetMaximumNumberOfControllers(), 0);
#endif
  }

  HWControllerMPIPIMPL::~HWControllerMPIPIMPL() {
          eventMPI_.UnregisterEventListener(this);
	  std::vector<HWController*>::iterator it;
	  for (it = listControllers_.begin(); it != listControllers_.end(); it++) {
		  HWController* controller = *(it);
#ifdef ENABLE_PROFILING
		  TimeStampOff(controller->GetID());
#endif
//		  delete controller; // FIXME: HWAnalogStickPIMPL crash
	  }
	  DisconnectAllControllers();

#ifdef ENABLE_PROFILING
	  FlatProfilerDone();
#endif

	  delete timer;
	  // Close Bluetooth client lib connection
	  if (dll_) {
		  pBTIO_Exit_(handle_);
		  dlclose(dll_);
	  }
  }

  void
  HWControllerMPIPIMPL::ScanCallback(void* context, void* data, int length) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
      pModule->AddController((char*)data);
      pModule->debugMPI_.DebugOut(kDbgLvlImportant, "ScanCallback: numControllers=%d\n", pModule->numControllers_);
  }

  void
  HWControllerMPIPIMPL::DeviceCallback(void* context, void* data, int length) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
      pModule->AddController((char*)data);
      pModule->debugMPI_.DebugOut(kDbgLvlImportant, "DeviceCallback: numControllers=%d\n", pModule->numControllers_);
  }

  void
  HWControllerMPIPIMPL::InputCallback(void* context, void* data, int length, char* addr) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
      HWController* controller = NULL;
      BtAdrWrap key(addr);
      if (pModule->mapControllers_.count(key) > 0)
    	  controller = pModule->mapControllers_.at(key);
      if (!controller)
    	  return;

#ifdef ENABLE_PROFILING
	  TimeStampOff(controller->GetID());
#endif

      HWControllerPIMPL* device = dynamic_cast<HWControllerPIMPL*>(controller->pimpl_.get());
      if (device)
    	  device->LocalCallback(device, data, length);
      if (timer)
		  timer->Start(props);

#ifdef ENABLE_PROFILING
	  TimeStampOn(controller->GetID());
#endif
  }

  void
  HWControllerMPIPIMPL::ScanForDevices(void) {
	  if (!dll_)
		  return;
	  if (!isScanning_) {
			debugMPI_.DebugOut(kDbgLvlImportant, "ScanForDevices\n");
			isScanning_ = true;
			pBTIO_ScanDevices_(handle_, 0);
  	  }
  }

  void
  HWControllerMPIPIMPL::AddController(char* link) {
	  BtAdrWrap key(link);
	  if (mapControllers_.count(key) > 0) {
	      // If controller already exists, test its connectivity
	      HWController* controller = FindController(link);
	      HWControllerLEDColor color = controller->GetLEDColor();
	      int r = SendCommand(controller, kBTIOCmdSetLEDState, &color, sizeof(color));
	      controller->pimpl_->SetConnected(r >= 0);
	      HWControllerEventMessage qmsg((r < 0) ? kHWControllerDisconnected : kHWControllerConnected, controller);
	      eventMPI_.PostEvent(qmsg, kHWControllerDefaultEventPriority);
	      return;
	  }

	  if (numControllers_ >= GetMaximumNumberOfControllers()) {
		  debugMPI_.DebugOut(kDbgLvlImportant, "AddController maxed out at %d\n", numControllers_);
		  return;
	  }

      HWController* controller = new HWController();
      //BADBAD: should not be accessing pimpl_ directly!
      controller->pimpl_->SetID(numControllers_);
      listControllers_.push_back(controller);
      mapControllers_.insert(std::pair<BtAdrWrap, HWController*>(key, controller));
      numControllers_++;
      controller->pimpl_->SetBluetoothAddress( FindControllerLink(controller) );
      controller->pimpl_->SetConnected(true);

      HWControllerEventMessage qmsg(kHWControllerConnected, controller);
      eventMPI_.PostEvent(qmsg, kHWControllerDefaultEventPriority);


      unsigned char hwVersion;
      unsigned short fwVersion;
      unsigned char* pHwVersion = &hwVersion;
      unsigned short* pFwVersion = &fwVersion;
      int resultVal = pBTIO_GetControllerVersion_(link, pHwVersion, pFwVersion);
      if(!resultVal) controller->pimpl_ ->SetVersionNumbers(hwVersion, fwVersion);

#ifdef ENABLE_PROFILING
      TimeStampOn(controller->GetID());
#endif
  }

  HWController*
  HWControllerMPIPIMPL::FindController(char* link) {
	  HWController* controller = NULL;
	  BtAdrWrap key(link);
      if (mapControllers_.count(key) > 0)
    	  controller = mapControllers_.at(key);
	  return controller;
  }

  char*
  HWControllerMPIPIMPL::FindControllerLink(HWController* controller) {
	  static BtAdrWrap link;
	  std::map<BtAdrWrap, HWController*>::iterator it;
	  for (it = mapControllers_.begin(); it != mapControllers_.end(); it++) {
		  if ((it)->second == controller) {
			  link = (it)->first;
			  break;
		  }
	  }
	  if (link.empty())
		  return NULL;
	  return (char*)link.val;
  }

  int
  HWControllerMPIPIMPL::SendCommand(HWController* controller, int command, void* data, int length) {
      if (!dll_)
    	  return -1;
	  return pBTIO_SendCommand_(handle_, command, data, length, FindControllerLink(controller));
  }

  int
  HWControllerMPIPIMPL::QueryStatus(HWController* controller, int command, void* data, int length) {
      if (!dll_)
    	  return -1;
	  return pBTIO_QueryStatus_(handle_, command, data, length, FindControllerLink(controller));
  }

  void
  HWControllerMPIPIMPL::RegisterSelfAsListener(void) {
    eventMPI_.RegisterEventListener(this);
    if (!isScanning_)
    	ScanForDevices();
  }

  // returns true if the sync button was pressed, false otherwise
  bool
  HWControllerMPIPIMPL::HandleConsoleSyncButton(const LeapFrog::Brio::IEventMessage &msgIn,
						LeapFrog::Brio::tEventPriority priority) {
    LeapFrog::Brio::tEventType type = msgIn.GetEventType();
    if (type == LeapFrog::Brio::kButtonStateChanged) {
      debugMPI_.DebugOut(kDbgLvlImportant, "\nLeapFrog::Brio::kButtonStateChanged");
      
      LeapFrog::Brio::tEventType newType = kHWControllerButtonStateChanged;
      const LeapFrog::Brio::CButtonMessage &
	msg = reinterpret_cast<const LeapFrog::Brio::CButtonMessage&>(msgIn);
      tButtonData2 buttonData = msg.GetButtonState2();
      
      if(buttonData.buttonTransition & kButtonSync) {
	debugMPI_.DebugOut(kDbgLvlImportant, "\nLeapFrog::Brio::kButtonSync");
	if( buttonData.buttonState & kButtonSync ) EnableControllerSync(true);
	return true;
      }
    }
    return false;
  }

#if defined(EMULATION)
  LeapFrog::Brio::tEventStatus
  HWControllerMPIPIMPL::HandleLegacyEvents(const LeapFrog::Brio::IEventMessage &msgIn,
					   LeapFrog::Brio::tEventPriority priority) {
    LeapFrog::Brio::tEventType type = msgIn.GetEventType();
    // Legacy event message handling for incoming Buttons, Accelerometer, and AnalogStick events
    HWController *controller = this->GetControllerByID(kHWDefaultControllerID);
    if (!controller) {
        debugMPI_.DebugOut(kDbgLvlImportant, "Notify: controller=%p for event type %08x\n", controller, (unsigned)type);
    	return LeapFrog::Brio::kEventStatusOK;
    }

    // Internally generated timer event
    if (type == LeapFrog::Brio::kTimerFiredEvent) {
        debugMPI_.DebugOut(kDbgLvlImportant, "Notify: timer event type %08x\n", (unsigned)type);
        controller->pimpl_->SetConnected(false);
        HWControllerEventMessage qmsg(kHWControllerDisconnected, controller);

        //Zero out all data and post messages stating that there has been a change
        controller->pimpl_->ZeroAllData();
        HWControllerEventMessage stickMsg(kHWControllerAnalogStickDataChanged, controller);
        eventMPI_.PostEvent(stickMsg, 0);
        HWControllerEventMessage acclMsg(kHWControllerAccelerometerDataChanged, controller);
        eventMPI_.PostEvent(acclMsg, 0);
        HWControllerEventMessage btnMsg(kHWControllerButtonStateChanged, controller);
        eventMPI_.PostEvent(btnMsg, 0);

        eventMPI_.PostEvent(qmsg, kHWControllerDefaultEventPriority);
       	return LeapFrog::Brio::kEventStatusOKConsumed;
    }

    if (type == LeapFrog::Brio::kAccelerometerDataChanged ||
	type == LeapFrog::Brio::kOrientationChanged ||
	type == LeapFrog::Brio::kButtonStateChanged ||
	type == LF::Hardware::kHWAnalogStickDataChanged) {

      LeapFrog::Brio::tEventType newType = kHWControllerDataChanged;
      if (type == LeapFrog::Brio::kAccelerometerDataChanged) {
	newType = kHWControllerAccelerometerDataChanged;
	priority = kHWControllerHighPriorityEvent;
	const LeapFrog::Brio::CAccelerometerMessage&
	  msg = reinterpret_cast<const LeapFrog::Brio::CAccelerometerMessage&>(msgIn);
	controller->pimpl_->SetAccelerometerData(msg.GetAccelerometerData());

      } else if (type == LeapFrog::Brio::kButtonStateChanged) {
	newType = kHWControllerButtonStateChanged;
	const LeapFrog::Brio::CButtonMessage &
	  msg = reinterpret_cast<const LeapFrog::Brio::CButtonMessage&>(msgIn);
	tButtonData2 buttonData = msg.GetButtonState2();
	if(buttonData.buttonTransition & kButtonSync) {
	  return LeapFrog::Brio::kEventStatusOK;
	}
        controller->pimpl_->SetButtonData(msg.GetButtonState2());
      } else if (type == LF::Hardware::kHWAnalogStickDataChanged) {
	newType = kHWControllerAnalogStickDataChanged;
	priority = kHWControllerHighPriorityEvent;
	const HWAnalogStickMessage &msg = reinterpret_cast<const HWAnalogStickMessage&>(msgIn);
	controller->pimpl_->SetAnalogStickData(msg.GetAnalogStickData());
      }

      HWControllerEventMessage newMsg(newType, controller);
      eventMPI_.PostEvent(newMsg, kHWControllerHighPriorityEvent); // FIXME -- priority? default??
    }
    return LeapFrog::Brio::kEventStatusOK;
  }

#endif

  LeapFrog::Brio::tEventStatus
  HWControllerMPIPIMPL::Notify(const LeapFrog::Brio::IEventMessage &msgIn) {
    LeapFrog::Brio::tEventPriority priority = kHWControllerDefaultEventPriority;

    if (HandleConsoleSyncButton(msgIn, priority)) {
      return LeapFrog::Brio::kEventStatusOK;
    }

#if defined(EMULATION)
    return HandleLegacyEvents(msgIn, priority);
#endif

  }

  LeapFrog::Brio::tErrType
  HWControllerMPIPIMPL::EnableControllerSync(bool enable) {

	  debugMPI_.DebugOut(kDbgLvlImportant, "EnableControllerSync: Sync enabled changed %x\n", (unsigned)enable);

	  int ret = pBTIO_PairWithRemoteDevice_(handle_);
#if 1 //FIXME
	  // ret == 1 indicates "/flags/autopair" not set - no pairing happens, post no events
	  if (ret == 1)
		return kNoErr; 
#endif
	  if (ret == 0){
		HWControllerEventMessage newMsg(kHWControllerSyncSuccess, 0);
          	eventMPI_.PostEvent(newMsg, kHWControllerDefaultEventPriority);
		return kNoErr;
	  } else {
		HWControllerEventMessage newMsg(kHWControllerSyncFailure, 0);
          	eventMPI_.PostEvent(newMsg, kHWControllerDefaultEventPriority);
	  	return ret;
	  }
  }

  HWController*
  HWControllerMPIPIMPL::GetControllerByID(LeapFrog::Brio::U32 id) {
    //TODO: handle multiple controllers
    // FIXME -- emulation controller instance
#ifdef EMULATION
    static HWController *theController_ = NULL;
    if (!theController_) {
      theController_ = new HWController();
      numControllers_++;
      listControllers_.push_back(theController_);
    }
    return theController_;
#else
    HWController* controller = NULL;
    std::vector<HWController*>::iterator it;
	if (listControllers_.empty())
		ScanForDevices();
    for (it = listControllers_.begin(); it != listControllers_.end(); it++) {
    	HWController* testController = *(it);
    	if (testController->GetID() == id) {
    		controller = testController;
    		break;
    	}
    }
    return controller;
#endif
  }

  void
  HWControllerMPIPIMPL::GetAllControllers(std::vector<HWController*> &controller) {
    //TODO: fill all controllers
    // controller.push_back(this->GetControllerByID(kHWDefaultControllerID));
	if (listControllers_.empty())
		ScanForDevices();
    controller = listControllers_;
  }

  LeapFrog::Brio::U8
  HWControllerMPIPIMPL::GetNumberOfConnectedControllers(void) const {
    //TODO: determine number of connected controllers
//	if (numControllers_ == 0)
//		ScanForDevices();
    return numControllers_; //1;
  }

  void
  HWControllerMPIPIMPL::DisconnectAllControllers() {
	  debugMPI_.DebugOut(kDbgLvlImportant, "Disconnecting all controllers n = %i\n", GetNumberOfConnectedControllers());

	  std::vector<HWController*>::iterator it;
	  for (it = listControllers_.begin(); it != listControllers_.end(); it++) {
		  HWController* controller = *(it);
		  if(controller) pBTIO_DisconnectDevice_(FindControllerLink(controller), 0);
	  }

	  listControllers_.clear();
	  mapControllers_.clear();
	  numControllers_ = 0;
  }

  LeapFrog::Brio::U8
  HWControllerMPIPIMPL::GetMaximumNumberOfControllers() {
	  const LeapFrog::Brio::U8 kHWMaximumNumberOfControllers = 2; //< maximum number of simultaneously connected controllers

	  return kHWMaximumNumberOfControllers;
  }

}	// namespace Hardware
}	// namespace LF
