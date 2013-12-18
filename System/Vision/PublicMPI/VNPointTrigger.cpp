#include <Vision/VNPointTrigger.h>
#include <VNPointTriggerPIMPL.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNHotSpot.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  VNPointTrigger::VNPointTrigger(void) :
    pimpl_(new VNPointTriggerPIMPL()) {
    VNTrigger::pimpl_ = pimpl_;
  }
  
  VNPointTrigger::~VNPointTrigger(void) {    
  }
  
  bool
  VNPointTrigger::Triggered(void) {
    return pimpl_->Triggered();
  }
    
} // namespace Vision
} // namespace LF
