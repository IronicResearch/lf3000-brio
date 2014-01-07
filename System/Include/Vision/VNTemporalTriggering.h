#ifndef __INCLUDE_VISION_VNTEMPORALTRIGGERING_H__
#define __INCLUDE_VISION_VNTEMPORALTRIGGERING_H__

namespace LF {
namespace Vision {
  
  /*!
   * \class VNTemporalTriggering
   *
   * Virtual base class for all temporally related triggering logic.  A VNTemporalTriggering
   * operates in conjunction with a NVTrigger object within a VNCompoundTrigger to determine
   * if a VNHotSpot object has been triggered both spatially and temporally.
   */
  class VNTemporalTriggering {
  public:

    /*!
     * \brief This method is used from within a VNCompoundTrigger object and
     * will return true if the VNTrigger object (a spatial trigger) is triggered and
     * satisfies the temporal algorithm of the VNTemporalActivator object.
     * \param triggered the response from a call to Triggered of a VNTrigger object
     * \return true if temporally speaking the trigger event is active, false if not
     */
    virtual bool Triggered(bool triggered) = 0;
  };
}
}

#endif // __INCLUDE_VISION_VNTEMPORALTRIGGERING_H__
