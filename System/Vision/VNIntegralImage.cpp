#include "VNIntegralImage.h"
#include "VNAlgorithmHelpers.h"

namespace LF {
namespace Vision {

	void IntegralImage( const cv::Mat& srcMat, cv::Mat& dstMat ) {
	  if (!CheckInputs(srcMat, dstMat, CV_32S)) {
	    return;
	  }

		//	todo: see http://www.aishack.in/2010/07/integral-images/


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

		for ( int y = 1; y < height; y++, src+=srcStep, dst+=dstStep) {
			rs = 0;
			for ( int x = 0; x < width; x++ ) {
				rs += src[x];
				dst[x] = dst[x - dstStep] + rs;

			}
		}
	}

	int IntegralSum(const cv::Mat &integral, cv::Rect &roi) {
		int tl = integral.at<unsigned int>(roi.y, roi.x);
		int tr = integral.at<unsigned int>(roi.y, roi.x+roi.width);
		int bl = integral.at<unsigned int>(roi.y+roi.height, roi.x);
		int br = integral.at<unsigned int>(roi.y+roi.height, roi.x+roi.width);
		return br-bl-tr+tl;
	}

}
}
