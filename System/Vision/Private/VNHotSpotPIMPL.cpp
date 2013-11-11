#include <VNHotSpotPIMPL.h>

const LeapFrog::Brio::U32 kVNDefaultHotSpotTag = 0;

namespace LF {
namespace Vision {

	VNHotSpotPIMPL::VNHotSpotPIMPL(void) :
		trigger_(NULL),
		tag_(kVNDefaultHotSpotTag),
		isTriggered_(false) {

	}

	VNHotSpotPIMPL::~VNHotSpotPIMPL(void) {

	}



} // namespace Vision
} // namespace LF
