#ifndef __VISION_INCLUDE_VNCOMPOUNDTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNCOMPOUNDTRIGGERPIMPL_H__

#include <Vision/VNCompoundTrigger.h>

namespace LF {
namespace Vision {

	class VNCompoundTriggerPIMPL {
	public:
		VNCompoundTriggerPIMPL(void);
		VNCompoundTriggerPIMPL(VNCompoundTriggerType type,
							   const std::vector<const VNTrigger*>& subTriggers);
		virtual ~VNCompoundTriggerPIMPL(void);

		VNCompoundTriggerType type_;
		std::vector<const VNTrigger*> triggers_;
	};

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNCOMPOUNDTRIGGERPIMPL_H__
