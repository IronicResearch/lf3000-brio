#ifndef __VISION_INCLUDE_VNINTERVALTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNINTERVALTRIGGERPIMPL_H__

namespace LF {
namespace Vision {

	class VNIntervalTriggerPIMPL {
	public:
		VNIntervalTriggerPIMPL(void);
		VNIntervalTriggerPIMPL(float interval);
		virtual ~VNIntervalTriggerPIMPL(void);

		float interval_;
	};

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNOINTERVALTRIGGERPIMPL_H__
