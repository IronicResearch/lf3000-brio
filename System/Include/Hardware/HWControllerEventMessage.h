#ifndef __INCLUDE_HARDWARE_HWCONTROLLEREVENTMESSAGE_H__
#define __INCLUDE_HARDWARE_HWCONTROLLEREVENTMESSAGE_H__

#include <EventMessage.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Hardware {

  // forward declaration
  class HWController;
  class HWControllerEventMessagePIMPL;

  /*!
   * \class HWControllerEventMessage
   *
   * NOTE: For use with LeapTV applications ONLY.
   * HWControllerEventMessage is used to send information back to the application
   * code when HWController events occur.  These messgaes get sent via an
   * IEventListener implemented in the application code.
   */
  class HWControllerEventMessage : public LeapFrog::Brio::IEventMessage {
  public:
    /*!
     * \brief Constructor
     * \param type the event types that will trigger message generation
     * \param hotSpot the HWController associated with the event type
     */
    HWControllerEventMessage(LeapFrog::Brio::tEventType type,
			     const HWController* controller);

    /*!
     * \brief Default destructor
     */
    virtual ~HWControllerEventMessage(void);

    /*!
     * \brief GetController returns the HWController associated with this message
     * \return A const pointer to the HWController associated with this message
     */
    const HWController* GetController(void) const;

    /*!
     * \brief GetSizeInBytes is used to determine the overall size of the message
     * \return The size in bytes, U16, of the message
     */
    LeapFrog::Brio::U16 GetSizeInBytes(void) const;

  private:
    const HWController* controller_;
  };

} // namespace Hardware
} // namespace LF

#endif // __INCLUDE_HARDWARE_HWCONTROLLEREVENTMESSAGE_H__
