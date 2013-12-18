#ifndef __INCLUDE_HARDWARE_HWANALOGSTICKTYPES_H__
#define __INCLUDE_HARDWARE_HWANALOGSTICKTYPES_H__
//		Defines types for the AnalogStick module.

#include <EventTypes.h>
#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>

using LeapFrog::Brio::tEventType;
using LeapFrog::Brio::tErrType;
namespace LF {
namespace Hardware {

	
  /*!
   * HW_ANALOGSTICK_EVENTS
   * \brief the events that can cause a HWAnalaogStickMessage to fire
   */
#define HW_ANALOGSTICK_EVENTS						\
  (kHWAnalogStickDataChanged)					

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, LeapFrog::Brio::FirstEvent(LeapFrog::Brio::kGroupAnalogStick), HW_ANALOGSTICK_EVENTS)

  /*!
   * \brief all analog stick events
   */
  
  const LeapFrog::Brio::tEventType kHWAllAnalogStickEvents = LeapFrog::Brio::AllEvents(LeapFrog::Brio::kGroupAnalogStick);
 

  /*!
   * HW_ANALOGSTICK_ERRORS
   */
#define HW_ANALOGSTICK_ERRORS			\
 (kHWAnalogStickEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, LeapFrog::Brio::FirstErr(LeapFrog::Brio::kGroupAnalogStick), HW_ANALOGSTICK_ERRORS)


  /*!
   * HWAnalogStick constant definitions
   */
  const LeapFrog::Brio::U8 kHWAnalogStickDeadZoneMin     = 0;   ///< minimum dead zone
  const LeapFrog::Brio::U8 kHWAnalogStickDeadZoneMax     = 127;  ///< maximum dead zone
  const LeapFrog::Brio::U8 kHWAnalogStickDeadZoneDefault = 0;    ///< default dead zone

  /*!
   *
   */
  const LeapFrog::Brio::U8 kHWDefaultAnalogStickID = 0;

  /*!
   * enumerated type for SetAnalogStickMode/GetAnalogStickMode
   */
  enum tHWAnalogStickMode {
    kHWAnalogStickModeDisable, ///< disabled analog stick
    kHWAnalogStickModeAnalog,  ///< data is continuous between -127...127
    kHWAnalogStickModeDPad     ///< data is binary dpad, kButtonLeft, kButtonRight, kButtonUp, kButtonDown
  };
 
  /*!
   * \brief the analog stick data, representing current position and current time
   */
  struct tHWAnalogStickData {
    float x; ///< -1 .. 1 in kHWAnalogStickModeAnalog. 
    float y; ///< -1 .. 1 in kHWAnalogStickModeAnalog. 

    LeapFrog::Brio::U8 id; ///< a unique identifier for the analog stick

    struct timeVal {
      LeapFrog::Brio::S32 seconds;
      LeapFrog::Brio::S32 microSeconds;
    } time;
  };
 
  
  /*!
   * \class HWAnalogStickMessage 
   * \brief The message class that gets passed back to the event listener
   * when a HWAnalogStick event occurs
   */
  class HWAnalogStickMessage : public LeapFrog::Brio::IEventMessage {
  public:
    HWAnalogStickMessage(const tHWAnalogStickData& data);
    virtual LeapFrog::Brio::U16 GetSizeInBytes(void) const;
    
    tHWAnalogStickData 	GetAnalogStickData(void) const;
  private:
    tHWAnalogStickData 	data_;
  };
 
} // namespace Hardware
} // namespace LF
#endif // __INCLUDE_HARDWARE_HWANALOGSTICKTYPES_H__
