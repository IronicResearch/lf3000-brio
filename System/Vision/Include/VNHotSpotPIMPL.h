#ifndef __VISION_INCLUDE_VNHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNHOTSPOTPIMPL_H__

#include <Vision/VNTrigger.h>
#include <Vision/VNVisionTypes.h>
#include <SystemTypes.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  class VNHotSpotPIMPL {
  public:
    VNHotSpotPIMPL(void);
    virtual ~VNHotSpotPIMPL(void);
    
    virtual void Trigger(void *input);

    VNTrigger* trigger_;
    LeapFrog::Brio::U32 tag_;
    bool isTriggered_;
    
  private:
    static LeapFrog::Brio::U32 instanceCounter_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNHOTSPOTPIMPL_H__
