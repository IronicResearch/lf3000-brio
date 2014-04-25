#ifndef __VNYUYV2RGB_H__
#define __VNYUYV2RGB_H__


#include <opencv2/opencv.hpp>

namespace LF {
	namespace Vision {
		void YUYV2RGB( const uint8_t* src, const int width, const int height, cv::Mat& dst );
	}
}

#endif // __VNYUYV2RGB_H__
