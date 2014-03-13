#include "HWControllerEventMessagePIMPL.h"
#include <Hardware/HWController.h>
#include <Hardware/HWControllerEventMessage.h>
#include <string.h>

namespace LF {
namespace Hardware {
  
  HWControllerEventMessagePIMPL::HWControllerEventMessagePIMPL(const HWController* controller) :
    controller_(controller) {
	  memset(address_, 0, sizeof(address_));
  }
  
  HWControllerEventMessagePIMPL::HWControllerEventMessagePIMPL(const LeapFrog::Brio::U8* address) :
    controller_(NULL) {
	  memcpy(address_, address, sizeof(address_));
  }

  HWControllerEventMessagePIMPL::~HWControllerEventMessagePIMPL(void) {
    
  }

}
}

