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
   * NOTE: For use with LeapTV applications only.
   *
   * \brief A VNOcclusionTrigger object is used to determine when a hot spot is triggered
   * based on the percentage of the hot spot that is occluded during foreground/background
   * segmentation.  This class is typically used in conjunction with the VNVirtualTouch
   * algorithm.
   *
   * NOTE: If the hot spot is only partially visible, as in when part of it is off screen,
   * only the pixels in the visible portion of the hot spot are considered for occlusiong
   * but the reference size for the hot spot is still the original size of the hot spot.
   * For instance, if you have a rectangular hot spot of size 20x20 and are requesting
   * 20% occludion to trigger you will need at least 80 pixels occluded no matter how much
   * of the rectangle is visible on the screen.
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
     * \biref SetOcclusionTriggerPercentage
     * \param percentOccluded represents the percentage of the hot spot that must be
     * occluded in order to cause a triggering event to occur
     */
    void SetOcclusionTriggerPercentage(float percentOccluded);

    /*!
     * \biref GetOcclusionTriggerPercentage
     * \return the float value representing the percentage of the hot spot that
     * must be occluded in order to cause a triggering event to occur
     */
    float GetOcclusionTriggerPercentage(void) const;

    /*!
     * \brief GetPercentOccluded
     * A trigger may be used for multiple VNHotSpots, querying this
     * value will only give you the percent occluded from the most recent
     * hot spot that used this trigger.  If you need to query the percent
     * occluded for individual hot spots you will need a unique VNOcclusionTrigger
     * for each hot spot.
     * \return the float value of the most recent occlusion calculation
     * for the most recent hot spot.
     */
    float GetPercentOccluded(void) const;

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
