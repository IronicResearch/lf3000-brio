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
    
    void NotVisibleOnScreen(void);
    void VisibleOnScreen(const cv::Point &p);

    bool IsVisible(void) const;
    VNPoint GetLocation(void) const;

    cv::Scalar hsvMin_;
    cv::Scalar hsvMax_;

  protected:
    bool visible_;
    VNPoint location_;   
  };
}
}

#endif // __VISION_INCLUDE_VNWANDPIMPL_H__
