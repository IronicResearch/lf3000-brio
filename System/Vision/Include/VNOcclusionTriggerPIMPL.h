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

    float percentOccludedToTrigger_;
    float percentOccluded_;
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__
