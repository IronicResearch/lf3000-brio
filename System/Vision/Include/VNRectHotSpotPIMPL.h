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
    
    void Trigger(void *input, const VNHotSpot *hs);

    virtual bool ContainsPoint(VNPoint &p);
    virtual int GetTriggerImage(cv::Mat &img);

    LeapFrog::Brio::tRect rect_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__
