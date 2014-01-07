#ifndef __INCLUDE_VISION_VNINTERVALTRIGGER_H__
#define __INCLUDE_VISION_VNINTERVALTRIGGER_H__

#include <Vision/VNTemporalTriggering.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declaration
  class VNIntervalTriggerPIMPL;

  /*!
   * kVNIntervalTriggerDefault
   * \brief If no value is specified for the interval, a VNIntervalTrigger
   * will use this value as the time required in between trigger events
   */
  const float kVNIntervalTriggerDefault = 0.5; // seconds

  /*!
   * \class VNIntervalTrigger
   *
   */
  class VNIntervalTrigger : public VNTemporalTriggering { 
  public:

    /*!
     * Default constructor
     * \param interval The amount of time a spatial trigger must not be triggered
     * before the compund trigger registers an event.  The default value is
     * set to kVNIntervalTriggerDefault
     */
    VNIntervalTrigger(float interval = kVNIntervalTriggerDefault);

    /*!
     * Destructor
     */
    ~VNIntervalTrigger(void);

    /*!
     * GetInterval
     * \brief returns the current interval value used for triggering
     */
    float GetInterval(void) const;

    /*!
     * SetInterval
     * \brief Sets the interval value used to cause a temporal trigger event
     */
    void SetInterval(float duration);

    /*!
     * \brief This method is used from within a VNCompoundTrigger object and
     * will return true if the VNTrigger object (a spatial trigger) is triggered and
     * satisfies the temporal algorithm of the VNTemporalActivator object.
     * \param triggered the response from a call to Triggered of a VNTrigger object
     * \return true if temporally speaking the trigger event is active, false if not
     */
    virtual bool Triggered(bool triggered);

  private:
    boost::shared_ptr<VNIntervalTriggerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy symantics
     */
    VNIntervalTrigger(const VNIntervalTrigger&);
    VNIntervalTrigger& operator =(const VNIntervalTrigger&);
  };

}
}

#endif // __INCLUDE_VISION_VNINTERVALTRIGGER_H__
