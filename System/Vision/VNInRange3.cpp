#include <stdio.h>
#include "VNInRange3.h"
#include "VNAlgorithmHelpers.h"
#if !defined(EMULATION) && defined(LF3000)
#include <arm_neon.h>
#endif

namespace LF {
	namespace Vision {
		void inRange3( const cv::Mat& src, const cv::Scalar& min, const cv::Scalar& max, cv::Mat& dst ) {
		  if (!CheckInputs(src, dst, CV_8U)) {
		    return;
		  }

#if defined(EMULATION) || !defined(LF3000)
			cv::inRange(src, min, max, dst);
	
#else
			
			uint8_t * __restrict dest = dst.data;
			uint8_t * __restrict source = src.data;
			uint8_t mn[3] = { uint8_t(min[0]), uint8_t(min[1]), uint8_t(min[2]) };
			uint8_t mx[3] = { uint8_t(max[0]), uint8_t(max[1]), uint8_t(max[2]) };
			int cnt = src.total();
			
			__asm__ volatile(
				"lsr %2, %2, #3 \n"				// load cnt and right shift 3 == sizeof(rgb)
				// load min constant d2, d3, d4
				"vld1.8 {d0}, [%3]! \n"
				"vdup.8 d2, d0[0] \n"
				"vdup.8 d3, d0[1] \n"
				"vdup.8 d4, d0[2] \n"
				// load max constants d5, d6, d7
				"vld1.8 {d0}, [%4]! \n"
				"vdup.8 d5, d0[0] \n"
				"vdup.8 d6, d0[1] \n"
				"vdup.8 d7, d0[2] \n"
				"0: \n"							// loop label
					// load the sourc data
					"vld3.8 {d8, d9, d10}, [%1]! \n"	// <q4(d8,d9) + q5(d10)
							 
					// go through min
					"vcge.u8 d11, d8, d2 \n"		// d11 = src.x >= min.x
					"vcge.u8 d12, d9, d3 \n"		// d12 = src.y >= min.y
					"vcge.u8 d13, d10, d4 \n"		// d13 = src.z >= min.z

					// go through max
					"vcge.u8 d14, d5, d8 \n"		// d11 = src.x <= max.x
					"vcge.u8 d15, d6, d9 \n"		// d12 = src.y <= max.y
					"vcge.u8 d16, d7, d10 \n"		// d13 = src.z <= max.z
							 
					// AND (&) them all together and put into d0 = d11 & d12 & d13 & d14 & d15 & d16
					"vand.8 d0, d11, d12 \n"
					"vand.8 d0, d0, d13 \n"
					"vand.8 d0, d0, d14 \n"
					"vand.8 d0, d0, d15 \n"
					"vand.8 d0, d0, d16 \n"
							 
					// save it off to dest
					"vst1.8 {d0}, [%0]!\n"
					
				"subs %2, %2, #1 \n"			// decrement cnt--
				"bne 0b \n"						// while cnt != 0
					 :
					 : "r"(dest), "r"(source), "r"(cnt), "r"(mn), "r"(mx)		// %0, %1, %2, %3
					 : "r4", "r5", "r6"
							 
							 

			);
			
			
#endif
			
		}
		

		
	}
}
