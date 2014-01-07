#ifndef __INCLUDE_VISION_VNOCCLUSIONTRIGGER_H__
#define __INCLUDE_VISION_VNOCCLUSIONTRIGGER_H__

#include <Vision/VNSpatialTrigger.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declarations
  class VNHotSpot;
  class VNOcclusionTriggerPIMPL;

  /*!
   * kVNDefaultPercentOccludedToTrigger
   * If the percentOccluded is not specified for this trigger it is set
   * to the value of this constant
   */
  extern const float kVNDefaultPercentOccludedToTrigger;

  /*!
   * \class VNOcclusionTrigger
   *
   * \brief A VNOcclusionTrigger object is used to determine when a hot spot is triggered
   * based on the percentage of the hot spot that is occluded during foreground/background
   * segmentation.  This class is typically used in conjunction with the VNVirtualTouch 
   * algorithm.  
   */
  class VNOcclusionTrigger : public VNSpatialTrigger {
  public:

    /*!
     * \brief Default constructor
     * \param percentOccluded the percentage of the hot spot that must be occluded
     * in order to cause a triggering event to occur.  If no value is specified 
     * percentOccluded defaults to kVNDefaultPercentOccludedToTrigger
     */
    VNOcclusionTrigger(float percentOccluded = kVNDefaultPercentOccludedToTrigger);

    /*!
     * Default destructor
     */
    virtual ~VNOcclusionTrigger(void);

    /*!
     * \brief Triggered is the virtual method required for all VNTrigger objects.  This
     * method is called once per algorithm cycle (VNAlgorithm) to determine if the 
     * hot spot(s) using this trigger is in fact triggered.
     * \return true if triggered, false if not
     */
    virtual bool Triggered(const VNHotSpot *hotSpot);

    /*!
     * \biref SetOcclusionPercentage 
     * \param percentOccluded represents the percentage of the hot spot that must be 
     * occluded in order to cause a triggering event to occur
     */
    void SetOcclusionPercentage(float percentOccluded);

    /*!
     * \biref GetOcclusionPercentage
     * \return the float value representing the percentage of the hot spot that
     * must be occluded in order to cause a triggering event to occur
     */
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
