#include <VNAccumulate.h>
#include <assert.h>

// 16.16 fixed point
typedef uint32_t fixed_t;

#define FRACT_BITS 		24
#define FRACT_BITS_D2 	(FRACT_BITS/2)
#define FIXED_ONE 		(1 << FRACT_BITS)
#define INT2FIXED(x) 	((x) << FRACT_BITS)
#define FLOAT2FIXED(x) 	((int32_t)((x) * (1 << FRACT_BITS))) 
#define FIXED2INT(x) 	((x) >> FRACT_BITS)
#define FIXED2DOUBLE(x) (((double)(x)) / (1 << FRACT_BITS))
#define MULT(x, y) 		( ((x) >> FRACT_BITS_D2) * ((y)>> FRACT_BITS_D2) )


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
