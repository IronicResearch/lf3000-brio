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
    VNTrigger::pimpl_ = pimpl_;
  }
  
  VNOcclusionTrigger::~VNOcclusionTrigger(void) {
    
  }
  
  bool
  VNOcclusionTrigger::Triggered(void) {
    pimpl_->Triggered();
  }
  
  void
  VNOcclusionTrigger::SetOcclusionPercentage(float percentOccluded) {
    pimpl_->percentOccludedToTrigger_ = percentOccluded;
  }
  
  float
  VNOcclusionTrigger::GetOcclusionPercentage(void) const {
    return pimpl_->percentOccludedToTrigger_;
  }
  
} // namespace Vision
} // namespace LF
