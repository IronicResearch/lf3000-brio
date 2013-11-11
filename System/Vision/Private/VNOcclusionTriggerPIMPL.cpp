#include <VNOcclusionTriggerPIMPL.h>

const float kVNDefaultPercentOccludedToTrigger = 0.2f;

namespace LF {
namespace Vision {

	VNOcclusionTriggerPIMPL::VNOcclusionTriggerPIMPL(void) :
		percentOccludedToTrigger_(kVNDefaultPercentOccludedToTrigger) {

	}

	VNOcclusionTriggerPIMPL::VNOcclusionTriggerPIMPL(float percentOccludedToTrigger) :
		percentOccludedToTrigger_(percentOccludedToTrigger) {

	}

	VNOcclusionTriggerPIMPL::~VNOcclusionTriggerPIMPL(void) {

	}

} // namespace Vision
} // namespace LF
