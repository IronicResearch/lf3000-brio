#include <Vision/VNDurationTrigger.h>
#include <VNDurationTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNDurationTrigger::VNDurationTrigger(float duration) :
    pimpl_(new VNDurationTriggerPIMPL(duration)) {

  }

  VNDurationTrigger::~VNDurationTrigger(void) {

  }

  float
  VNDurationTrigger::GetDuration(void) const {
	  return pimpl_->GetDuration();
  }

  void
  VNDurationTrigger::SetDuration(float duration){
	  pimpl_->SetDuration(duration);
  }

  bool
  VNDurationTrigger::Triggered(bool spatiallyTriggered) {
    return pimpl_->Triggered(spatiallyTriggered);
  }
}
}
