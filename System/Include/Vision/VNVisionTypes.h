#ifndef __INCLUDE_VISION_VNVISIONTYPES_H__
#define __INCLUDE_VISION_VNVISIONTYPES_H__

#include <SystemTypes.h>
#include <SystemEvents.h>
#include <GroupEnumeration.h>

using namespace LeapFrog::Brio;

namespace LF {
namespace Vision {

#define VN_VISION_EVENTS			\
  (kVNHotSpotTriggeredEvent)		\
  (kVNHotSpotTriggerChangeEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, LeapFrog::Brio::FirstEvent(LeapFrog::Brio::kGroupVision), VN_VISION_EVENTS)

const LeapFrog::Brio::tEventType kVNAllVisionEvents = LeapFrog::Brio::AllEvents(LeapFrog::Brio::kGroupVision);

}
}

#endif // __INCLUDE_VISION_VNVISIONTYPES_H__
