#include <Vision/VNVirtualTouch.h>
#include <VNVirtualTouchPIMPL.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  static const float kVNMinLearningRate = 0.001f;
  static const float kVNMaxLearningRate = 1.f;
  const float kVNDefaultVirtualTouchLearningRate = 0.2f;
  
  VNVirtualTouch::VNVirtualTouch(float learningRate) :
    pimpl_(new VNVirtualTouchPIMPL(learningRate)) {
    
  }
  
  VNVirtualTouch::~VNVirtualTouch(void) {
    
  }
  
  float
  VNVirtualTouch::SetLearningRate(float rate) {
    if (rate >= kVNMinLearningRate && rate <= kVNMaxLearningRate)
      pimpl_->learningRate_ = rate;
    return pimpl_->learningRate_;
  }
  
  float
  VNVirtualTouch::GetLearningRate(void) const {
    return pimpl_->learningRate_;
  }
  
  void
  VNVirtualTouch::Execute(void *input, void *output) {
    pimpl_->Execute(static_cast<cv::Mat*>(input), static_cast<cv::Mat*>(output));
  }

} // namespace Vision
} // namespace LF
