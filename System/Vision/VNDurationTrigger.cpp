#include <Vision/VNDurationTrigger.h>
#include <VNDurationTriggerPIMPL.h>

namespace LF {
namespace Vision {

	VNDurationTrigger::VNDurationTrigger(void) :
		pimpl_(new VNDurationTriggerPIMPL()) {

	}

	VNDurationTrigger::VNDurationTrigger(float minimumDuration) :
		pimpl_(new VNDurationTriggerPIMPL(minimumDuration)) {

	}

	VNDurationTrigger::~VNDurationTrigger(void) {

	}

	bool
	VNDurationTrigger::Triggered(const VNHotSpot& hotSpot) {
		// TODO:
		return false;
	}

	void
	VNDurationTrigger::SetDuration(float duration) {
		pimpl_->duration_ = duration;
	}

	float
	VNDurationTrigger::GetDuration(void) const {
		return pimpl_->duration_;
	}

} // namespace Vision
} // namespace LF
