#ifndef __VISION_INCLUDE_VNHOTSPOTEVENTMESSAGEPIMPL_H__
#define __VISION_INCLUDE_VNHOTSPOTEVENTMESSAGEPIMPL_H__

namespace LF {
namespace Vision {
	class VNHotSpot;

	class VNHotSpotEventMessagePIMPL {
	public:
		VNHotSpotEventMessagePIMPL(const VNHotSpot* hotSpot);

		~VNHotSpotEventMessagePIMPL(void);

		const VNHotSpot* hotSpot_;
	};

}
}

#endif // __VISION_INCLUDE_VNHOTSPOTEVENTMESSAGEPIMPL_H__
