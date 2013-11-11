#ifndef __INCLUDE_VISION_VNINTERVALTRIGGER_H__
#define __INCLUDE_VISION_VNINTERVALTRIGGER_H__

#include <Vision/VNTrigger.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  class VNHotSpot;
  class VNIntervalTriggerPIMPL;

  extern const float kVNDefaultIntervalToTrigger;

  /*!
   * \class VNIntervalTrigger
   * \brief 
   * A VNIntervalTrigger allows a triggering event to occur
   * iff the previous triggering even occured at least 'duration'
   * seconds ago.
   */
  class VNIntervalTrigger : public VNTrigger {
  public:
    VNIntervalTrigger(void);
    VNIntervalTrigger(float duration);
    virtual ~VNIntervalTrigger(void);
    
    VNIntervalTrigger(const VNIntervalTrigger& dt);
    VNIntervalTrigger& operator=(const VNIntervalTrigger& dt);

    virtual bool Triggered(const VNHotSpot& hotSpot);

    void SetInterval(float interval);
    float GetInterval(void) const;

  private:
    boost::shared_ptr<VNIntervalTriggerPIMPL> pimpl_;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNINTERVALTRIGGER_H__
