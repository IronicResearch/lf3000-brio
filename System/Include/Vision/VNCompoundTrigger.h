#ifndef __INCLUDE_VISION_VNCOMPOUNDTRIGGER_H__
#define __INCLUDE_VISION_VNCOMPOUNDTRIGGER_H__

#include <Vision/VNTrigger.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace LF {
namespace Vision {

  /*!
   * VNCompoundTriggerType
   * defines the type of compound triggers
   */
  typedef enum VNCompoundTriggerType {
    kVNAndTriggerType, // logical AND trigger
    kVNOrTriggerType,  // logical OR  trigger
    kVNNotTriggerType, // logical NOT trigger
    kVNNoCompoundTriggerTypeSet // not yet set
  } VNCompoundTriggerType;

  class VNHotSpot;
  class VNCompoundTriggerPIMPL;

  /*!
   * \class VNCompoundTrigger
   * \brief
   * A compound trigger is used to combine multiple sub triggers
   * to form a more complex hot spot trigger condition.  There are
   * three types of compound triggers, logical AND, logical OR and
   * logical NOT.  Two or more VNCompoundTriggers can be combined
   * to form a hierarchical, complex compound trigger.
   */
  class VNCompoundTrigger : public VNTrigger {
  public:
    VNCompoundTrigger(void);
    VNCompoundTrigger(VNCompoundTriggerType type,
		      	  	  const std::vector<const VNTrigger*>& subTriggers);
    virtual ~VNCompoundTrigger(void);

    void SetCompoundTriggerType(VNCompoundTriggerType type);
    VNCompoundTriggerType GetCompoundTriggerType(void) const;

    void SetSubTriggers(const std::vector<const VNTrigger*>& subTriggers);
    std::vector<const VNTrigger*>& GetSubTriggers(void) const;

    virtual bool Triggered(const VNHotSpot& hotSpot);

  private:
    boost::shared_ptr<VNCompoundTriggerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     */
    VNCompoundTrigger(const VNCompoundTrigger& dt);
    VNCompoundTrigger& operator=(const VNCompoundTrigger& dt);
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNCOMPOUNDTRIGGER_H__
