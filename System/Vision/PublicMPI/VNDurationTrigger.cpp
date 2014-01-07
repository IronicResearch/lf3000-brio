#include <Vision/VNDurationTrigger.h>
#include <VNDurationTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNDurationTrigger::VNDurationTrigger(float duration) :
    pimpl_(new VNDurationTriggerPIMPL(duration)) {

  }

  VNDurationTrigger::~VNDurationTrigger(void) {

  }

  bool
  VNDurationTrigger::Triggered(bool spatiallyTriggered) {
    return pimpl_->Triggered(spatiallyTriggered);
  }
}
}
