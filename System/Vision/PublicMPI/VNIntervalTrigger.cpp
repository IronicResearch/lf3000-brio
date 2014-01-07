#include <Vision/VNIntervalTrigger.h>
#include <VNIntervalTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNIntervalTrigger::VNIntervalTrigger(float interval) :
    pimpl_(new VNIntervalTriggerPIMPL(interval)) {

  }

  VNIntervalTrigger::~VNIntervalTrigger(void) {

  }

  bool
  VNIntervalTrigger::Triggered(bool spatiallyTriggered) {
    return pimpl_->Triggered(spatiallyTriggered);
  }
}
}
