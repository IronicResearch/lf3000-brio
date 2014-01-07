#include <Vision/VNCompoundTrigger.h>
#include <VNCompoundTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNCompoundTrigger::VNCompoundTrigger(VNSpatialTrigger *spatialTrigger,
				       VNTemporalTriggering *temporalTrigger) :
    pimpl_(new VNCompoundTriggerPIMPL(spatialTrigger, temporalTrigger)) {

  }

  VNCompoundTrigger::~VNCompoundTrigger(void) {

  }

  bool
  VNCompoundTrigger::Triggered(const VNHotSpot *hs) {
    return pimpl_->Triggered(hs);
  }
}
}
