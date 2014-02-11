#include "HWControllerEventMessagePIMPL.h"
#include <Hardware/HWController.h>

namespace LF {
namespace Hardware {
  
  HWControllerEventMessagePIMPL::HWControllerEventMessagePIMPL(const HWController* controller) :
    controller_(controller) {
  }
  
  HWControllerEventMessagePIMPL::~HWControllerEventMessagePIMPL(void) {
    
  }

}
}

