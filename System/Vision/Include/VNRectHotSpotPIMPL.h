#ifndef __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__

#include <DisplayTypes.h>
#include <VNHotSpotPIMPL.h>
#include <VNCoordinateTranslator.h>

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
    virtual bool GetIntegralImage(cv::Mat &img);

    // assumption is that all coordinates passed in to this method
    // are relative to the display coordinate system
    void SetRect(const LeapFrog::Brio::tRect &rect);
    LeapFrog::Brio::tRect GetRect(void) const;

    // translates the lfRect (in display coordinates) to rect_ in vision coordinates
    virtual void UpdateVisionCoordinates(void);

    virtual cv::Rect GetBoundingBox(void) const;
  protected:
    // assumption is that all coordinates passed in to this method
    // are relative to the display coordinate system
    void SetRect(LeapFrog::Brio::S16 left,
		 LeapFrog::Brio::S16 right,
		 LeapFrog::Brio::S16 top,
		 LeapFrog::Brio::S16 bottom);
    cv::Rect ClipRectToImage(cv::Mat &img) const;
    void UpdateNumberOfActivePixels(const cv::Mat &img);

    // stored in vision coordinates
    cv::Rect rect_;
    
    VNCoordinateTranslator *translator_;
  private:
    // stored in display coordinates
    LeapFrog::Brio::tRect lfRect_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNRECTHOTSPOTPIMPL_H__
