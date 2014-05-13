#ifndef __VN_INTEGRAL_IMAGE__
#define __VN_INTEGRAL_IMAGE__

#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {
	
	void IntegralImage( const cv::Mat& src, cv::Mat& dst );
}
}
#endif // __VN_INTEGRAL_IMAGE__