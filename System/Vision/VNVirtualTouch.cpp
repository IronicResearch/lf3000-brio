#include <Vision/VNVirtualTouch.h>
#include <VNVirtualTouchPIMPL.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

	const float kVNDefaultVirtualTouchLearningRate = 0.2f;

	VNVirtualTouch::VNVirtualTouch(float learningRate) :
		pimpl_(new VNVirtualTouchPIMPL(learningRate)) {

	}

	VNVirtualTouch::~VNVirtualTouch(void) {

	}

	void
	VNVirtualTouch::SetLearningRate(float rate) {
		pimpl_->learningRate_ = rate;
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
