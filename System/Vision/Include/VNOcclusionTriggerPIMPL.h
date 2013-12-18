#ifndef __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__

#include <VNTriggerPIMPL.h>

namespace LF {
namespace Vision {

  class VNOcclusionTriggerPIMPL : public VNTriggerPIMPL {
  public:
    VNOcclusionTriggerPIMPL(float percentOccludedToTrigger);
    virtual ~VNOcclusionTriggerPIMPL(void);
    bool Triggered(void) const;

    float percentOccludedToTrigger_;
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__
