#ifndef __VISION_INCLUDE_VNPOINTTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNPOINTTRIGGERPIMPL_H__

#include <VNTriggerPIMPL.h>

namespace LF {
namespace Vision {

  class VNPointTriggerPIMPL : public VNTriggerPIMPL {
  public:
    VNPointTriggerPIMPL(void);
    virtual ~VNPointTriggerPIMPL(void);
    bool Triggered(void) const;
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNPOINTTRIGGERPIMPL_H__
