#include <VNCompoundTriggerPIMPL.h>
#include <Vision/VNSpatialTrigger.h>
#include <Vision/VNTemporalTriggering.h>

namespace LF {
namespace Vision {

  VNCompoundTriggerPIMPL::VNCompoundTriggerPIMPL(VNSpatialTrigger *spatialTrigger,
						 VNTemporalTriggering *temporalTrigger) :
    spatialTrigger_(spatialTrigger),
    temporalTrigger_(temporalTrigger) {

  }

  VNCompoundTriggerPIMPL::~VNCompoundTriggerPIMPL(void) {

  }


  bool
  VNCompoundTriggerPIMPL::Triggered(const VNHotSpot *hs) {
    if (spatialTrigger_ && temporalTrigger_)
      return temporalTrigger_->Triggered(spatialTrigger_->Triggered(hs));
    return false;
  }

}
}
