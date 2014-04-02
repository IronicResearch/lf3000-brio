#ifndef __VISION_INCLUDE_HWCONTROLLEREVENTMESSAGEPIMPL_H__
#define __VISION_INCLUDE_HWCONTROLLEREVENTMESSAGEPIMPL_H__

#include <CoreTypes.h>

namespace LF {
namespace Hardware {
  class HWController;
  class HWControllerEventMessage;
  
  class HWControllerEventMessagePIMPL {
  public:
    HWControllerEventMessagePIMPL(const HWController* controller);
    ~HWControllerEventMessagePIMPL(void);
    
    const HWController* controller_;
  };

}
}

#endif // __VISION_INCLUDE_HWCONTROLLEREVENTMESSAGEPIMPL_H__
