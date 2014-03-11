#ifndef __VISION_INCLUDE_VNINTERVALTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNINTERVALTRIGGERPIMPL_H__

#include <boost/timer.hpp>

namespace LF {
namespace Vision {
  
  class VNIntervalTriggerPIMPL {
  public:
    VNIntervalTriggerPIMPL(float interval);
    virtual ~VNIntervalTriggerPIMPL(void);
    
    bool Triggered(bool spatiallyTriggered);
    
    float interval_;
    
    bool firstTrigger_;
    boost::timer timer_;
  };
    
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNOINTERVALTRIGGERPIMPL_H__
