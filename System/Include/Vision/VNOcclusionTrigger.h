#ifndef __INCLUDE_VISION_VNOCCLUSIONTRIGGER_H__
#define __INCLUDE_VISION_VNOCCLUSIONTRIGGER_H__

#include <Vision/VNTrigger.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  class VNHotSpot;
  class VNOcclusionTriggerPIMPL;

  extern const float kVNDefaultPercentOccludedToTrigger;

  /*!
   * \class VNOcclusionTrigger
   * \brief
   * A VNOcclusionTrigger returns true when the hot spot
   * has at least 'percentOccluded' percent of it's region
   * occluded
   */
  class VNOcclusionTrigger : public VNTrigger {
  public:
    VNOcclusionTrigger(void);
    VNOcclusionTrigger(float percentOccluded);
    virtual ~VNOcclusionTrigger(void);

    virtual bool Triggered(const VNHotSpot& hotSpot);

    void SetOcclusionPercentage(float percentOccluded);
    float GetOcclusionPercentage(void) const;

  private:
    boost::shared_ptr<VNOcclusionTriggerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantic
     */
    VNOcclusionTrigger(const VNOcclusionTrigger& ot);
    VNOcclusionTrigger& operator=(const VNOcclusionTrigger& ot);
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNOCCLUSIONTRIGGER_H__
