#ifndef __VISION_INCLUDE_VNVIRTUALTOUCHPIMPL_H__
#define __VISION_INCLUDE_VNVIRTUALTOUCHPIMPL_H__

#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  class VNVirtualTouchPIMPL {
  public:
    VNVirtualTouchPIMPL(float learningRate);
    virtual ~VNVirtualTouchPIMPL(void);
    
    void Execute(cv::Mat *input, cv::Mat *output);
    
    float learningRate_;
    int threshold_;
    cv::Mat gray_;
    cv::Mat background_;
    cv::Mat backImage_;
    cv::Mat foreground_;
  };

}
}

#endif // __VISION_INCLUDE_VNVIRTUALTOUCHPIMPL_H__
