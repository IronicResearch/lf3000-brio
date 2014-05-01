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

    // the assumption is the point passed in, p, is in vision coordinates
    // as this methos is exclusively used from inside the Vision library
    virtual bool ContainsPoint(const VNPoint &p) const;
    virtual int GetTriggerImage(cv::Mat &img);

    virtual bool GetIntegralImage(cv::Mat &img);

    void SetRadius(float radius);
    float GetRadius(void) const;
    
    void SetCenter(const VNPoint &center);
    VNPoint GetCenter(void) const;

    virtual void UpdateVisionCoordinates(void); 
  private:
    void CreateMask(void);

    // in display coordinates
    float radius_;
    VNPoint center_;

    // in vision coordinates
    float radiusX_;
    float radiusY_;
    VNPoint visionCenter_;

    cv::Mat tmpMat_;
    cv::Mat mask_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNCIRCLEHOTSPOTPIMPL_H__
