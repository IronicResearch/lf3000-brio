#include "HWControllerMPIPIMPL.h"
#include <Hardware/HWController.h>
#include <HWControllerPIMPL.h>
#include <Hardware/HWControllerEventMessage.h>
#include <iostream> // AJL DEBUG
#include <dlfcn.h>

const LeapFrog::Brio::tEventType 
  kHWControllerListenerTypes[] = {LeapFrog::Brio::kAccelerometerDataChanged,
				  LeapFrog::Brio::kOrientationChanged,
				  LeapFrog::Brio::kButtonStateChanged,
				  LF::Hardware::kHWAnalogStickDataChanged};

static const LeapFrog::Brio::tEventPriority kHWControllerDefaultEventPriority = 0; // FIXME
static const LeapFrog::Brio::tEventPriority kHWControllerHighPriorityEvent = 255;  // FIXME

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
	    // Dynamically load Bluetooth client lib
		dll_ = dlopen("libBluetopiaIO.so", RTLD_LAZY);
		if (dll_ != NULL) {
			pBTIO_Init_			= (pFnInit)dlsym(dll_, "BTIO_Init");
			pBTIO_Exit_			= (pFnExit)dlsym(dll_, "BTIO_Exit");
			pBTIO_SendCommand_ 	= (pFnSendCommand)dlsym(dll_, "BTIO_SendCommand");
			pBTIO_QueryStatus_	= (pFnQueryStatus)dlsym(dll_, "BTIO_QueryStatus");

			// Connect to Bluetooth client service?
//			handle_ = pBTIO_Init_(this);
//			pBTIO_SendCommand_(handle_, kBTIOCmdSetDeviceCallback, (void*)&DeviceCallback, sizeof(void*));
//			pBTIO_SendCommand_(handle_, kBTIOCmdSetInputCallback, (void*)&InputCallback, sizeof(void*));
		}
		else {
			std::cout << "dlopen failed to load libBluetopiaIO.so, error=\n" << dlerror();
		}
  }
  
  HWControllerMPIPIMPL::~HWControllerMPIPIMPL() {
	  // Close Bluetooth client lib connection
	  if (dll_) {
		  BTIO_Exit(handle_);
		  dlclose(dll_);
	  }
  }  

  void
  HWControllerMPIPIMPL::DeviceCallback(void* context, void* data, int length) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
      std::cout << "DeviceCallback: data=" << data << "length=" << length << "\n";
  }

  void
  HWControllerMPIPIMPL::InputCallback(void* context, void* data, int length) {
	  HWControllerMPIPIMPL* pModule = (HWControllerMPIPIMPL*)context;
      std::cout << "InputCallback: data=" << data << "length=" << length << "\n";
  }

  void
  HWControllerMPIPIMPL::RegisterSelfAsListener(void) {
    eventMPI_.RegisterEventListener(this);
  }

  LeapFrog::Brio::tEventStatus 
  HWControllerMPIPIMPL::Notify(const LeapFrog::Brio::IEventMessage &msgIn) {
    LeapFrog::Brio::tEventType type = msgIn.GetEventType();
    LeapFrog::Brio::tEventPriority priority = kHWControllerDefaultEventPriority;
    HWController *controller = this->GetControllerByID(kHWDefaultControllerID);

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
	// FIXME -- multiple controller instances
    static HWController *theController_ = NULL;
    if (!theController_) {
      theController_ = new HWController();
    }
    return theController_;
  }
  
  void 
  HWControllerMPIPIMPL::GetAllControllers(std::vector<HWController*> &controller) {
    //TODO: fill all controllers
    controller.push_back(this->GetControllerByID(kHWDefaultControllerID));
  }
  
  LeapFrog::Brio::U8 
  HWControllerMPIPIMPL::GetNumberOfConnectedControllers(void) const {
    //TODO: determine number of connected controllers
    return 1;
  }

}	// namespace Hardware
}	// namespace LF
