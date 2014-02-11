#include "HWControllerMPIPIMPL.h"
#include <Hardware/HWController.h>
#include <HWControllerPIMPL.h>
#include <Hardware/HWControllerEventMessage.h>
#include <iostream> // AJL DEBUG

const LeapFrog::Brio::tEventType 
  kHWControllerListenerTypes[] = {LeapFrog::Brio::kAccelerometerDataChanged,
				  LeapFrog::Brio::kOrientationChanged,
				  LeapFrog::Brio::kButtonStateChanged,
				  LF::Hardware::kHWAnalogStickDataChanged};

static const LeapFrog::Brio::tEventPriority kHWControllerDefaultEventPriority = 0;
static const LeapFrog::Brio::tEventPriority kHWControllerHighPriorityEvent = 255;

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
  }
  
  HWControllerMPIPIMPL::~HWControllerMPIPIMPL() {
  }  

  void
  HWControllerMPIPIMPL::RegisterSelfAsListener(void) {
    LeapFrog::Brio::CEventMPI eventMPI;
    eventMPI.RegisterEventListener(this);
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
      LeapFrog::Brio::CEventMPI eventMPI;
      eventMPI.PostEvent(newMsg, 0);
    } 
  }

  HWController* 
  HWControllerMPIPIMPL::GetControllerByID(LeapFrog::Brio::U32 id) {
    //TODO: handle multiple controllers
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
