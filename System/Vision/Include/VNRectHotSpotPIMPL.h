#ifndef __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__

#include <DisplayTypes.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  class VNRectHotSpotPIMPL : public VNHotSpotPIMPL {
  public:
    VNRectHotSpotPIMPL(void);
    VNRectHotSpotPIMPL(const LeapFrog::Brio::tRect& rect);
    virtual ~VNRectHotSpotPIMPL(void);
    
    void Trigger(void *input);

    LeapFrog::Brio::tRect rect_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNHOTSPOTPIMPL_H__
