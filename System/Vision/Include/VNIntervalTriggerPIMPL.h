#ifndef __VISION_INCLUDE_VNINTERVALTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNINTERVALTRIGGERPIMPL_H__

#include <boost/timer.hpp>

namespace LF {
namespace Vision {
  
  class VNIntervalTriggerPIMPL {
  public:
    VNIntervalTriggerPIMPL(float interval);
    virtual ~VNIntervalTriggerPIMPL(void);
    
    float GetInterval(void) const;

    void SetInterval(float interval);

    bool Triggered(bool spatiallyTriggered);

    bool firstTrigger_;
    boost::timer timer_;

  private:
    float interval_;
  };
    
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNOINTERVALTRIGGERPIMPL_H__
