#include <VNDurationTriggerPIMPL.h>

const float kVNDefaultDurationToTrigger = 0.2f; // in seconds

namespace LF {
namespace Vision {

	VNDurationTriggerPIMPL::VNDurationTriggerPIMPL(void) :
		duration_(kVNDefaultDurationToTrigger) {

	}

	VNDurationTriggerPIMPL::VNDurationTriggerPIMPL(float duration) :
		duration_(duration) {

	}

	VNDurationTriggerPIMPL::~VNDurationTriggerPIMPL(void) {

	}

} // namespace Vision
} // namespace LF
