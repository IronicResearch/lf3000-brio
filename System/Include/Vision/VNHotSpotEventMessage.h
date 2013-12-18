#ifndef __INCLUDE_VISION_VNHOTSPOTEVENTMESSAGE_H__
#define __INCLUDE_VISION_VNHOTSPOTEVENTMESSAGE_H__

#include <EventMessage.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declaration
  class VNHotSpot;
  class VNHotSpotEventMessagePIMPL;

  /*!
   * \class VNHotSpotEventMessage
   *
   * VNHotSpotEventMessage is used to send information back to the application
   * code when VNHotSpots are triggered.  These messgaes get sent via an 
   * IEventListener implemented in the application code.
   */
  class VNHotSpotEventMessage : public LeapFrog::Brio::IEventMessage {
  public:
    /*!
     * \brief Constructor
     * \param type the event types that will trigger message generation
     * \param hotSpot the VNHotSpot associated with the event type
     */
    VNHotSpotEventMessage(LeapFrog::Brio::tEventType type,
			  const VNHotSpot* hotSpot);

    /*!
     * \brief Default destructor
     */
    virtual ~VNHotSpotEventMessage(void);
    
    /*!
     * \brief GetHotSpot returns the VNHotSpot associated with this message
     * \return A const pointer to the VNHotSpot associated with this message
     */
    const VNHotSpot* GetHotSpot(void) const;

    /*!
     * \brief GetSizeInBytes is used to determine the overall size of the message
     * \return The size in bytes, U16, of the message
     */
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
