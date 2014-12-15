#ifndef hsv2rgb_hsv2rgb_h
#define hsv2rgb_hsv2rgb_h

#include <stdint.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

	void RGBToHSV( const cv::Mat& input, cv::Mat& output );

}
}
#endif
