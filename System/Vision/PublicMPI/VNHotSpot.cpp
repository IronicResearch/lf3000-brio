#include <Vision/VNHotSpot.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {
  
  VNHotSpot::VNHotSpot(void) :
    pimpl_(new VNHotSpotPIMPL()) {
  }
  
  VNHotSpot::~VNHotSpot(void) {
    
  }
  
  void
  VNHotSpot::SetTrigger(VNTrigger *trigger) {
    pimpl_->trigger_ = trigger;
  }
  
  VNTrigger*
  VNHotSpot::GetTrigger(void) const {
    return pimpl_->trigger_;
  }
  
  void
  VNHotSpot::SetTag(LeapFrog::Brio::U32 tag) {
    pimpl_->tag_ = tag;
  }
  
  LeapFrog::Brio::U32
  VNHotSpot::GetTag(void) const {
    return pimpl_->tag_;
  }
  
  bool
  VNHotSpot::IsTriggered(void) const {
    return pimpl_->isTriggered_;
  }
  
} // namespace Vision
} // namespace LF
