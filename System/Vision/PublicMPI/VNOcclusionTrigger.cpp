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
    return pimpl_->Triggered(*hotSpot->pimpl_);
  }
  
  void
  VNOcclusionTrigger::SetOcclusionTriggerPercentage(float percentOccluded) {
    pimpl_->percentOccludedToTrigger_ = percentOccluded;
  }
  
  float
  VNOcclusionTrigger::GetOcclusionTriggerPercentage(void) const {
    return pimpl_->percentOccludedToTrigger_;
  }

  float
  VNOcclusionTrigger::GetPercentOccluded(void) const {
    return pimpl_->percentOccluded_;
  }
  
} // namespace Vision
} // namespace LF
