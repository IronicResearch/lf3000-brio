#include <Vision/VNHotSpotEventMessage.h>
#include <VNHotSpotEventMessagePIMPL.h>

namespace LF {
namespace Vision {

  VNHotSpotEventMessage::VNHotSpotEventMessage(LeapFrog::Brio::tEventType type,
					       const VNHotSpot* hotSpot) :
    LeapFrog::Brio::IEventMessage(type),
    pimpl_(new VNHotSpotEventMessagePIMPL(hotSpot)) {
    
  }
  
  VNHotSpotEventMessage::VNHotSpotEventMessage(LeapFrog::Brio::tEventType type,
					       std::vector<const VNHotSpot*> hotSpots) :
    LeapFrog::Brio::IEventMessage(type),
    pimpl_(new VNHotSpotEventMessagePIMPL(hotSpots)) {
    
  }
  
  VNHotSpotEventMessage::~VNHotSpotEventMessage(void) {
    
  }
  
  const VNHotSpot*
  VNHotSpotEventMessage::GetHotSpot(void) const {
    if (pimpl_) {
      return pimpl_->hotSpot_;
    }
    return NULL;
  }
  
  std::vector<const VNHotSpot*>
  VNHotSpotEventMessage::GetHotSpots(void) const {
    if (pimpl_) {
      return pimpl_->hotSpots_;
    }
    return std::vector<const VNHotSpot*>();
  }
  
  LeapFrog::Brio::U16
  VNHotSpotEventMessage::GetSizeInBytes(void) const {
    return sizeof(VNHotSpotEventMessage);
  }
} // namespace Vision
} // namespace LF
