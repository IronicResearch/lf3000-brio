#include <Vision/VNHotSpotEventMessage.h>
#include <VNHotSpotEventMessagePIMPL.h>

namespace LF {
namespace Vision {

	VNHotSpotEventMessage::VNHotSpotEventMessage(LeapFrog::Brio::tEventType type,
									      	  	 const VNHotSpot* hotSpot) :
		LeapFrog::Brio::IEventMessage(type),
		pimpl_(new VNHotSpotEventMessagePIMPL(hotSpot)) {

	}

	VNHotSpotEventMessage::~VNHotSpotEventMessage(void) {

	}

	const VNHotSpot*
	VNHotSpotEventMessage::GetHotSpot(void) const {
		return pimpl_->hotSpot_;
	}

	LeapFrog::Brio::U16
	VNHotSpotEventMessage::GetSizeInBytes(void) const {
		return sizeof(VNHotSpotEventMessage);
	}
} // namespace Vision
} // namespace LF
