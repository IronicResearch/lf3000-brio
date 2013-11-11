#ifndef __INCLUDE_VISION_VNHOTSPOT_H__
#define __INCLUDE_VISION_VNHOTSPOT_H__

#include <boost/shared_ptr.hpp>
#include <DisplayTypes.h>
#include <Vision/VNTrigger.h>

namespace LF {
namespace Vision {

  extern const LeapFrog::Brio::U32 kVNDefaultHotSpotTag;

  class VNOcclusionTrigger;
  class VNRectHotSpot;
  class VNHotSpotPIMPL;

  class VNHotSpot {
  public:
	virtual void Trigger(void *input) const = 0;

	bool IsTriggered(void) const;

    void SetTrigger(VNTrigger *trigger);
    VNTrigger* GetTrigger(void) const;

    void SetTag(LeapFrog::Brio::U32 tag);
    LeapFrog::Brio::U32 GetTag(void) const;

  private:
    boost::shared_ptr<VNHotSpotPIMPL> pimpl_;

    /*!
     * Explicitly disable unintended inheritance
     */
    VNHotSpot(void);
    virtual ~VNHotSpot(void);

    /*!
     * Explicitly disable copy semantic
     */
    VNHotSpot(const VNHotSpot& hotSpot);
    VNHotSpot& operator=(const VNHotSpot& hotSpot);
    
    friend class VNOcclusionTrigger;
    friend class VNRectHotSpot;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNHOTSPOT_H__
