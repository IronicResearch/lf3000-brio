#include "VNIntegralImage.h"


namespace LF {
namespace Vision {

	void IntegralImage( const cv::Mat& srcMat, cv::Mat& dstMat ) {
		if ( dstMat.empty() ) {
			dstMat.create( srcMat.size(), CV_32S );
		}
		
		
//	todo: see http://www.aishack.in/2010/07/integral-images/
		
//		integral image is width + 1, height + 1
		
		// set up variables for data access
		int width = srcMat.size().width;
		int height = srcMat.size().height;
		int srcStep = width;//srcMat.step1()/sizeof(uint8_t);
		int dstStep = width;//dstMat.step1()/sizeof(int32_t);
		uint8_t* src = (uint8_t*)srcMat.data;
		int32_t* dst = (int32_t*)dstMat.data;
		
		int rs = 0;
		// first row
		for ( int x = 0; x < width; x++ ) {
			rs += src[x];
			dst[x] = dst[x] + rs;
		}

		dst += dstStep;
		src += srcStep;
		
		for ( int y = 0; y < height; y++, src+=srcStep, dst+=dstStep) {
			rs = 0;
			for ( int x = 0; x < width; x++ ) {
				rs += src[x];
				dst[x] = dst[x - dstStep] + rs;

			}
		}
/*
		// first row only
		for(int j=0; j<width; j+=4) {
			rs += src.at<uint8_t>(j);
			dst.at<int32_t>(j) = rs;
			
			rs += src.at<uint8_t>(j+1);
			dst.at<int32_t>(j+1) = rs;
			
			rs += src.at<uint8_t>(j+2);
			dst.at<int32_t>(j+2) = rs;
			
			rs += src.at<uint8_t>(j+3);
			dst.at<int32_t>(j+3) = rs;
		}

		
		// remaining cells are sum above and to the left
		int i_minone;
		for(int i=1; i<height; ++i) {
			rs = 0;
			i_minone = i-1;
			for(int j=0; j<width; j+=4) {
				rs += src.at<uint8_t>(i,j);
				dst.at<int32_t>(i,j) = rs + dst.at<int32_t>(i_minone,j);
				
				rs += src.at<uint8_t>(i,j+1);
				dst.at<int32_t>(i,j+1) = rs + dst.at<int32_t>(i_minone,j+1);
				
				rs += src.at<uint8_t>(i,j+2);
				dst.at<int32_t>(i,j+2) = rs + dst.at<int32_t>(i_minone,j+2);
				
				rs += src.at<uint8_t>(i,j+3);
				dst.at<int32_t>(i,j+3) = rs + dst.at<int32_t>(i_minone,j+3);
			}
		}
 */
	}
}
}