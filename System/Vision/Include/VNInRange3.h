#ifndef _VNInRange3_h_
#define _VNInRange3_h_

#include <opencv2/opencv.hpp>

namespace LF {
	namespace Vision {
		void inRange3( const cv::Mat& src, const cv::Scalar& min, const cv::Scalar& max, cv::Mat& dst );
	}
}


#endif
