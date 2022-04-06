#include <VNDurationTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNDurationTriggerPIMPL::VNDurationTriggerPIMPL(float duration) :
    duration_(duration),
    wasSpatiallyTriggered_(false) {

	SetDuration(duration);
  }

  VNDurationTriggerPIMPL::~VNDurationTriggerPIMPL(void) {

  }

  float
  VNDurationTriggerPIMPL::GetDuration(void) const {
	  return duration_;
  }

  void
  VNDurationTriggerPIMPL::SetDuration(float duration) {

	if (duration >= 0.0f) {
	  duration_ = duration;
	} else {
	  duration_ = 0.0f;
	}
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
