#include <VNDurationTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNDurationTriggerPIMPL::VNDurationTriggerPIMPL(float duration) :
    duration_(duration),
    wasSpatiallyTriggered_(false) {

  }

  VNDurationTriggerPIMPL::~VNDurationTriggerPIMPL(void) {

  }

  bool
  VNDurationTriggerPIMPL::Triggered(bool spatiallyTriggered) {
    bool result = false;
    if (spatiallyTriggered) {
      if (!wasSpatiallyTriggered_) {
	timer_.restart();
      } else {
	if (timer_.elapsed() >= duration_)
	  result = true;
      }
    }

    wasSpatiallyTriggered_ = spatiallyTriggered;
    return result;
  }

}
}
