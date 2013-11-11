#ifndef __VISION_INCLUDE_VNHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNHOTSPOTPIMPL_H__

#include <Vision/VNTrigger.h>
#include <SystemTypes.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

	class VNHotSpotPIMPL {
	public:
		VNHotSpotPIMPL(void);
		virtual ~VNHotSpotPIMPL(void);

		VNTrigger* trigger_;
		LeapFrog::Brio::U32 tag_;
		cv::Mat triggerImage_;
		bool isTriggered_;
	};

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNHOTSPOTPIMPL_H__
