#ifndef __INCLUDE_VISION_VNCOMPOUNDTRIGGER_H__
#define __INCLUDE_VISION_VNCOMPOUNDTRIGGER_H__

#include <Vision/VNTrigger.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declaration
  class VNCompoundTriggerPIMPL;
  class VNSpatialTrigger;
  class VNTemporalTriggering;

  /*!
   * \class VNCompoundTrigger
   *
   * NOTE: For use with LeapTV applications only.
   * \brief A VNCompoundTrigger object combines the functionality of a VNSpatialTrigger
   * object and a VNTemporalTriggering object allowing for the tracking of a
   * spatial and temporal triggering event.
   *
   */
  class VNCompoundTrigger : public VNTrigger {
  public:
    /*!
     * Constructor
     * \param spatialTrigger The spatial component of the compound trigger
     * \param temporalTrigger The temporal component of the compound trigger
     */
    VNCompoundTrigger(VNSpatialTrigger *spatialTrigger,
		      VNTemporalTriggering *temporalTrigger);

    /*!
     * Destructor
     */
    virtual ~VNCompoundTrigger(void);

    /*!
     * \brief Triggered is the virtual method required for all VNTrigger objects.  This
     * method is called once per algorithm cycle (VNAlgorithm) to determine if the
     * hot spot(s) using this trigger is in fact triggered.  VNCompoundTrigger objects
     * will first check the spatial
     * \return true if triggered, false if not
     */
    virtual bool Triggered(const VNHotSpot *hotSpot);
  private:
    boost::shared_ptr<VNCompoundTriggerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics and default constuctions
     */
    VNCompoundTrigger(const VNCompoundTrigger&);
    VNCompoundTrigger& operator =(const VNCompoundTrigger&);
    VNCompoundTrigger(void);
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNCOMPOUNDTRIGGER_H__
