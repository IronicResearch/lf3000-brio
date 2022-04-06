#ifndef __VISION_INCLUDE_VNHOTSPOTEVENTMESSAGEPIMPL_H__
#define __VISION_INCLUDE_VNHOTSPOTEVENTMESSAGEPIMPL_H__

#include <vector>

namespace LF {
namespace Vision {
  class VNHotSpot;
  
  class VNHotSpotEventMessagePIMPL {
  public:
    VNHotSpotEventMessagePIMPL(const VNHotSpot* hotSpot);
    VNHotSpotEventMessagePIMPL(std::vector<const VNHotSpot*> hotSpots);
    
    ~VNHotSpotEventMessagePIMPL(void);
    
    const VNHotSpot* hotSpot_;
    std::vector<const VNHotSpot*> hotSpots_;    
  };

}
}

#endif // __VISION_INCLUDE_VNHOTSPOTEVENTMESSAGEPIMPL_H__
