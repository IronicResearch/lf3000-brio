#ifndef __INCLUDE_VISION_VNDURATIONTRIGGERING_H__
#define __INCLUDE_VISION_VNDURATIONTRIGGERING_H__

#include <Vision/VNTemporalTriggering.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declaration
  class VNDurationTriggerPIMPL;

  /*!
   * \class kVNDurationTriggerDefault
   * NOTE: For use with LeapTV applications only.
   * \brief If no value is specified for the duration, a VNDurationTrigger
   * will use this value as the time required to trigger.
   */
  const float kVNDurationTriggerDefault = 0.5; // seconds

  /*!
   * \class VNDurationTrigger
   *
   */
  class VNDurationTrigger : public VNTemporalTriggering {
  public:

    /*!
     * Default constructor
     * \param duration The amount of time a spatial trigger must be triggered
     * before the compound trigger registers an event.  The default value is
     * set to kVNDurationTriggerDefault.
     * The value should >= 0.0f.
     */
    VNDurationTrigger(float duration = kVNDurationTriggerDefault);

    /*!
     * Destructor
     */
    ~VNDurationTrigger(void);

    /*!
     * GetDuration
     * \brief returns the current duration value used for triggering
     */
    float GetDuration(void) const;

    /*!
     * SetDuration
     * \brief Sets the duration value used to cause a temporal trigger event
     * \param The value should >= 0.0f.
     */
    void SetDuration(float duration);

    /*!
     * \brief This method is used from within a VNCompoundTrigger object and
     * will return true if the VNTrigger object (a spatial trigger) is triggered and
     * satisfies the temporal algorithm of the VNTemporalActivator object.
     * \param triggered the response from a call to Triggered of a VNTrigger object
     * \return true if temporally speaking the trigger event is active, false if not
     */
    virtual bool Triggered(bool triggered);

  private:
    boost::shared_ptr<VNDurationTriggerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy symantics
     */
    VNDurationTrigger(const VNDurationTrigger&);
    VNDurationTrigger& operator =(const VNDurationTrigger&);
  };

}
}

#endif // __INCLUDE_VISION_VNDURATIONTRIGGERING_H__
