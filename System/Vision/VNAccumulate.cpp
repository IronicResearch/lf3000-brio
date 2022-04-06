#include <VNAccumulate.h>
#include <assert.h>
#include <VNFixedPoint.h>

namespace LF {
namespace Vision {
	void accumulateWeightedFixedPoint( const cv::Mat& src, cv::Mat& dst32, float alpha ) {
		assert( src.size().area() == dst32.size().area() );

		fixed_t a = FLOAT2FIXED( alpha );
		fixed_t b = FLOAT2FIXED( 1.0f - alpha );

		fixed_t d, s, result;
		for( int i = 0; i < src.rows * src.cols; i++ ) {
			s = INT2FIXED( src.at<uint8_t>(i) );
			d = dst32.at<uint32_t>(i);
			result = MULT(a, s) + MULT(b, d);
			dst32.at<uint32_t>(i) = result;
		}
	}


	void convertFixedPointToU8( const cv::Mat& srcfp, cv::Mat& dstu8) {
		if( dstu8.empty() ) {
			dstu8.create( srcfp.size(), CV_8U );
		}

		for( int i = 0; i < srcfp.rows * srcfp.cols; i++ ) {
			uint32_t r = FIXED2INT(srcfp.at<uint32_t>(i));
			dstu8.at<uint8_t>(i) = r;
		}
	}
}
}
