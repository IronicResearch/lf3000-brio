#include <VNPointTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNPointTriggerPIMPL::VNPointTriggerPIMPL(void) {
    
  }
  
  VNPointTriggerPIMPL::~VNPointTriggerPIMPL(void) {
    
  }

  bool
  VNPointTriggerPIMPL::Triggered(VNHotSpotPIMPL &hs) const {
    VNPoint p = hs.WandPoint();
    return hs.ContainsPoint(p);
  }

} // namespace Vision
} // namespace LF
