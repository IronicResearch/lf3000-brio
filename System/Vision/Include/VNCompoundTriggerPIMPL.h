#ifndef __VISION_INCLUDE_VNCOMPOUNDTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNCOMPOUNDTRIGGERPIMPL_H__

#include <Vision/VNCompoundTrigger.h>

namespace LF {
namespace Vision {
    
  class VNCompoundTriggerPIMPL {
  public:
    VNCompoundTriggerPIMPL(VNSpatialTrigger *spatialTrigger,
			   VNTemporalTriggering *temporalTrigger);
    virtual ~VNCompoundTriggerPIMPL(void);

    bool Triggered(const VNHotSpot *hs);

    VNSpatialTrigger *spatialTrigger_;
    VNTemporalTriggering *temporalTrigger_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNCOMPOUNDTRIGGERPIMPL_H__
