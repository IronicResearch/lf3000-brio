#include <VNIntervalTriggerPIMPL.h>
#include <KernelMPI.h>

namespace LF {
namespace Vision {

  VNIntervalTriggerPIMPL::VNIntervalTriggerPIMPL(float interval) :
    firstTrigger_(false) {
	  SetInterval(interval);
	  timerProperties_.type = LeapFrog::Brio::TIMER_RELATIVE_SET;
	  timerHndl_ = kernelMPI_.CreateTimer(NULL, timerProperties_);
  }

  VNIntervalTriggerPIMPL::~VNIntervalTriggerPIMPL(void) {
	  if (timerHndl_ != LeapFrog::Brio::kInvalidTimerHndl) {
	    kernelMPI_.DestroyTimer(timerHndl_);
	  }
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
      float elapsedTimeInS = 0.0f;

      // determine the elapsed time
      if (timerHndl_ != LeapFrog::Brio::kInvalidTimerHndl) {
	LeapFrog::Brio::U32 elapsedTimeInMs = 0;
	kernelMPI_.GetTimerElapsedTime(timerHndl_, &elapsedTimeInMs);
	elapsedTimeInS = (static_cast<float>(elapsedTimeInMs))/1000.0f; // FIXME: DO not use hardcoded value for milliseconds in second
      }

      // if we have not been triggered before or our elapsed time is negative or we have a valid timer and the elapsed time
      // is greater than the interval we are looking for or we don't have a valid timer, then trigger
      if (!firstTrigger_ ||
	  elapsedTimeInS < 0 ||
	  (timerHndl_ != LeapFrog::Brio::kInvalidTimerHndl && elapsedTimeInS >= interval_) ||
	  (timerHndl_ == LeapFrog::Brio::kInvalidTimerHndl)) {
	firstTrigger_ = true;

	if (timerHndl_ != LeapFrog::Brio::kInvalidTimerHndl) {
	  kernelMPI_.ResetTimer(timerHndl_, timerProperties_);
	}

	result = true;
      } 
    }
    
    return result;
  }

}
}
