#include <VNPointTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNPointTriggerPIMPL::VNPointTriggerPIMPL(void) {
    
  }
  
  VNPointTriggerPIMPL::~VNPointTriggerPIMPL(void) {
    
  }

  bool
  VNPointTriggerPIMPL::Triggered(void) const {
    cv::Point p(triggerPoint_.x, triggerPoint_.y);
    return triggerRect_.contains(p);
  }

} // namespace Vision
} // namespace LF
