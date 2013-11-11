#ifndef __INCLUDE_VISION_VNHOTSPOTEVENTMESSAGE_H__
#define __INCLUDE_VISION_VNHOTSPOTEVENTMESSAGE_H__

#include <EventMessage.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  class VNHotSpot;
  class VNHotSpotEventMessagePIMPL;
  class VNHotSpotEventMessage : public LeapFrog::Brio::IEventMessage {
  public:
    VNHotSpotEventMessage(LeapFrog::Brio::tEventType,
    				 	  const VNHotSpot* hotSpot);
    virtual ~VNHotSpotEventMessage(void);
    
    const VNHotSpot* GetHotSpot(void) const;
    LeapFrog::Brio::U16 GetSizeInBytes(void) const;

  private:
    boost::shared_ptr<VNHotSpotEventMessagePIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     */
    VNHotSpotEventMessage(const VNHotSpotEventMessage&);
    VNHotSpotEventMessage& operator=(const VNHotSpotEventMessage&);
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNHOTSPOTEVENTMESSAGE_H__
