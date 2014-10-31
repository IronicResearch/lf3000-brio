#ifndef __INCLUDE_VISION_VNVISIONTYPES_H__
#define __INCLUDE_VISION_VNVISIONTYPES_H__

#include <SystemTypes.h>
#include <SystemEvents.h>
#include <SystemErrors.h>
#include <GroupEnumeration.h>
#include <StringTypes.h>
#include <vector>

// Need to include this for event types
using namespace LeapFrog::Brio;

namespace LF {
namespace Vision {

  /*!
   * VN_VISION_EVENTS
   *
   * NOTE: For use with LeapTV applications ONLY.
   *
   * \brief the events that can cause VNHotSpotEventMessages to fire
   * kVNHotSpotTriggeredEvent
   *  - a single notify per hot spot that is triggered
   * kVNHotSpotTriggerChangeEvent
   *  - a single notify per hot spot that changes it's triggered state
   * kVNHotSpotGroupTriggeredEvent
   *  - a single notify per vision iteration containing all hot spots that were triggered
   * kVNHotSpotGroupTriggerChangeEvent
   *  - a single notify per vision iteration containing all hot spots who's triggered state changed
   */
#define VN_VISION_EVENTS		\
  (kVNHotSpotTriggeredEvent)		\
  (kVNHotSpotTriggerChangeEvent)	\
  (kVNHotSpotGroupTriggeredEvent)	\
  (kVNHotSpotGroupTriggerChangeEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, LeapFrog::Brio::FirstEvent(LeapFrog::Brio::kGroupVision), VN_VISION_EVENTS)

#define VN_VISION_ERRORS			\
  (kVNCameraDoesNotSupportRequiredVisionFormat)		\
  (kVNVideoSurfaceNotOfCorrectSizeForVisionCapture)	\
  (kVNVideoCaptureFailed)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupVision), VN_VISION_ERRORS)

  /*!
   * All possible vision events that can trigger an event message
   */
  const LeapFrog::Brio::tEventType kVNAllVisionEvents = LeapFrog::Brio::AllEvents(LeapFrog::Brio::kGroupVision);

  /*!
   * The default wand id
   */
  static const LeapFrog::Brio::U32 kVNDefaultWandID = 0;

  /*!
   * VNPixelType
   * 8-bit pixel data, typedef'ed to U8
   */
  typedef LeapFrog::Brio::U8 VNPixelType;

  /*!
   * The minimum value a pixel can take on
   */
  static const VNPixelType kVNMinPixelValue = 0;
  /*!
   * The maximum value a pixel can take on
   */
  static const VNPixelType kVNMaxPixelValue = 255;

  /*!
   * A list of key-value pairs representing potential input for vision classes
   */
  typedef std::pair<LeapFrog::Brio::CString, float> VNInputParameter;
  typedef std::vector<VNInputParameter> VNInputParameters;

  /*!
   * \class VNPoint
   *
   * \brief A basic 2d point class.  Primarily used to represent
   * onscreen coordinates for wand tracking
   */
  class VNPoint {
  public:
    /*!
     * Default constructor, set to 0,0 point
     */
    VNPoint(void) :
      x(0), y(0) { }
    /*!
     * Constructor
     * \param x_ the x value of the point
     * \param y_ the y value of the point
     */
  VNPoint(LeapFrog::Brio::S16 x_, LeapFrog::Brio::S16 y_):
      x(x_), y(y_) { }

    /*!
     * The x value of the point, publicly available
     */
      LeapFrog::Brio::S16 x;
    /*!
     * The y value of the point, publicly available
     */
      LeapFrog::Brio::S16 y;
  };

  /*!
   * Frame size constants
   */
  static const LeapFrog::Brio::U16 kVNVGAWidth   = 640;
  static const LeapFrog::Brio::U16 kVNVGAHeight  = 480;
  static const LeapFrog::Brio::U16 kVNQVGAWidth  = 320;
  static const LeapFrog::Brio::U16 kVNQVGAHeight = 240;

}
}

#endif // __INCLUDE_VISION_VNVISIONTYPES_H__
