#include <Vision/VNVirtualTouch.h>
#include <VNVirtualTouchPIMPL.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  static const float kVNMinLearningRate = 0.001f;
  static const float kVNMaxLearningRate = 1.f;
  const float kVNDefaultVirtualTouchLearningRate = 0.2f;
  const int kVNDefaultVirtualTouchThreshold = 10;


  VNVirtualTouch::VNVirtualTouch(float learningRate, int intensityThreshold) :
    pimpl_(new VNVirtualTouchPIMPL(learningRate, intensityThreshold)) {

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
  VNVirtualTouch::Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
			     LeapFrog::Brio::U16 frameProcessingHeight) {
    pimpl_->Initialize(frameProcessingWidth,
		       frameProcessingHeight);
  }

  void
  VNVirtualTouch::Execute(cv::Mat &input, cv::Mat &output) {
    pimpl_->Execute(input, output);
  }

} // namespace Vision
} // namespace LF
