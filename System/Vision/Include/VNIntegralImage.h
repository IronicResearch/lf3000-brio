#ifndef __VN_INTEGRAL_IMAGE__
#define __VN_INTEGRAL_IMAGE__

#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

	void IntegralImage( const cv::Mat& src, cv::Mat& dst );
	int IntegralSum( const cv::Mat &integral, cv::Rect &roi );
}
}
#endif // __VN_INTEGRAL_IMAGE__
