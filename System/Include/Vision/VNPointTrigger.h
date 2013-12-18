#ifndef __INCLUDE_VISION_VNPOINTTRIGGER_H__
#define __INCLUDE_VISION_VNPOINTTRIGGER_H__

#include <Vision/VNTrigger.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declarations
  class VNHotSpot;
  class VNPointTriggerPIMPL;

  /*!
   * \class VNPointTrigger
   *
   * \brief A VNPointTrigger object is used to determine when a hot spot is triggered
   * based on if a point of interest, defined by the algorithm, is within the hot spot.
   * This type of trigger is typically used in conjunction with the VNWandTracking 
   * algorithm
   */
  class VNPointTrigger : public VNTrigger {
  public:

    /*!
     * Default constructor
     */
    VNPointTrigger(void);

    /*!
     * Default destructor
     */
    virtual ~VNPointTrigger(void);

    /*!
     * \brief Triggered is the virtual method required for all VNTrigger objects.  This
     * method is called once per algorithm cycle (VNAlgorithm) to determine if the 
     * hot spot(s) using this trigger is in fact triggered.
     * \return true if triggered, false if not
     */
    virtual bool Triggered(void);

  private:
    boost::shared_ptr<VNPointTriggerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantic
     */
    VNPointTrigger(const VNPointTrigger& ot);
    VNPointTrigger& operator=(const VNPointTrigger& ot);
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNOCCLUSIONTRIGGER_H__
