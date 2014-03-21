#include <VNVirtualTouchPIMPL.h>
#include "VNRGB2Gray.h"
#undef LF_PROFILE 
#include <VNProfiler.h>

namespace LF {
namespace Vision {

  static const int kVNDefaultVirtualTouchThreshold = 10;
  static const float kVNDownScale = 0.5f;
  static const float kVNUpScale = 1.0f/kVNDownScale;

  VNVirtualTouchPIMPL::VNVirtualTouchPIMPL(float learningRate) :
    learningRate_(learningRate),
    threshold_(kVNDefaultVirtualTouchThreshold) {
    
  }
  
  VNVirtualTouchPIMPL::~VNVirtualTouchPIMPL(void) {
    
  }
  
  void
  VNVirtualTouchPIMPL::Execute(cv::Mat &input, cv::Mat &output) {

	  PROF_BLOCK_START("Execute");

	  PROF_BLOCK_START("down scale");	  
	  // scale down to half size
	  cv::Mat resizedInput;
	  cv::resize(input, resizedInput, cv::Size(), kVNDownScale, kVNDownScale, cv::INTER_LINEAR);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("RGB to GRAY");
	  fast_rgb_to_gray( resizedInput, gray_ );
	  PROF_BLOCK_END();
	  // initialize background to first frame
	  if (background_.empty())
		  gray_.convertTo(background_, CV_32F);

	  PROF_BLOCK_START("convertTo 8U");
	  // convert background to 8U
	  background_.convertTo(backImage_, CV_8U);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("absdiff");
	  // compute difference between current image and background
	  cv::absdiff(backImage_, gray_, foreground_);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("threshold");
	  // apply threshold to foreground image
	  cv::threshold(foreground_, output, threshold_, 255, cv::THRESH_BINARY);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("up scale");
	  // scale output back to fullsize
	  cv::resize(output, output, cv::Size(), kVNUpScale, kVNUpScale, cv::INTER_LINEAR);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("accumulateWeighted");
	  // accumulate the background
	  cv::accumulateWeighted(gray_, background_, learningRate_);
	  PROF_BLOCK_END();

	  PROF_BLOCK_END();
  }

}
}
