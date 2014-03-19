#include "VNRGB2Gray.h"
#if !defined(EMULATION)
#include <arm_neon.h>
#endif

#if defined(EMULATION)  
void fast_rgb_to_gray(const cv::Mat& input, cv::Mat& output) {
	cv::cvtColor(input, output, CV_RGB2GRAY);
}
#else
void fast_rgb_to_gray(const cv::Mat& input, cv::Mat& output) {

	if( output.empty() ) {// initialize gray matrix if not already
		output.create(input.size(), CV_8U);
	}

	uint8_t __restrict * dest = output.data;
	uint8_t __restrict * src  = input.data;
	int numPixels             = input.total();  
	asm volatile(
			"lsr %2, %2, #3 \n"
			"# build the three constants:  \n"
			"mov r4, #28                   \n" // Blue channel multiplier
			"mov r5, #151                  \n" // Green channel multiplier
			"mov r6, #77                   \n" // Red channel multiplier
			"vdup.8 d4, r4                 \n"
			"vdup.8 d5, r5                 \n"
			"vdup.8 d6, r6                 \n"

			"0: \n"
			"# load 8 pixels: \n"  //RGBR
			"vld3.8 {d0-d2}, [%1]! \n"
			"# do the weight average: \n"
			"vmull.u8 q7, d0, d4 \n"
			"vmlal.u8 q7, d1, d5 \n"
			"vmlal.u8 q7, d2, d6 \n"
			"# shift and store: \n"
			"vshrn.u16 d7, q7, #8 \n" // Divide q3 by 256 and store in the d7
			"vst1.8 {d7}, [%0]! \n"
			"subs %2, %2, #1 \n" // Decrement iteration count
			"bne 0b \n" // Repeat unil iteration count is not zero
			:
			: "r"(dest), "r"(src), "r"(numPixels)
			: "r4", "r5", "r6"
				);
}
#endif //!EMULATION

