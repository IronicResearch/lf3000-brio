#include <Vision/VNIntervalTrigger.h>
#include <VNIntervalTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNIntervalTrigger::VNIntervalTrigger(float interval) :
    pimpl_(new VNIntervalTriggerPIMPL(interval)) {

  }

  VNIntervalTrigger::~VNIntervalTrigger(void) {

  }


  float
  VNIntervalTrigger::GetInterval(void) const {
	  return pimpl_->GetInterval();
  }

  void
  VNIntervalTrigger::SetInterval(float duration){
	  return pimpl_->SetInterval(duration);
  }

  bool
  VNIntervalTrigger::Triggered(bool spatiallyTriggered) {
    return pimpl_->Triggered(spatiallyTriggered);
  }

}
}
