#include <Vision/VNIntervalTrigger.h>
#include <VNIntervalTriggerPIMPL.h>

namespace LF {
namespace Vision {

	VNIntervalTrigger::VNIntervalTrigger(void) :
		pimpl_(new VNIntervalTriggerPIMPL()) {

	}

	VNIntervalTrigger::VNIntervalTrigger(float minimumInterval) :
		pimpl_(new VNIntervalTriggerPIMPL(minimumInterval)) {

	}

	VNIntervalTrigger::~VNIntervalTrigger(void) {

	}

	bool
	VNIntervalTrigger::Triggered(const VNHotSpot& hotSpot) {
		// TODO:
		return false;
	}

	void
	VNIntervalTrigger::SetInterval(float interval) {
		pimpl_->interval_ = interval;
	}

	float
	VNIntervalTrigger::GetInterval(void) const {
		return pimpl_->interval_;
	}

} // namespace Vision
} // namespace LF
