#include <VNIntervalTriggerPIMPL.h>

const float kVNDefaultIntervalToTrigger = 0.2f; // in seconds

namespace LF {
namespace Vision {

	VNIntervalTriggerPIMPL::VNIntervalTriggerPIMPL(void) :
		interval_(kVNDefaultIntervalToTrigger) {

	}

	VNIntervalTriggerPIMPL::VNIntervalTriggerPIMPL(float interval) :
		interval_(interval) {

	}

	VNIntervalTriggerPIMPL::~VNIntervalTriggerPIMPL(void) {

	}

} // namespace Vision
} // namespace LF
