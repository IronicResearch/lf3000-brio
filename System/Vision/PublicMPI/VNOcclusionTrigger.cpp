#include <Vision/VNOcclusionTrigger.h>
#include <VNOcclusionTriggerPIMPL.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNHotSpot.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {
  const float kVNDefaultPercentOccludedToTrigger = 0.2f;

  VNOcclusionTrigger::VNOcclusionTrigger(float percentOccluded) :
    pimpl_(new VNOcclusionTriggerPIMPL(percentOccluded)) {
  }
  
  VNOcclusionTrigger::~VNOcclusionTrigger(void) {
    
  }
  
  bool
  VNOcclusionTrigger::Triggered(const VNHotSpot *hotSpot) {
    if (hotSpot && pimpl_) {
      return pimpl_->Triggered(*hotSpot->pimpl_);
    }
    return false;
  }
  
  void
  VNOcclusionTrigger::SetOcclusionTriggerPercentage(float percentOccluded) {
    if (pimpl_) {
      pimpl_->percentOccludedToTrigger_ = percentOccluded;
    }
  }
  
  float
  VNOcclusionTrigger::GetOcclusionTriggerPercentage(void) const {
    if (pimpl_) {
      return pimpl_->percentOccludedToTrigger_;
    }
    return kVNDefaultPercentOccludedToTrigger;
  }

  float
  VNOcclusionTrigger::GetPercentOccluded(void) const {
    if (pimpl_) {
      return pimpl_->percentOccluded_;
    }
    return 0.0f;
  }
  
} // namespace Vision
} // namespace LF
