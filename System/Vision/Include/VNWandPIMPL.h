#ifndef __VISION_INCLUDE_VNWANDPIMPL_H__
#define __VISION_INCLUDE_VNWANDPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  class VNWandPIMPL {
  public:
    VNWandPIMPL(void);
    virtual ~VNWandPIMPL(void);
    
    void NotFoundOnScreen(void);

    bool visible_;
    VNPoint location_;
    cv::Scalar hsvMin_;
    cv::Scalar hsvMax_;
  };
}
}

#endif // __VISION_INCLUDE_VNWANDPIMPL_H__
