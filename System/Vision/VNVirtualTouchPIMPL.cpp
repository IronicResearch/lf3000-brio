#include <VNVirtualTouchPIMPL.h>
#include "VNRGB2Gray.h"

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
    
    // scale down to half size
    static cv::Mat resizedInput;
    cv::resize(input, resizedInput, cv::Size(), kVNDownScale, kVNDownScale, cv::INTER_LINEAR);
    
    fast_rgb_to_gray( resizedInput, gray_ );
    // initialize background to first frame
    if (background_.empty())
      gray_.convertTo(background_, CV_32F);
    
    // convert background to 8U
    background_.convertTo(backImage_, CV_8U);
    
    // compute difference between current image and background
    cv::absdiff(backImage_, gray_, foreground_);
    
    // apply threshold to foreground image
    cv::threshold(foreground_, output, threshold_, 255, cv::THRESH_BINARY);
    
    // accumulate the background
    cv::accumulateWeighted(gray_, background_, learningRate_);

    // scale output back to fullsize
    cv::resize(output, output, cv::Size(), kVNUpScale, kVNUpScale, cv::INTER_LINEAR);    
  }
}
}
