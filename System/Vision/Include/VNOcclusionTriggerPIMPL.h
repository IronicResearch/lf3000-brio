#ifndef __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__

namespace LF {
namespace Vision {

	class VNOcclusionTriggerPIMPL {
	public:
		VNOcclusionTriggerPIMPL(void);
		VNOcclusionTriggerPIMPL(float percentOccludedToTrigger);
		virtual ~VNOcclusionTriggerPIMPL(void);

		float percentOccludedToTrigger_;
	};

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNOCCLUSIONTRIGGERPIMPL_H__
