#ifndef __INCLUDE_VISION_VNTRIGGER_H__
#define __INCLUDE_VISION_VNTRIGGER_H__

namespace LF {
namespace Vision {
  
class VNHotSpot;
  class VNTrigger {
  public:
    virtual bool Triggered(const VNHotSpot& hotSpot) = 0;
  };
}
}

#endif // __INCLUDE_VISION_VNTRIGGER_H__
