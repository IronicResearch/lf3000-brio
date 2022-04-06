#ifndef __VNACCUM_H__
#define __VNACCUM_H__

#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {
	void accumulateWeightedFixedPoint( const cv::Mat& src, cv::Mat& dst32, float alpha );
	void convertFixedPointToU8( const cv::Mat& srcfp, cv::Mat& dstu8);
}
}
#endif // __VNACCUM_H__
