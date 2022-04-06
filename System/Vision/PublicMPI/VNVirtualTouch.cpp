#include <Vision/VNVirtualTouch.h>
#include <VNVirtualTouchPIMPL.h>
#include <opencv2/opencv.hpp>
#include <DebugMPI.h>
#include <Utility.h>

namespace LF {
namespace Vision {

  const float kVNDefaultVirtualTouchLearningRate = 0.2f;
  const int kVNDefaultVirtualTouchThreshold = 10;


  VNVirtualTouch::VNVirtualTouch(float learningRate) :
    pimpl_(static_cast<VNVirtualTouchPIMPL*>(NULL)) {

	  if(HasPlatformCapability(kCapsVision)) {
		  pimpl_.reset(new VNVirtualTouchPIMPL(learningRate));
	  } else {
		LeapFrog::Brio::CDebugMPI localDbgMPI(kGroupVision);
		localDbgMPI.DebugOut(kDbgLvlImportant, "VNVirtualTouch::VNVirtualTouch(float) called on a platform which does not support vision\n");
	  }
  }

  VNVirtualTouch::VNVirtualTouch(VNInputParameters *params) :
    pimpl_(static_cast<VNVirtualTouchPIMPL*>(NULL)) {

	  if(HasPlatformCapability(kCapsVision)) {
		  pimpl_.reset(new VNVirtualTouchPIMPL(params));
	  } else {
		LeapFrog::Brio::CDebugMPI localDbgMPI(kGroupVision);
		localDbgMPI.DebugOut(kDbgLvlImportant, "VNVirtualTouch::VNVirtualTouch(VNInputParameters) called on a platform which does not support vision\n");
	  }
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
