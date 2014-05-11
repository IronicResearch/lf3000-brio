#include "VNYUYV2RGB.h"
#include <stdio.h>
#if !defined(EMULATION)
#define VN_NEON_YUYV2GRAY 1
#else
#define VN_NEON_YUYV2GRAY 0
#endif
#if VN_NEON_YUYV2GRAY
#include <arm_neon.h>
#endif

namespace LF {
namespace Vision {
#define SAT(c) if (c & (~255)) { if (c < 0) c = 0; else c = 255; }
	

	void YUYV2RGB( const cv::Mat& src, cv::Mat& dst ) {
		YUYV2RGB( src.data, src.rows, src.cols, dst );
	}
	
	void YUYV2RGB( const uint8_t* src, const int width, const int height, cv::Mat& dst ) {
	
		if( dst.empty() ) { 	// initialize rgb image if not already initialized
			dst.create(width, height, CV_8UC3);
		}

		const uint8_t *s = src;
		uint8_t *d = dst.data;
		int l, c;
		int r, g, b, cr, cg, cb, y1, y2;

		l = height;
		while (l--) {
			c = width >> 1;
			while (c--) {
				y1 = *s++;
				cb = ((*s - 128) * 454) >> 8;
				cg = (*s++ - 128) * 88;
				y2 = *s++;
				cr = ((*s - 128) * 359) >> 8;
				cg = (cg + (*s++ - 128) * 183) >> 8;

				r = y1 + cr;
				b = y1 + cb;
				g = y1 - cg;
				SAT(r);
				SAT(g);
				SAT(b);

				*d++ = r;
				*d++ = g;
				*d++ = b;

				r = y2 + cr;
				b = y2 + cb;
				g = y2 - cg;
				SAT(r);
				SAT(g);
				SAT(b);

				*d++ = r;
				*d++ = g;
				*d++ = b; 
			}
		}
	}
	
	void YUYV2Gray( const cv::Mat& src, cv::Mat& dst ) {
		YUYV2Gray( src.data, src.rows, src.cols, dst );
	}
	
	void YUYV2Gray( const uint8_t* src, const int width, const int height, cv::Mat& dst ) {
		if( dst.empty() ) { 	// initialize rgb image if not already initialized
			dst.create(width, height, CV_8UC1);
		}

#if VN_NEON_YUYV2GRAY
		uint8_t __restrict * dest = dst.data;
		uint8_t __restrict * source  = (uint8_t*)src;
		int numPixels             = (int)dst.total();
		
		__asm__ volatile(
				"0:"
					"vld2.8 {d0,d1}, [%1]! \n"

					"vld2.8 {d2,d3}, [%1]! \n"
					"vld2.8 {d4,d5}, [%1]! \n"
					"vld2.8 {d6,d7}, [%1]! \n"

					"vst1.8 {d0}, [%0]! \n"
					"vst1.8 {d2}, [%0]! \n"
					"vst1.8 {d4}, [%0]! \n"
					"vst1.8 {d6}, [%0]! \n"
				"subs %2, %2, #32 \n"
				"bne 0b \n"
		 :
		 :	"r"(dest),	// %0
			"r"(source),	// %1
			"r"(numPixels)	// %2
		 :  "r4", "r5", "r6"
					 
		);
#else

		const uint8_t *s = src;
		uint8_t *d = dst.data;
		int l, c;

		l = height;
		while (l--) {
			c = width >> 1;
			while (c--) {
				*d = *s++;
				d++;
				s++;
				*d = *s++;
				d++;
				s++;
			}
		}
#endif // VN_NEON_YUYV2GRAY
	}
}
}
