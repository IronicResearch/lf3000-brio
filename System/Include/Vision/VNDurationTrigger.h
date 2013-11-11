#ifndef __INCLUDE_VISION_VNDURATIONTRIGGER_H__
#define __INCLUDE_VISION_VNDURATIONTRIGGER_H__

#include <Vision/VNTrigger.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  class VNHotSpot;
  class VNDurationTriggerPIMPL;

  extern const float kVNDefaultDurationToTrigger;

  /*!
   * \class VNDurationTrigger
   * \brief
   * A VNDurationTrigger will return true when the hot spot
   * has detected a trigger event for at least 'duration'
   * seconds.
   */
  class VNDurationTrigger : public VNTrigger {
  public:
    VNDurationTrigger(void);
    VNDurationTrigger(float duration);
    virtual ~VNDurationTrigger(void);
    
    VNDurationTrigger(const VNDurationTrigger& dt);
    VNDurationTrigger& operator=(const VNDurationTrigger& dt);

    virtual bool Triggered(const VNHotSpot& hotSpot);

    void SetDuration(float duration);
    float GetDuration(void) const;

  private:
    boost::shared_ptr<VNDurationTriggerPIMPL> pimpl_;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNDURATIONTRIGGER_H__
