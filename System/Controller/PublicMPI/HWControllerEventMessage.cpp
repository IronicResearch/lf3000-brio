#include <Hardware/HWControllerEventMessage.h>
#include "HWControllerEventMessagePIMPL.h"

namespace LF {
namespace Hardware {

  HWControllerEventMessage::HWControllerEventMessage(LeapFrog::Brio::tEventType type,
						     const HWController* controller) :
    LeapFrog::Brio::IEventMessage(type),
    controller_(controller) {
  }
  
  HWControllerEventMessage::~HWControllerEventMessage(void) {
    
  }
  
  const HWController*
  HWControllerEventMessage::GetController(void) const {
    return controller_;
  }
  
  LeapFrog::Brio::U16
  HWControllerEventMessage::GetSizeInBytes(void) const {
    return sizeof(HWControllerEventMessage);
  }
  
} // namesapce Hardware
} // namespace LF
