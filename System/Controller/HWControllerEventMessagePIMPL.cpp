#include "HWControllerEventMessagePIMPL.h"
#include <Hardware/HWController.h>
#include <Hardware/HWControllerEventMessage.h>
#include <string.h>

namespace LF {
namespace Hardware {
  
  HWControllerEventMessagePIMPL::HWControllerEventMessagePIMPL(const HWController* controller) :
    controller_(controller) {
  }
  
  HWControllerEventMessagePIMPL::~HWControllerEventMessagePIMPL(void) {
    
  }

}
}

