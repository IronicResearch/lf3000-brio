#ifndef __VISION_INCLUDE_VNVIRTUALTOUCHPIMPL_H__
#define __VISION_INCLUDE_VNVIRTUALTOUCHPIMPL_H__

#include <CoreTypes.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  class VNVirtualTouchPIMPL {
  public:
    VNVirtualTouchPIMPL(float learningRate);
    virtual ~VNVirtualTouchPIMPL(void);
    
    void Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
		    LeapFrog::Brio::U16 frameProcessingHeight);
    void Execute(cv::Mat &input, cv::Mat &output);


    float learningRate_;
    int threshold_;
    cv::Mat gray_;
    cv::Mat background_;
    cv::Mat backImage_;
    cv::Mat foreground_;
  private:
	void AbsDifferenceThreshold(cv::Mat& background, cv::Mat &gray, cv::Mat& output, int threshold, float alpha);
	void ConvertToRGB(const cv::Mat& in, cv::Mat& outrgb);
	void ConvertToGray(const cv::Mat& in, cv::Mat& outrgb);
  };

}
}

#endif // __VISION_INCLUDE_VNVIRTUALTOUCHPIMPL_H__
