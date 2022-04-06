#include <VNIntervalTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNIntervalTriggerPIMPL::VNIntervalTriggerPIMPL(float interval) :
    firstTrigger_(false) {
	  SetInterval(interval);
  }

  VNIntervalTriggerPIMPL::~VNIntervalTriggerPIMPL(void) {

  }

  float
  VNIntervalTriggerPIMPL::GetInterval(void) const {
	  return interval_;
  }

  void
  VNIntervalTriggerPIMPL::SetInterval(float interval) {

	if (interval >= 0.0f){
	  interval_ = interval;
	} else {
	  interval_ = 0.0f;
	}
  }

  bool
  VNIntervalTriggerPIMPL::Triggered(bool spatiallyTriggered) {
    bool result = false;
    if (spatiallyTriggered) {
      if (!firstTrigger_ || timer_.elapsed() < 0 || timer_.elapsed() >= interval_) {
	firstTrigger_ = true;
	timer_.restart();
	result = true;
      } 
    }
    
    return result;
  }

}
}
