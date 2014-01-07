#include <Vision/VNPointTrigger.h>
#include <VNPointTriggerPIMPL.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNHotSpot.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  VNPointTrigger::VNPointTrigger(void) :
    pimpl_(new VNPointTriggerPIMPL()) {
  }
  
  VNPointTrigger::~VNPointTrigger(void) {    
  }
  
  bool
  VNPointTrigger::Triggered(const VNHotSpot *hotSpot) {
    return pimpl_->Triggered(*hotSpot->pimpl_);
  }
    
} // namespace Vision
} // namespace LF
