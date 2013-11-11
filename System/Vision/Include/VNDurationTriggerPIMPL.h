#ifndef __VISION_INCLUDE_VNDURATIONTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNDURATIONTRIGGERPIMPL_H__

namespace LF {
namespace Vision {

	class VNDurationTriggerPIMPL {
	public:
		VNDurationTriggerPIMPL(void);
		VNDurationTriggerPIMPL(float duration);
		virtual ~VNDurationTriggerPIMPL(void);

		float duration_;
	};

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNDURATIONTRIGGERPIMPL_H__
