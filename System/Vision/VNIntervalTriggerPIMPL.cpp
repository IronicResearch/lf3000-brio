#include <VNIntervalTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNIntervalTriggerPIMPL::VNIntervalTriggerPIMPL(float interval) :
    interval_(interval),
    firstTrigger_(false) {

  }

  VNIntervalTriggerPIMPL::~VNIntervalTriggerPIMPL(void) {

  }

  bool
  VNIntervalTriggerPIMPL::Triggered(bool spatiallyTriggered) {
    bool result = false;
    if (spatiallyTriggered) {
      if (!firstTrigger_ || timer_.elapsed() >= interval_) {
	firstTrigger_ = true;
	timer_.restart();
	result = true;
      } 
    }
    
    return result;
  }

}
}
