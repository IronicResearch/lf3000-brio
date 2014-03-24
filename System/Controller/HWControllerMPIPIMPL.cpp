#include "HWControllerMPIPIMPL.h"
#include <Hardware/HWController.h>
#include <HWControllerPIMPL.h>
#include <Hardware/HWControllerEventMessage.h>
#include <iostream> // AJL DEBUG
#include <dlfcn.h>
#include <string.h>

const LeapFrog::Brio::tEventType 
  kHWControllerListenerTypes[] = {LeapFrog::Brio::kAccelerometerDataChanged,
				  LeapFrog::Brio::kOrientationChanged,
				  LeapFrog::Brio::kButtonStateChanged,
				  LF::Hardware::kHWAllControllerEvents,
				  LF::Hardware::kHWAnalogStickDataChanged};

static const LeapFrog::Brio::tEventPriority kHWControllerDefaultEventPriority = 128; // async
static const LeapFrog::Brio::tEventPriority kHWControllerHighPriorityEvent = 0;  // immediate

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
    IEventListener(kHWControllerListenerTypes, ArrayCount(kHWControllerListenerTypes)) {
	    numControllers_ = 0;
	    listControllers_.clear();
	    mapControllers_.clear();
	    isScanning_ = false;

	    // Dynamically load Bluetooth client lib
		dll_ = dlopen("libBluetopiaIO.so", RTLD_LAZY);
		if (dll_ != NULL) {
			pBTIO_Init_			= (pFnInit)dlsym(dll_, "BTIO_Init");
			pBTIO_Exit_			= (pFnExit)dlsym(dll_, "BTIO_Exit");
			pBTIO_SendCommand_ 	= (pFnSendCommand)dlsym(dll_, "BTIO_SendCommand");
			pBTIO_QueryStatus_	= (pFnQueryStatus)dlsym(dll_, "BTIO_QueryStatus");
			pBTIO_ScanDevices_	= (pFnScanForDevices)dlsym(dll_, "BTIO_ScanForDevices");

			// Connect to Bluetooth client service?
			handle_ = pBTIO_Init_(this);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetDeviceCallback, (void*)&DeviceCallback, sizeof(void*), NULL);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetInputCallback, (void*)&InputCallback, sizeof(void*), NULL);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetScanCallback, (void*)&ScanCallback, sizeof(void*), NULL);
		}
		else {
			std::cout << "dlopen failed to load libBluetopiaIO.so, error=\n" << dlerror();
		}
  }
  
  HWControllerMPIPIMPL::~HWControllerMPIPIMPL() {
	  // Close Bluetooth client lib connection
	  if (dll_) {
		  pBTIO_Exit_(handle_);
		  dlclose(dll_);
	  }
  }  

  void
  HWControllerMPIPIMPL::ScanCallback(void* context, void* data, int length) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
      std::cout << "ScanCallback: data=" << data << "length=" << length << "\n";
      pModule->AddController((char*)data);
      std::cout << "ScanCallback: numControllers= " << pModule->numControllers_ << "\n";
  }

  void
  HWControllerMPIPIMPL::DeviceCallback(void* context, void* data, int length) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
      std::cout << "DeviceCallback: data=" << data << "length=" << length << "\n";
      pModule->AddController((char*)data);
      std::cout << "DeviceCallback: numControllers= " << pModule->numControllers_ << "\n";
  }

  void
  HWControllerMPIPIMPL::InputCallback(void* context, void* data, int length, char* addr) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
//      std::cout << "InputCallback: data=" << data << "length=" << length << "\n";
      HWController* controller = NULL; // = pModule->GetControllerByID(kHWDefaultControllerID); // FIXME
      std::string key(addr);
      if (pModule->mapControllers_.count(key) > 0)
    	  controller = pModule->mapControllers_.at(key);
      if (!controller)   
    	  return;
      HWControllerPIMPL* device = dynamic_cast<HWControllerPIMPL*>(controller->pimpl_.get());
      if (device)
    	  device->LocalCallback(device, data, length);
  }

  void
  HWControllerMPIPIMPL::ScanForDevices(void) {
	  if (!isScanning_) {
	      HWControllerEventMessage qmsg(kHWControllerLowBattery, NULL);
	   	  eventMPI_.PostEvent(qmsg, kHWControllerDefaultEventPriority, this);
  	  }
	  if (numControllers_ == 0) {
		  std::string placeholder("DEFAUL");
		  AddController((char*)placeholder.c_str());
	  }
  }

  void
  HWControllerMPIPIMPL::AddController(char* link) {
	  std::string key(link);
	  if (mapControllers_.count(key) > 0)
		  return;

	  // Replace placeholder controller with real BT device link
	  std::string placeholder("DEFAUL");
	  if (mapControllers_.count(placeholder) > 0) {
		  HWController* controller = mapControllers_.at(placeholder);
		  mapControllers_.erase(placeholder);
		  listControllers_.pop_back();
		  numControllers_--;
//		  delete controller;
	  }

      HWController* controller = new HWController();
      controller->pimpl_->SetID(numControllers_);
      listControllers_.push_back(controller);
      mapControllers_.insert(std::pair<std::string, HWController*>(key, controller));
      numControllers_++;
      HWControllerEventMessage qmsg(kHWControllerModeChanged, controller);
      eventMPI_.PostEvent(qmsg, kHWControllerDefaultEventPriority, this);
  }

  HWController*
  HWControllerMPIPIMPL::FindController(char* link) {
	  HWController* controller = NULL;
	  std::string key(link);
      if (mapControllers_.count(key) > 0)
    	  controller = mapControllers_.at(key);
	  return controller;
  }

  char*
  HWControllerMPIPIMPL::FindControllerLink(HWController* controller) {
	  std::string link;
	  std::map<std::string, HWController*>::iterator it;
	  for (it = mapControllers_.begin(); it != mapControllers_.end(); it++) {
		  if ((it)->second == controller) {
			  link = (it)->first;
			  break;
		  }
	  }
	  if (link.empty())
		  return NULL;
	  return (char*)link.c_str();
  }

  int
  HWControllerMPIPIMPL::SendCommand(HWController* controller, int command, void* data, int length) {
	  return pBTIO_SendCommand_(handle_, command, data, length, FindControllerLink(controller));
  }

  int
  HWControllerMPIPIMPL::QueryStatus(HWController* controller, int command, void* data, int length) {
	  return pBTIO_QueryStatus_(handle_, command, data, length, FindControllerLink(controller));
  }

  void
  HWControllerMPIPIMPL::RegisterSelfAsListener(void) {
    eventMPI_.RegisterEventListener(this);
    if (!isScanning_)
    	ScanForDevices();
  }


  LeapFrog::Brio::tEventStatus 
  HWControllerMPIPIMPL::Notify(const LeapFrog::Brio::IEventMessage &msgIn) {
    LeapFrog::Brio::tEventType type = msgIn.GetEventType();
    LeapFrog::Brio::tEventPriority priority = kHWControllerDefaultEventPriority;
//    HWController *controller = this->GetControllerByID(kHWDefaultControllerID);

    // Internally generated event to start scanning for controllers
    if (type == kHWControllerLowBattery) {
        const HWControllerEventMessage& hwmsg = reinterpret_cast<const HWControllerEventMessage&>(msgIn);
        if (!isScanning_) {
            std::cout << "Notify: ScanCallback\n";
            pBTIO_SendCommand_(handle_, kBTIOCmdSetScanCallback, (void*)&ScanCallback, sizeof(void*), NULL);
            pBTIO_ScanDevices_(handle_, 0);
			pBTIO_SendCommand_(handle_, kBTIOCmdSetInputContext, this, sizeof(void*), NULL);
            isScanning_ = true;
            return LeapFrog::Brio::kEventStatusOKConsumed;
        }
       	return LeapFrog::Brio::kEventStatusOK;
    }

    // Internally generated event for creating new controllers
    if (type == kHWControllerModeChanged) {
        const HWControllerEventMessage& hwmsg = reinterpret_cast<const HWControllerEventMessage&>(msgIn);
        if (hwmsg.GetController() == NULL) {
 //           HWController* controller = new HWController();
 //           HWControllerMPIPIMPL::Instance()->listControllers_.push_back(controller);
 //           HWControllerMPIPIMPL::Instance()->numControllers_++;
 //           std::cout << "Notify: HWController=" << controller << " , count=" << numControllers_ << "\n";
            return LeapFrog::Brio::kEventStatusOKConsumed;            
        }
       	return LeapFrog::Brio::kEventStatusOK;
    }

    HWController *controller = this->GetControllerByID(kHWDefaultControllerID);
    if (!controller)
    	return LeapFrog::Brio::kEventStatusOK;

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

      } else if (type == LeapFrog::Brio::kOrientationChanged) {
	newType = kHWControllerAccelerometerOrientationChanged;
	priority = kHWControllerHighPriorityEvent;

      } else if (type == LeapFrog::Brio::kButtonStateChanged) {
	newType = kHWControllerButtonStateChanged;
	const LeapFrog::Brio::CButtonMessage &
	  msg = reinterpret_cast<const LeapFrog::Brio::CButtonMessage&>(msgIn);
	controller->pimpl_->SetButtonData(msg.GetButtonState2());

      } else if (type == LF::Hardware::kHWAnalogStickDataChanged) {
	newType = kHWControllerAnalogStickDataChanged;
	priority = kHWControllerHighPriorityEvent;
	const HWAnalogStickMessage &msg = reinterpret_cast<const HWAnalogStickMessage&>(msgIn);
	controller->pimpl_->SetAnalogStickData(msg.GetAnalogStickData());
      }	       

      HWControllerEventMessage newMsg(newType, controller);
      eventMPI_.PostEvent(newMsg, 0); // FIXME -- priority?
    } 
    return LeapFrog::Brio::kEventStatusOK;
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
    	controller = *(it);
    	if (controller->GetID() == id)
    		break;
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

}	// namespace Hardware
}	// namespace LF
