#include "VNYUYV2RGB.h"
#include <stdio.h>
#include "VNAlgorithmHelpers.h"
#if !defined(EMULATION)
#define VN_NEON_YUYV2GRAY 1
#define VN_NEON_YUYV2RGB 0
#else
#define VN_NEON_YUYV2GRAY 0
#define VN_NEON_YUYV2RGB 0
#endif
#if VN_NEON_YUYV2GRAY
#include <arm_neon.h>
#endif

namespace LF {
namespace Vision {
#define SAT(c) if (c & (~255)) { if (c < 0) c = 0; else c = 255; }

  void YUYV2RGB( const uint8_t* src, const int width, const int height, cv::Mat& dst );
  void YUYV2Gray( const uint8_t* src, const int width, const int height, cv::Mat& dst );
  void YUYV2YUV( const uint8_t* src, const int width, const int height, cv::Mat& dst );

	void YUYV2RGB( const cv::Mat& src, cv::Mat& dst ) {
	  if (CheckInputs(src, dst, CV_8UC3)) {
	    YUYV2RGB( src.data, src.size().width, src.size().height, dst );
	  }
	}

	// see: http://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx#YUV422formats16bitsperpixel
	// see: http://en.wikipedia.org/wiki/YUY2#Y.27UV422_to_RGB888_conversion
	void YUYV2RGB( const uint8_t* src, const int width, const int height, cv::Mat& dst ) {

#if VN_NEON_YUYV2RGB

        uint8_t __restrict * dest   = dst.data;
        uint8_t __restrict * source = (uint8_t*)src;
        int numPixels               = (int)dst.total();


		__asm__ volatile(
			 "0:"

						 ".macro mul_col_s16 res_d, col_d\n"
						 "vmull.s16   q12, d16, \\col_d[0]   @ multiply col element 0 by matrix col 0 \n"
//						 vmlal.s16   q12, d17, \col_d[1]   @ multiply-acc col element 1 by matrix col 1
//						 vmlal.s16   q12, d18, \col_d[2]   @ multiply-acc col element 2 by matrix col 2
//						 vmlal.s16   q12, d19, \col_d[3]   @ multiply-acc col element 3 by matrix col 3
//						 vqrshrn.s32 \res_d, q12, #14            @ shift right and narrow accumulator into
//						 @  Q1.14 fixed point format, with saturation
						 ".endm \n"

// TODO: vld4.8 to de interleave to y0 u y1 v
				// de-interleave YUYV
				 //"vld2.8 {d2,d3}, [%1]! \n"
				 // y0 == d2, u = d3, y1 == d4, v == d5
				 "vld4.8 {d2,d3,d4,d5}, [%1]! \n"

				//	int c0 = y0 - 16;
				//	int c1 = y1 - 16;
				"vmov.i8 d0, #16 \n"
				"vsubl.u8 q3, d2, d0 \n"  // q3 == c0
				"vsubl.u8 q4, d4, d0 \n"  // q4 == c1

				//	int d = u0 - 128;
				//	int e = v0 - 128;
				"vmov.i8 d0, #128 \n"
				"vsubl.u8 q5, d3, d0 \n"  // q5 == d
				"vsubl.u8 q6, d5, d0 \n"  // q6 == e

				//	 R = clip(( 298 * C           + 409 * E + 128) >> 8)
				//	 G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
				//	 B = clip(( 298 * C + 516 * D           + 128) >> 8)

				// 298 * c0 == q3
				// 298 * c1 == q4
				"vmov.i16 q0, #298 \n"
				"vmul.i16 q3, q3, q0 \n"
				"vmul.i16 q4, q4, q0 \n"

				// 516 * d == q7
				"vmov.i16 q0, #516 \n"
				"vmul.i16 q7, q5, q0 \n"

				// 100 * d  == q8
				"vmov.i16 q0, #100 \n"
				"vmul.i16 q8, q5, q0 \n"

				// 409 * e == q9
				"vmov.i16 q0, #409 \n"
				"vmul.i16 q9, q6, q0 \n"

				// 208 * e == q10
				"vmov.i16 q0, #208 \n"
		        "vmul.i16 q10, q6, q0 \n"

				 /**
				  *   first do other c0
				  */

				// 298 * c0 + 409 * e == q11
				"vadd.i16 q11, q3, q9 \n"

				// 298 * c0 - 100 * d  == q12
				"vsub.i16 q12, q3, q8 \n"
				// + -208 * e             == q12
				"vsub.i16 q12, q12, q10 \n"

				// 298 * c0 + 516 * d  == q13
				"vadd.i16 q13, q3, q7 \n"

				// +128
				"vmov.i16 q0, #128 \n"
				"vadd.i16 q11, q11, q0 \n"
				"vadd.i16 q12, q12, q0 \n"
				"vadd.i16 q13, q13, q0 \n"

				// >> 8 and narrow to 8 bit (saturating)
//				"vqshrun.s16 d0, q11, #8 \n"
//				"vqshrun.s16 d1, q12, #8 \n"
//				"vqshrun.s16 d2, q13, #8 \n"
						 "vqshrn.u16 d0, q11, #8 \n"
						 "vqshrn.u16 d1, q12, #8 \n"
						 "vqshrn.u16 d2, q13, #8 \n"

				"vst3.8 {d0, d1, d2}, [%0]!\n"

				 /**
				  *   then do other c1
				  */
				 // 298 * c0 + 409 * e == q11
				 "vadd.i16 q11, q4, q9 \n"

				 // 298 * c0 - 100 * d  == q12
				 "vsub.i16 q12, q4, q8 \n"
				 // + -208 * e             == q12
				 "vsub.i16 q12, q12, q10 \n"

				 // 298 * c0 + 516 * d  == q13
				 "vadd.i16 q13, q4, q7 \n"

				 // +128
				 "vmov.i16 q0, #128 \n"
				 "vadd.i16 q11, q11, q0 \n"
				 "vadd.i16 q12, q12, q0 \n"
				 "vadd.i16 q13, q13, q0 \n"

				 // >> 8 and narrow to 8 bit (saturating)
//				"vqshrun.s16 d0, q11, #8 \n"
//				"vqshrun.s16 d1, q12, #8 \n"
//				"vqshrun.s16 d2, q13, #8 \n"
				 "vqshrn.u16 d0, q11, #8 \n"
				 "vqshrn.u16 d1, q12, #8 \n"
				 "vqshrn.u16 d2, q13, #8 \n"

				 "vst3.8 {d0, d1, d2}, [%0]!\n"


			 "subs %2, %2, #16 \n"
			 "bne 0b \n"
			 :
			 :	"r"(dest),	// %0
			 "r"(source),	// %1
			 "r"(numPixels) // %2
			 :  "r4", "r5", "r6"

			 );


#else

#if 1
        uint8_t __restrict * dest   = dst.data;
        uint8_t __restrict * source = (uint8_t*)src;
        int numPixels               = width * height;

		//	 R = clip(( 298 * C           + 409 * E + 128) >> 8)
		//	 G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
		//	 B = clip(( 298 * C + 516 * D           + 128) >> 8)

		int16_t y0, u, y1, v, c0, c1, d, e, r, g, b;
		for ( int i = 0; i < numPixels/2; i++, source+=4, dest+=6 ) {
			y0 = source[0];
			u = source[1];
			y1 = source[2];
			v = source[3];

			//			Cr = Cr - 128;
			//			Cb = Cb - 128;
			//			R = Y + Cr + (Cr>>2) + (Cr>>3) + (Cr>>5)
			//			G = Y - ((Cb>>2) + (Cb>>4) + (Cb>>5)) - ((Cr>>1) + (Cr>>3) + (Cr>>4) + (Cr>>5))
			//			B = Y + Cb + (Cb>>1) + (Cb>>2) + (Cb>>6)

			int16_t Cr = u - 128;
			int16_t Cb = v - 128;
			int16_t Y = y0;

			b = Y + Cr + (Cr>>2) + (Cr>>3) + (Cr>>5);
			g = Y - ((Cb>>2) + (Cb>>4) + (Cb>>5)) - ((Cr>>1) + (Cr>>3) + (Cr>>4) + (Cr>>5));
			r = Y + Cb + (Cb>>1) + (Cb>>2) + (Cb>>6);

			SAT(r);
			SAT(g);
			SAT(b);

			dest[0] = r;
			dest[1] = g;
			dest[2] = b;


			Y = y1;

			b = Y + Cr + (Cr>>2) + (Cr>>3) + (Cr>>5);
			g = Y - ((Cb>>2) + (Cb>>4) + (Cb>>5)) - ((Cr>>1) + (Cr>>3) + (Cr>>4) + (Cr>>5));
			r = Y + Cb + (Cb>>1) + (Cb>>2) + (Cb>>6);

			SAT(r);
			SAT(g);
			SAT(b);

			dest[3] = r;
			dest[4] = g;
			dest[5] = b;

/*
			c0 = y0 - 16;
			c1 = y1 - 16;
			d = u - 128;
			e = v - 128;

			r = (298 * c0 + 409 * e				+ 128) >> 8;
			g = (298 * c0 - 100 * d - 208 * e	+ 128) >> 8;
			b = (298 * c0 + 516 * d				+ 128) >> 8;


			SAT(r);
			SAT(g);
			SAT(b);

			dest[0] = r;
			dest[1] = g;
			dest[2] = b;


			r = (298 * c1 + 409 * e				+ 128) >> 8;
			g = (298 * c1 - 100 * d - 208 * e	+ 128) >> 8;
			b = (298 * c1 + 516 * d				+ 128) >> 8;

			SAT(r);
			SAT(g);
			SAT(b);

			dest[3] = r;
			dest[4] = g;
			dest[5] = b;
 */

		}
#else
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
#endif

#endif
	}

	void YUYV2Gray( const cv::Mat& src, cv::Mat& dst ) {
	  if (CheckInputs(src, dst, CV_8UC1)) {
	    YUYV2Gray( src.data, src.size().width, src.size().height, dst );
	  }
	}

	void YUYV2Gray( const uint8_t* src, const int width, const int height, cv::Mat& dst ) {

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


	// converts to YUV ( actually UVY where intensity is last entry )
	void YUYV2YUV( const cv::Mat& src, cv::Mat& dst ) {
	  if (CheckInputs(src, dst, CV_8UC3)) {
	    YUYV2YUV( src.data, src.size().width, src.size().height, dst );
	  }
	}

	void YUYV2YUV( const uint8_t* src, const int width, const int height, cv::Mat& dst ) {

		uint8_t __restrict * dest   = (uint8_t*)dst.data;
		int8_t __restrict * source = (int8_t*)src;
		int numPixels               = width * height;

		int16_t y0, u, y1, v;
		for ( int i = 0; i < numPixels/2; i++, source+=4, dest+=6 ) {
			y0 = source[0];
			u = source[1] + 127;	// convert from +/- 128 to 0..255
			y1 = source[2];
			v = source[3] + 127;	// convert from +/- 128 to 0..255


			dest[0] = y0;
			dest[1] = u;
			dest[2] = v;

			dest[3] = y1;
			dest[4] = u;
			dest[5] = v;

 		}

	}

}
}
