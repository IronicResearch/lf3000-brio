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
    if (pimpl_) {
      pimpl_->trigger_ = trigger;
    }
  }
  
  VNTrigger*
  VNHotSpot::GetTrigger(void) const {
    if (pimpl_) {
      return pimpl_->trigger_;
    }
    return NULL;
  }
  
  void
  VNHotSpot::SetTag(LeapFrog::Brio::U32 tag) {
    if (pimpl_) {
      pimpl_->tag_ = tag;
    }
  }
  
  LeapFrog::Brio::U32
  VNHotSpot::GetTag(void) const {
    if (pimpl_) {
      return pimpl_->tag_;
    }
    return 0;
  }
  
  bool
  VNHotSpot::IsTriggered(void) const {
    if (pimpl_) {
      return pimpl_->isTriggered_;
    }
    return false;
  }
  
} // namespace Vision
} // namespace LF
