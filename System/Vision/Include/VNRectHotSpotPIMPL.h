#ifndef __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__

#include <DisplayTypes.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  class VNRectHotSpotPIMPL : public VNHotSpotPIMPL {
  public:
    VNRectHotSpotPIMPL(void);
    VNRectHotSpotPIMPL(const LeapFrog::Brio::tRect& rect);
    virtual ~VNRectHotSpotPIMPL(void);
    
    virtual void Trigger(cv::Mat &input, const VNHotSpot *hs);

    virtual bool ContainsPoint(const VNPoint &p) const;
    virtual int GetTriggerImage(cv::Mat &img);

    void SetRect(const LeapFrog::Brio::tRect &rect);
    LeapFrog::Brio::tRect GetRect(void) const;

  protected:
    void SetRect(LeapFrog::Brio::S16 left,
		 LeapFrog::Brio::S16 right,
		 LeapFrog::Brio::S16 top,
		 LeapFrog::Brio::S16 bottom);
    cv::Rect ClipRectToImage(cv::Mat &img) const;
    void UpdateNumberOfActivePixels(const cv::Mat &img);

    cv::Rect rect_;
    
  private:
    LeapFrog::Brio::tRect lfRect_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__
