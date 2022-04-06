#include <VNHotSpotEventMessagePIMPL.h>
#include <Vision/VNHotSpot.h>

namespace LF {
namespace Vision {

  VNHotSpotEventMessagePIMPL::VNHotSpotEventMessagePIMPL(const VNHotSpot* hotSpot) :
    hotSpot_(hotSpot),
    hotSpots_(1,hotSpot) {
  }
  
  VNHotSpotEventMessagePIMPL::VNHotSpotEventMessagePIMPL(std::vector<const VNHotSpot*> hotSpots) :
    hotSpot_(NULL),
    hotSpots_(hotSpots) {
  }
  
  VNHotSpotEventMessagePIMPL::~VNHotSpotEventMessagePIMPL(void) {
    
  }

}
}

