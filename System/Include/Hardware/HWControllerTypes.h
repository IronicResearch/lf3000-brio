#ifndef __INCLUDE_HARDWARE_HWCONTROLLERTYPES_H__
#define __INCLUDE_HARDWARE_HWCONTROLLERTYPES_H__

#include <EventTypes.h>
#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>

using LeapFrog::Brio::tEventType;
using LeapFrog::Brio::tErrType;
namespace LF {
namespace Hardware {

  // forward declaration
  class HWController;

  /*!
   * HW_CONTROLLER_EVENTS
   * \brief the events that can cause a \ref HWControllerEventMessage to fire
   */
#define HW_CONTROLLER_EVENTS						\
  (kHWControllerDataChanged)						\
  (kHWControllerAccelerometerDataChanged)				\
  (kHWControllerDeprecatedEvent)			/*Do not use */		\
  (kHWControllerAnalogStickDataChanged) 				\
  (kHWControllerButtonStateChanged)					\
  (kHWControllerModeChanged)						\
  (kHWControllerLowBattery)						\
  (kHWControllerDisconnected)					\
  (kHWControllerConnected)						\
  (kHWControllerSyncSuccess)					\
  (kHWControllerSyncFailure)
    
BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, LeapFrog::Brio::FirstEvent(LeapFrog::Brio::kGroupController), HW_CONTROLLER_EVENTS)

  /*!
   * \brief all controller events
   */
  
  const LeapFrog::Brio::tEventType kHWAllControllerEvents = LeapFrog::Brio::AllEvents(LeapFrog::Brio::kGroupController);
 

  /*!
   * HW_CONTROLLER_ERRORS
   */
#define HW_CONTROLLER_ERRORS			\
  (kHWControllerEmulationConfigErr)		\
    (kHWControllerNotInWandModeForTracking)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, LeapFrog::Brio::FirstErr(LeapFrog::Brio::kGroupController), HW_CONTROLLER_ERRORS)


  /*!
   * HWController constant definitions
   */
  const LeapFrog::Brio::U8 kHWMaximumNumberOfControllers = 2; //< maximum number of simultaneously connected controllers
  const LeapFrog::Brio::U8 kHWDefaultControllerID = 0;
  
  /*!
   * HWController functionality bit masks
   */
  typedef const LeapFrog::Brio::U32 HWControllerFunctionalityMask;
  static HWControllerFunctionalityMask kHWControllerHasAccelerometer = (1 << 0);
  static HWControllerFunctionalityMask kHWControllerHasAnalogStick   = (1 << 1);
  static HWControllerFunctionalityMask kHWControllerHasButtons       = (1 << 2);
  static HWControllerFunctionalityMask kHWControllerHasLED           = (1 << 3);

  /*!
   * Modes the controller can be in
   */
  typedef enum HWControllerMode {
    kHWControllerNoMode = 0, //< the controller is not in either mode
    kHWControllerMode,       //< the controller is in controller mode
    kHWControllerWandMode    //< the controller is in wand pointing mode
  } HWControllerMode;

  /*!
   * All possible colors of the controller LED in bitmask form
   */
  typedef LeapFrog::Brio::U32 HWControllerLEDColor;
  typedef HWControllerLEDColor HWControllerLEDColorMask;
  static HWControllerLEDColor kHWControllerLEDOff      = 0;
  static HWControllerLEDColor kHWControllerLEDGreen    = (1 << 0);
  static HWControllerLEDColor kHWControllerLEDRed      = (1 << 1);
  static HWControllerLEDColor kHWControllerLEDBlue     = (1 << 2);
  static HWControllerLEDColor kHWControllerLEDYellow   = (1 << 3);
  static HWControllerLEDColor kHWControllerLEDCyan     = (1 << 4);
  static HWControllerLEDColor kHWControllerLEDMagenta = (1 << 5);

  /*!
   * Default LED color for light tracking
   */
  static HWControllerLEDColor kHWControllerLEDDefaultColor = kHWControllerLEDGreen;

  typedef LeapFrog::Brio::U8 HWControllerRGBLEDColor;
  static const HWControllerRGBLEDColor HWControllerLEDGreenRGB[3] = {215, 244, 154};
  /*!
   * TODO: Add other colors
   */

  typedef LeapFrog::Brio::U32 HWControllerButtonMask;
 
} // namespace Hardware
} // namespace LF
#endif // __INCLUDE_HARDWARE_HWCONTROLLERTYPES_H__
