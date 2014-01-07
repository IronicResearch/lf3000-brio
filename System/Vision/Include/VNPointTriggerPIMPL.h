#ifndef __VISION_INCLUDE_VNPOINTTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNPOINTTRIGGERPIMPL_H__

#include <VNTriggerPIMPL.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  class VNPointTriggerPIMPL : public VNTriggerPIMPL {
  public:
    VNPointTriggerPIMPL(void);
    virtual ~VNPointTriggerPIMPL(void);
    bool Triggered(VNHotSpotPIMPL &hs) const;
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNPOINTTRIGGERPIMPL_H__
