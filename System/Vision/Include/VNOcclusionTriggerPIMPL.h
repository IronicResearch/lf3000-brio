#ifndef __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__

#include <VNTriggerPIMPL.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  class VNOcclusionTriggerPIMPL : public VNTriggerPIMPL {
  public:
    VNOcclusionTriggerPIMPL(float percentOccludedToTrigger);
    virtual ~VNOcclusionTriggerPIMPL(void);
    bool Triggered(VNHotSpotPIMPL &hs);

    void TriggerWithIntegralImage(VNHotSpotPIMPL &hs,
				  cv::Mat img,
				  int &numPixles,
				  int &numDiffPixels);
    void TriggerWithDiffImage(VNHotSpotPIMPL &hs,
			      int &numPixles,
			      int &numDiffPixels);

    void SetOcclusionTriggerPercentage(float percentOccluded);

    float GetOcclusionTriggerPercentage(void) const;

    float percentOccluded_;

  private:
    float percentOccludedToTrigger_;
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__
