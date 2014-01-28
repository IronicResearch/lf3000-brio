#ifndef __VISION_INCLUDE_VNCIRCLEHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNCIRCLEHOTSPOTPIMPL_H__

#include <DisplayTypes.h>
#include <VNRectHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  class VNCircleHotSpotPIMPL : public VNRectHotSpotPIMPL {
  public:
    VNCircleHotSpotPIMPL(const VNPoint &center,
			 float radius);
    virtual ~VNCircleHotSpotPIMPL(void);
    
    virtual void Trigger(cv::Mat &input, const VNHotSpot *hs);

    virtual bool ContainsPoint(const VNPoint &p) const;
    virtual int GetTriggerImage(cv::Mat &img);

    void SetRadius(float radius);
    float GetRadius(void) const;
    
    VNPoint center_;

  private:
    void CreateMask(void);

    float radius_;
    float r2_;
    cv::Mat tmpMat_;
    cv::Mat mask_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNCIRCLEHOTSPOTPIMPL_H__
