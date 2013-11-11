#include <VNCompoundTriggerPIMPL.h>

namespace LF {
namespace Vision {

	VNCompoundTriggerPIMPL::VNCompoundTriggerPIMPL(void) :
		type_(kVNNoCompoundTriggerTypeSet) {

	}

	VNCompoundTriggerPIMPL::VNCompoundTriggerPIMPL(VNCompoundTriggerType type,
												   const std::vector<const VNTrigger*>& subTriggers) :
		type_(type),
		triggers_(subTriggers) {

	}

	VNCompoundTriggerPIMPL::~VNCompoundTriggerPIMPL(void) {

	}

} // namespace Vision
} // namespace LF
