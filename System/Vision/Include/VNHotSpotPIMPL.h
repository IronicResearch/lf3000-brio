#ifndef __VISION_INCLUDE_VNHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNHOTSPOTPIMPL_H__

#include <Vision/VNTrigger.h>
#include <Vision/VNVisionTypes.h>
#include <SystemTypes.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {
  class VNHotSpot;

  class VNHotSpotPIMPL {
  public:
    VNHotSpotPIMPL(void);
    virtual ~VNHotSpotPIMPL(void);
    
    virtual void Trigger(void *input, const VNHotSpot *hs);

    /*!
     * methods for triggers to call to facilitate triggering
     */
    virtual bool ContainsPoint(VNPoint &p);
    VNPoint WandPoint(void);
    // returns the number of pixels in the region
    // and sets the img to the triggering image
    virtual int GetTriggerImage(cv::Mat &img);

    VNTrigger* trigger_;
    LeapFrog::Brio::U32 tag_;
    bool isTriggered_;
    cv::Mat triggerImage_;

  private:
    static LeapFrog::Brio::U32 instanceCounter_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNHOTSPOTPIMPL_H__
