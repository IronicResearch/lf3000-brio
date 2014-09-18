#include "VNRGB2HSV.h"
#include "VNAlgorithmHelpers.h"
#include <stdio.h>
#if !defined(EMULATION) && defined(LF3000)
#include <arm_neon.h>
#endif

namespace LF {
namespace Vision {

struct rgb_color {
    uint8_t r, g, b;    /* Channel intensities between 0 and 255 */
};

struct hsv_color {
    uint8_t hue;        /* Hue degree between 0 and 255 */
    uint8_t sat;        /* Saturation between 0 (gray) and 255 */
    uint8_t val;        /* Value between 0 (black) and 255 */
};

#define MIN3(x,y,z)	((y) <= (z) ? \
					((x) <= (y) ? (x) : (y)) \
					: \
					((x) <= (z) ? (x) : (z)))

#define MAX3(x,y,z)	((y) >= (z) ? \
					((x) >= (y) ? (x) : (y)) \
					: \
					((x) >= (z) ? (x) : (z)))

// see: http://en.literateprograms.org/RGB_to_HSV_color_space_conversion_%28C%29
void c_int_rgb2hsv( uint8_t* dest, const uint8_t* source, int cnt ) {

	uint8_t rgb_min, rgb_max;
	struct rgb_color* rgb = (struct rgb_color*)source;
	struct hsv_color* hsv = (struct hsv_color*)dest;
	
	for ( int i = 0; i < cnt; i++, rgb++, hsv++ ) {
		// find min and max RGB value
		rgb_min = MIN3(rgb->r, rgb->g, rgb->b);
		rgb_max = MAX3(rgb->r, rgb->g, rgb->b);
		
		hsv->val = rgb_max;
		if ( hsv->val == 0 ) {
			hsv->hue = hsv->sat = 0;
			continue;
		}
		
		// compute saturation
		hsv->sat = 255*((uint32_t)(rgb_max - rgb_min))/((uint32_t)hsv->val);
		if (hsv->sat == 0) {
			hsv->hue = 0;
			continue;
		}
		
		// Compute saturation
		if (rgb_max == rgb->r) {
			hsv->hue = 0 + 43*(rgb->g - rgb->b)/(rgb_max - rgb_min);
		} else if (rgb_max == rgb->g) {
			hsv->hue = 85 + 43*(rgb->b - rgb->r)/(rgb_max - rgb_min);
		} else /* rgb_max == rgb.b */ {
			hsv->hue = 171 + 43*(rgb->r - rgb->g)/(rgb_max - rgb_min);
		}
		
		// OpenCV uses H: 0 - 180: see http://stackoverflow.com/questions/10948589/choosing-correct-hsv-values-for-opencv-thresholding-with-inranges
		//hsv->hue = (((uint32_t)hsv->hue) * 180) / 255;
	}
}

#if !defined(EMULATION) && defined(LF3000)
void neon_int_rgb2hsv( uint8_t * __restrict dest, uint8_t * __restrict source, int cnt ) {
	
	float float_constants[] = { 255.0f, 43.0f };	// d8 {0,1}
	
	// HSV is in d4, d5, d6 (q2, q3/2)
	__asm__ volatile(
		"lsr %2, %2, #3 \n"					// load cnt and right shift 3 == sizeof(rgb)
					 
		// load float constants
		"vld1.32 {d8}, [%3]! \n"		// load constants into q4
		"0: \n"								// loop label
			"vld3.8 {d0-d2}, [%1]! \n "		// load 8 RGB pixels and split components into 3 lanes D0 == [RRRRRRRR], D1 == [GGGGGGGG], D2 = [BBBBBBBB]
			
			// MIN(r,g,b)
			"vmin.u8 d3, d0, d1 \n"
			"vmin.u8 d4, d1, d2 \n"
			"vmin.u8 d5, d3, d4 \n"			// min D5

			// MAX(r,g,b)
			"vmax.u8 d3, d0, d1 \n"
			"vmax.u8 d4, d1, d2 \n"
			"vmax.u8 d6, d3, d4 \n"			// max D6  <--- Val
			// max - min
			"vsub.u8 d3, d6, d5 \n"			// d3 = max - min
			
			// NOTE: min rgisters are now available d4, d5 == q2
				 
			// convert to float: http://stackoverflow.com/questions/14498776/neon-assembly-code-how-to-convert-byte-to-float
			// convert max-min d3 to 32 bit float
			"vmovl.u8 q14, d3 \n"
			"vmovl.u16 q5, d28 \n"
			"vmovl.u16 q6, d29 \n"	// d3 --> q5, q6  :: max-min
			"vcvt.f32.u32  q5, q5 \n"
			"vcvt.f32.u32  q6, q6 \n"
			
			// NOTE: d3 register now available
				 
			// NOTE: Q14 is a scratch register
				 
			// calculate 1.0/val
			// val = max
			// expand val d6 to 32 bit float
			"vmovl.u8 q14, d6 \n"
			"vmovl.u16 q7, d28 \n"
			"vmovl.u16 q8, d29 \n"	// d6 --> q7, q8  :: val
			"vcvt.f32.u32  q7, q7 \n"
			"vcvt.f32.u32  q8, q8 \n"
			// calculate reciprocal 1.0 / val
			"vrecpe.f32 q7, q7 \n"
			"vrecpe.f32 q8, q8 \n"
					 
			
				 	 
			// calculate saturation
			// s = 255 * (max - min) / v
			// q7 = q14 * q5 * q7
			// q8 = q14 * q6 * q8
			"vmul.f32 q7, q5, q7 \n"	// (max - min) * 1/v
			"vmul.f32 q8, q6, q8 \n"	// (max - min) * 1/v
			"vmul.f32 q7, q7, d8[0] \n"	// * 255
			"vmul.f32 q8, q8, d8[0] \n"	// * 255
			// convert back to integer and narrow from f32 to u8
			"vcvt.u32.f32 q7, q7 \n"
			"vcvt.u32.f32 q8, q8 \n"
			"vmovn.u32 d28, q7 \n"
			"vmovn.u32 d29, q8 \n"
			"vmovn.u16 d5, q14 \n"  // <-- saturation is now in D5
					 
				
			// calculate reciprocal 43 * (1.0 / (max - min)) in q5 and q6
			"vrecpe.f32 q5, q5 \n"
			"vmul.f32 q5, q5, d8[1] \n" // * 43.0
			"vrecpe.f32 q6, q6 \n"
			"vmul.f32 q6, q6, d8[1] \n" // * 43.0
					 
			
			// expand Red d0 to float q7, q8
			"vmovl.u8 q14, d0 \n"
			"vmovl.u16 q7, d28 \n"
			"vmovl.u16 q8, d29 \n"	// d0 --> q7, q8
			"vcvt.f32.u32  q7, q7 \n"
			"vcvt.f32.u32  q8, q8 \n"
				 
			// expand Green d1 to float q9, q10
			"vmovl.u8 q14, d1 \n"
			"vmovl.u16 q9, d28 \n"
			"vmovl.u16 q10, d29 \n"	// d1 --> q9, q10
			"vcvt.f32.u32  q9, q9 \n"
			"vcvt.f32.u32  q10, q10 \n"
					 
			
			// expand Blue d2 to float q11, q12
			"vmovl.u8 q14, d2 \n"
			"vmovl.u16 q11, d28 \n"
			"vmovl.u16 q12, d29 \n"	// d1 --> q11, q12
			"vcvt.f32.u32  q11, q11 \n"
			"vcvt.f32.u32  q12, q12 \n"
			
					 
			//  q14 is scratch
					 
			//
			// HUE
			//
					 
		//// ----- if max == r
			//// hue = 0 + 43*(g - b) * 1/(max - min)
			//// part 1: 4 floats
			"vsub.f32 q13, q9, q11\n"	// g - b
			"vmul.f32 q14, q13, q5 \n"	// 43 * (g - b) / delta
			// convert and narrow.
			// Note: use of sign, trickery to preserve signed to unsigned rollover
			"vcvt.s32.f32 q14, q14 \n"
			"vmovn.s32 d30, q14 \n"		// q15(d30) temp
	
			//// part 2
			"vsub.f32 q13, q10, q12 \n"
			"vmul.f32 q14, q13, q6 \n" // 43 * (g - b) / delta
			// narrow to d31
			// Note: use of sign, trickery to preserve signed to unsigned rollover
			"vcvt.s32.f32 q14, q14 \n"
			"vmovn.s32 d31, q14 \n"		// q15(d31)
	
			// final result narrow 16 (Q14: d26, d27) to 8 final result is in D26
			"vmovn.u16 d26, q15 \n"
					 
			"vceq.u8 d3, d6, d0 \n"		// d3 = max == r
		    "vbit.u8 d4, d26, d3 \n"	// move to d4 if max == r

		//// ----- if max == g
			//// hue = 85 + 43*(b - r) * 1/(max - min)
			//// part 1: 4 floats
			"vsub.f32 q13, q11, q7\n"	// b - r
			"vmul.f32 q14, q13, q5 \n"	// 43 * (b - r) / delta
			"vdup.32 q15, d9[0] \n"		// q15 = 85.0
			// convert and narrow d30.
			// Note: use of sign, trickery to preserve signed to unsigned rollover
			"vcvt.s32.f32 q14, q14 \n"
			"vmovn.s32 d30, q14 \n"		// q15(d30) temp

			//// part 2
			"vsub.f32 q13, q12, q8 \n"	// b - r
			"vmul.f32 q14, q13, q6 \n"	// 43 * (b - r) / delta
			// narrow to d31
			// Note: use of sign, trickery to preserve signed to unsigned rollover
			"vcvt.s32.f32 q14, q14 \n"
			"vmovn.s32 d31, q14 \n"		// q15(d31)

			// final result narrow 16 (Q14: d26, d27) to 8 final result is in D26
			"vmovn.u16 d26, q15 \n"
			"vmov.u8 d0, #85 \n"
			"vadd.u8 d26, d26, d0\n" // +85
					 

			//NOTE B float is done freeing q11, q12
			// generate mask
			"vceq.u8 d22, d6, d1 \n"	// d22 = max == g
			"vbic.u8 d23, d22, d3 \n"	// mask = ~comparison0 & comparison1
			"vbit.u8 d4, d26, d23 \n"	// move to d4 if max == g
					 
			
					 
		//// ----- if max == b
			//// hue = 171 + 43*(r - g) * 1/(max - min)
			//// part 1
			"vsub.f32 q13, q7, q9\n"	// r - g
			"vmul.f32 q14, q13, q5 \n"	// 43 * (r - g) / delta
			"vdup.32 q15, d9[1] \n"		// q15 = 171.0
			// convert and narrow d30.
			// Note: use of sign, trickery to preserve signed to unsigned rollover
			"vcvt.s32.f32 q14, q14 \n"
			"vmovn.s32 d30, q14 \n"		// q15(d30) temp

			//// part 2
			"vsub.f32 q13, q8, q10 \n"	// r - g
			"vmul.f32 q14, q13, q6 \n"	// 43 * (r - g) / delta
			// narrow to d31
			// Note: use of sign, trickery to preserve signed to unsigned rollover
			"vcvt.s32.f32 q14, q14 \n"
			"vmovn.s32 d31, q14 \n"		// q15(d31)

			// final result narrow 16 (Q14: d26, d27) to 8 final result is in D26
			"vmovn.u16 d26, q15 \n"
			"vmov.u8 d0, #171 \n"
			"vadd.u8 d26, d26, d0\n" // +171

			"vceq.u8 d23, d6, d2 \n"	// d23 = max == b
			// generate mask
			"vorr.u8 d24, d3, d22\n"	// concatenate previous comparisons, max == r and max == g
			"vbic.u8 d23, d23, d24 \n"	// mask = ~comparison0 & comparison1
			"vbit.u8 d4, d26, d23 \n"	// move to d4 if max == b
					 

			 // store the HSV results in dest
			 "vst3.8 {d4, d5, d6}, [%0]!\n"
		 

		"subs %2, %2, #1 \n"			// decrement cnt--
		"bne 0b \n"						// while cnt != 0
				 :
				 : "r"(dest), "r"(source), "r"(cnt), "r"(float_constants)		// %0, %1, %2, %3
				 : "r4", "r5", "r6"
	);
}

#endif // !EMULATION


void RGBToHSV( const cv::Mat& input, cv::Mat& output ) {
  if (!CheckInputs(input, output, CV_8UC3)) {
    return;
  }

  uint8_t * __restrict dest = output.data;
  uint8_t * __restrict source = input.data;
  int cnt = input.total();


#if defined(EMULATION) || !defined(LF3000)
  c_int_rgb2hsv( dest, source, cnt );
#else // EMULATION
  neon_int_rgb2hsv( dest, source, cnt );
#endif // EMULATION
}

} 	// namespace Vision
} 	// namespace LF
