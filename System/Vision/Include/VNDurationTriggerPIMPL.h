#ifndef __VISION_INCLUDE_VNDURATIONTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNDURATIONTRIGGERPIMPL_H__

#include <boost/timer.hpp>

namespace LF {
namespace Vision {

  class VNDurationTriggerPIMPL {
  public:
    VNDurationTriggerPIMPL(float duration);
    virtual ~VNDurationTriggerPIMPL(void);
    
    bool Triggered(bool spatiallyTriggered);

    float duration_;

    bool wasSpatiallyTriggered_;
    boost::timer timer_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNDURATIONTRIGGERPIMPL_H__
