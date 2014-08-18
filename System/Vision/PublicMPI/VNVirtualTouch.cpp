#include <Vision/VNVirtualTouch.h>
#include <VNVirtualTouchPIMPL.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  static const float kVNMinLearningRate = 0.001f;
  static const float kVNMaxLearningRate = 1.f;
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
      if (rate >= kVNMinLearningRate && rate <= kVNMaxLearningRate)
	pimpl_->learningRate_ = rate;
      return pimpl_->learningRate_;
    }
    return kVNDefaultVirtualTouchLearningRate;
  }

  float
  VNVirtualTouch::GetLearningRate(void) const {
    if (pimpl_) {
      return pimpl_->learningRate_;
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
