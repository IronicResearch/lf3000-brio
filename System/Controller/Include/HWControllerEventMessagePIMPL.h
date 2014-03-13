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
    HWControllerEventMessagePIMPL(const LeapFrog::Brio::U8* address);
    
    ~HWControllerEventMessagePIMPL(void);
    
    const HWController* controller_;
    LeapFrog::Brio::U8 address_[6];
  };

}
}

#endif // __VISION_INCLUDE_HWCONTROLLEREVENTMESSAGEPIMPL_H__
