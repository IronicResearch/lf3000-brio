#ifndef __VISION_INCLUDE_VNARBITRARYSHAPEHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNARBITRARYSHAPEHOTSPOTPIMPL_H__

#include <DisplayTypes.h>
#include <VNRectHotSpotPIMPL.h>
#include <FontTypes.h>

namespace LF {
namespace Vision {

  class VNArbitraryShapeHotSpotPIMPL : public VNRectHotSpotPIMPL {
  public:
    VNArbitraryShapeHotSpotPIMPL(void);
    VNArbitraryShapeHotSpotPIMPL(const LeapFrog::Brio::tRect& rect,
				 const cv::Mat &filterImage);
    VNArbitraryShapeHotSpotPIMPL(const LeapFrog::Brio::tRect& rect,
				 const LeapFrog::Brio::tFontSurf &filterImage);
    virtual ~VNArbitraryShapeHotSpotPIMPL(void);
    
    virtual void Trigger(cv::Mat &input, const VNHotSpot *hs);

    virtual bool ContainsPoint(const VNPoint &p) const;
    virtual int GetTriggerImage(cv::Mat &img);

    void SetFilterImage(const cv::Mat &filterImage);
    void SetFilterImage(const LeapFrog::Brio::tFontSurf &filterImage);
    cv::Mat GetFilterImage(void) const;

  protected:
    cv::Mat filterImage_;
    cv::Mat tmpMat_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNARBITRARYSHAPEHOTSPOTPIMPL_H__
