#ifndef __INCLUDE_VISION_VNRECTHOTSPOT_H__
#define __INCLUDE_VISION_VNRECTHOTSPOT_H__

#include <boost/shared_ptr.hpp>
#include <Vision/VNHotSpot.h>

namespace LF {
namespace Vision {

  class VNVisionMPI;
  class VNRectHotSpotPIMPL;
  class VNTrigger;

  class VNRectHotSpot : public VNHotSpot {
  public:
    VNRectHotSpot(void);
    VNRectHotSpot(const LeapFrog::Brio::tRect& rect);
    virtual ~VNRectHotSpot(void);
    
    void Trigger(void *input) const;

    void SetRect(const LeapFrog::Brio::tRect& rect);
    LeapFrog::Brio::tRect& GetRect(void) const;

  private:
    boost::shared_ptr<VNRectHotSpotPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantic
     */
    VNRectHotSpot(const VNRectHotSpot& hotSpot);
    VNRectHotSpot& operator=(const VNRectHotSpot& hotSpot);

    friend class VNVisionMPI;
    friend class VNTrigger;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNRECTHOTSPOT_H__
