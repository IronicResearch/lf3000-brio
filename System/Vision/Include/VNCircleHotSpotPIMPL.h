#ifndef __VISION_INCLUDE_VNCIRCLEHOTSPOTPIMPL_H__
#define __VISION_INCLUDE_VNCIRCLEHOTSPOTPIMPL_H__

#include <DisplayTypes.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

  class VNCircleHotSpotPIMPL : public VNHotSpotPIMPL {
  public:
    VNCircleHotSpotPIMPL(const VNPoint &center,
			 float radius);
    virtual ~VNCircleHotSpotPIMPL(void);
    
    void Trigger(void *input, const VNHotSpot *hs);

    virtual bool ContainsPoint(VNPoint &p);
    virtual int GetTriggerImage(cv::Mat &img);

    void SetRadius(float radius);
    float GetRadius(void) const;
    
    VNPoint center_;

  private:
    float radius_;
    float r2_;
    int numPixels_;
  };
  
} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNCIRCLEHOTSPOTPIMPL_H__
