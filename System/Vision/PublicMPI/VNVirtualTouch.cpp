#include <Vision/VNVirtualTouch.h>
#include <VNVirtualTouchPIMPL.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  const float kVNDefaultVirtualTouchLearningRate = 0.2f;
  const int kVNDefaultVirtualTouchThreshold = 10;


  VNVirtualTouch::VNVirtualTouch(float learningRate) :
    pimpl_(new VNVirtualTouchPIMPL(learningRate)) {

  }

  VNVirtualTouch::VNVirtualTouch(VNInputParameters *params) :
    pimpl_(new VNVirtualTouchPIMPL(params)) {

  }

  VNVirtualTouch::~VNVirtualTouch(void) {

  }

  float
  VNVirtualTouch::SetLearningRate(float rate) {
    if (pimpl_) {
      return pimpl_->SetLearningRate(rate);
    }
    return kVNDefaultVirtualTouchLearningRate;
  }

  float
  VNVirtualTouch::GetLearningRate(void) const {
    if (pimpl_) {
      return pimpl_->GetLearningRate();
    }
    return kVNDefaultVirtualTouchLearningRate;
  }

  void
  VNVirtualTouch::Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
			     LeapFrog::Brio::U16 frameProcessingHeight) {
    if (pimpl_) {
      pimpl_->Initialize(frameProcessingWidth,
			 frameProcessingHeight);
    }
  }

  void
  VNVirtualTouch::Execute(cv::Mat &input, cv::Mat &output) {
    if (pimpl_) {
      pimpl_->Execute(input, output);
    }
  }

} // namespace Vision
} // namespace LF
