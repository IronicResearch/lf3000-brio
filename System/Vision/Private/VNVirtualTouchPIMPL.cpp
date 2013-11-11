#include <VNVirtualTouchPIMPL.h>
#include <opencv2/imgproc/types_c.h>

namespace LF {
namespace Vision {

	static const int kVNDefaultVirtualTouchThreshold = 10;

	VNVirtualTouchPIMPL::VNVirtualTouchPIMPL(float learningRate) :
		learningRate_(learningRate),
		threshold_(kVNDefaultVirtualTouchThreshold) {

	}

	VNVirtualTouchPIMPL::~VNVirtualTouchPIMPL(void) {

	}

	void
	VNVirtualTouchPIMPL::Execute(cv::Mat *input, cv::Mat *output) {
		cv::cvtColor(*input, gray_, CV_BGR2GRAY);

		// initialize background to first frame
		if (background_.empty())
			gray_.convertTo(background_, CV_32F);

		// convert background to 8U
		background_.convertTo(backImage_, CV_8U);

		// compute difference between current image and background
		cv::absdiff(backImage_, gray_, foreground_);

		// apply threshold to foreground image
		cv::threshold(foreground_, *output, threshold_, 255, cv::THRESH_BINARY_INV);

		// accumulate the background
		cv::accumulateWeighted(gray_, background_, learningRate_);

	}

}
}
