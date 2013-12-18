#ifndef __VISION_INCLUDE_VNTRIGGERPIMPL_H__
#define __VISION_INCLUDE_VNTRIGGERPIMPL_H__

#include <opencv2/core/core.hpp>
#include <Vision/VNVisionTypes.h>

namespace LF {
namespace Vision {


  class VNTriggerPIMPL {
  public:
    VNTriggerPIMPL(void);
    virtual ~VNTriggerPIMPL(void);
    void SetInputData(VNPoint point,
		      cv::Rect& rect,
		      cv::Mat& img);

  protected:
    VNPoint triggerPoint_;
    cv::Rect triggerRect_;
    cv::Mat triggerImg_;
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNTRIGGERPIMPL_H__
