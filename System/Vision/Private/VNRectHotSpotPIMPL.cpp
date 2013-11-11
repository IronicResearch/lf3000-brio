#include <VNRectHotSpotPIMPL.h>

namespace LF {
namespace Vision {

	VNRectHotSpotPIMPL::VNRectHotSpotPIMPL(void) {
		rect_.left = 0;
		rect_.right = 0;
		rect_.top = 0;
		rect_.bottom = 0;
	}

	VNRectHotSpotPIMPL::VNRectHotSpotPIMPL(const LeapFrog::Brio::tRect& rect) :
		rect_(rect) {

	}

	VNRectHotSpotPIMPL::~VNRectHotSpotPIMPL(void) {

	}

} // namespace Vision
} // namespace LF
