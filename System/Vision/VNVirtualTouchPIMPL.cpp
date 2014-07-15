#include <VNVirtualTouchPIMPL.h>
#include "VNRGB2Gray.h"
#undef VN_PROFILE
//#define VN_PROFILE 1
#include <VNProfiler.h>
#include <VNAlgorithmHelpers.h>
#include <GroupEnumeration.h>
#include <CameraMPI.h>
#include <DebugMPI.h>
#include <stdio.h>
#include <VNYUYV2RGB.h>

#define VN_OPTIMIZE_FIXEDPOINT 1
#if VN_OPTIMIZE_FIXEDPOINT
#define VN_OPTIMIZE_FRAME_PASSES 1
#include "VNAccumulate.h"
#include "VNFixedPoint.h"

#define VN_NEON_OPTIMIZE_FRAME_PASSES !defined(EMULATION)

#if VN_NEON_OPTIMIZE_FRAME_PASSES
#include <arm_neon.h>
#endif // VN_NEON_OPTIMIZE_FRAME_PASSES

#endif // VN_OPTIMIZE_FIXEDPOINT

namespace LF {
namespace Vision {

  static const float kVNDownScale = 0.5f;
  static const float kVNUpScale = 1.0f/kVNDownScale;

  VNVirtualTouchPIMPL::VNVirtualTouchPIMPL(float learningRate, int intensityThreshold) :
    learningRate_(learningRate),
    threshold_(intensityThreshold) {

  }

  VNVirtualTouchPIMPL::~VNVirtualTouchPIMPL(void) {

  }

  void
  VNVirtualTouchPIMPL::Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
				  LeapFrog::Brio::U16 frameProcessingHeight) {
    ResetCamera();
  }

  void
  VNVirtualTouchPIMPL::Execute(cv::Mat &input, cv::Mat &output) {

    PROF_FRAMES_PER_SECOND( "Execute");

	  PROF_BLOCK_START("Execute");
	  // initialize background to first frame
	  if (learnedBackground_.total() != input.total() ) {
		  ConvertToGray( input, gray_);
#if VN_OPTIMIZE_FIXEDPOINT
		  gray_.convertTo(learnedBackground_, CV_32S);
#else
		  gray_.convertTo(learnedBackground_, CV_32F);
#endif

	  }

	  PROF_BLOCK_START("AbsDifferenceThreshold");
	  AbsDifferenceThreshold( learnedBackground_, input, output, threshold_, learningRate_);

	  PROF_BLOCK_END();

	  PROF_BLOCK_END(); // Execute

    PROF_PRINT_REPORT_AFTER_COUNT(30);
  }


  void
  VNVirtualTouchPIMPL::ConvertToGray(const cv::Mat& in, cv::Mat& outgray) {

	  switch( in.type() ) {
		  case CV_8UC2: // YUYV
			  LF::Vision::YUYV2Gray(in, outgray);
			  break;

		  case CV_8UC3: // RGB
			  LF::Vision::RGB2Gray(in, outgray);
			  break;

		  default:

			  assert(!"Unsupported image format");
	  }
  }

  void
  VNVirtualTouchPIMPL::ConvertToRGB(const cv::Mat& in, cv::Mat& outrgb) {

	  switch( in.type() ) {
		  case CV_8UC2: // YUYV
			  LF::Vision::YUYV2RGB(in, outrgb);
			  break;

		  case CV_8UC3:
			  outrgb = in.clone();
			  break;

		  default:

			  assert(!"Unsupported image format");
	  }
  }

  void
  #if VN_NEON_OPTIMIZE_FRAME_PASSES
  __attribute__((optimize("O0")))
  #endif
  VNVirtualTouchPIMPL::AbsDifferenceThreshold( cv::Mat& background, cv::Mat &yuyv, cv::Mat& output, int threshold, float alpha) {

	  if( output.empty() ) {
		  output.create(background.size(), CV_8U);
	  }


#if VN_NEON_OPTIMIZE_FRAME_PASSES



	  uint8_t __restrict volatile * yuy2    = yuyv.data;
	  uint8_t __restrict volatile * bg     = background.data;
	  uint8_t __restrict volatile * bg_out = background.data;
	  uint8_t __restrict volatile * out    = output.data;
	  volatile int  numPixels               = (int)background.total();

	  volatile fixed_t a = FLOAT2FIXED( alpha );
	  volatile fixed_t b = FLOAT2FIXED( 1.0f - alpha );

	  volatile uint32_t constants[] = {	a >> FRACT_BITS_D2,	// NOTE: we shift right because we are going to multiply this (see fixed point MULT(x, y) macro)
		  b >> FRACT_BITS_D2,
		  (uint32_t)threshold,
		  255 };

	  asm volatile(
					   // load constants
					   "vld1.32 {q0}, [%5]\n"
					   "0:"	// loop

					   // load gray from yuyv
					   //"vld1.8	{d2},		[%0]! \n"
					   "vld2.8 {d2,d3}, [%0]! \n"

					   // load background
					   "vld1.32	{q2},		[%1]! \n"
					   "vld1.32	{q3},		[%1]! \n"

					   // convert background to int and narrow to 8 bit
					   "vshrn.u32 d8, q2, #16 \n"
					   "vshrn.u32 d9, q3, #16 \n"
					   "vshrn.u16 d8, q4, #8 \n"


					   // absolute difference background - current(gray)
					   "vabd.u8	d9,	d8, d2 \n"	// foreground = d9

					   // apply threshold: foreground > threshold
					   "vdup.8	d10, d1[0] \n"	// load the threshold from constants
             "vcgt.u8	d10, d9, d10 \n"		// greater then threshold

					   // save foreground to output
					   "vst1.8 {d10}, [%2]! \n"

					   // expand gray to 32 bit 8.24 fixed point
					   // s[0] = INT2FIXED( gray.at<uint8_t>(i) );
					   "vmovl.u8 q4, d2 \n"
					   "vmovl.u16 q5, d8 \n"
					   "vmovl.u16 q6, d9 \n"
					   "vshl.u32 q5, q5, #12 \n"  // NOTE: we only shift left 12 and not 24 because we are going to do a multiply below which requires a shift right by 12.  So 24-12 = 12. (see fixed point MULT(x, y) macro)
					   "vshl.u32 q6, q6, #12 \n"  // NOTE: we only shift left 12 and not 24 because we are going to do a multiply below which requires a shift right by 12.  So 24-12 = 12. (see fixed point MULT(x, y) macro)

					   // a * b
					   "vmul.u32 q5, q5, d0[0] \n"
					   "vmul.u32 q6, q6, d0[0] \n"

					   // + b * d
					   "vshr.u32 q2, q2, #12 \n" // prepare background for multiply by doing a shift right >> FRACT_BITS_D2
					   "vshr.u32 q3, q3, #12 \n" // prepare background for multiply by doing a shift right >> FRACT_BITS_D2
					   "vmla.u32 q5, q2, d0[1] \n"
					   "vmla.u32 q6, q3, d0[1] \n"

					   // save back to background
					   "vst1.32 {q5}, [%7]! \n"
					   "vst1.32 {q6}, [%7]! \n"

					   "subs %6, %6, #8 \n"  // BUGBUG: crashes on #8???
					   "bne 0b \n"
					   :
					   :	"r"(yuy2),		// %0
					   "r"(bg),		// %1
					   "r"(out),		// %2
					   "r"(a),			// %3
					   "r"(b),			// %4
					   "r"(constants),	// %5
					   "r"(numPixels),	// %6
					   "r"(bg_out)		// %7
					   :

					   );



#else // VN_NEON_OPTIMIZE_FRAME_PASSES

	  // TODO: convert to gray in loop below
	  // static cv::Mat gray_;
	  ConvertToGray( yuyv, gray_ );

	  int32_t backImage[4], foreground[4];

	  fixed_t d[4], s[4];

	  fixed_t a = FLOAT2FIXED( alpha );
	  fixed_t b = FLOAT2FIXED( 1.0f - alpha );
	  const int32_t sz = background.rows * background.cols;
	  for( int i = 0; i < sz; i+=4 ) {
		  // convertTo
		  backImage[0] = FIXED2INT(background.at<uint32_t>(i));
		  backImage[1] = FIXED2INT(background.at<uint32_t>(i+1));
		  backImage[2] = FIXED2INT(background.at<uint32_t>(i+2));
		  backImage[3] = FIXED2INT(background.at<uint32_t>(i+3));
		  // compute difference between current image and background
		  // cv::absdiff
		  foreground[0] = std::abs(backImage[0] - gray_.at<uint8_t>(i));
		  foreground[1] = std::abs(backImage[1] - gray_.at<uint8_t>(i+1));
		  foreground[2] = std::abs(backImage[2] - gray_.at<uint8_t>(i+2));
		  foreground[3] = std::abs(backImage[3] - gray_.at<uint8_t>(i+3));

		  // apply threshold to foreground image
		  // cv::threshold
		  output.at<uint8_t>(i) = (foreground[0] > threshold) * 255;//? 255 : 0;
		  output.at<uint8_t>(i+1) = (foreground[1] > threshold) * 255;//? 255 : 0;
		  output.at<uint8_t>(i+2) = (foreground[2] > threshold) * 255;//? 255 : 0;
		  output.at<uint8_t>(i+3) = (foreground[3] > threshold) * 255;//? 255 : 0;


		  s[0] = INT2FIXED( gray_.at<uint8_t>(i) );
		  s[1] = INT2FIXED( gray_.at<uint8_t>(i+1) );
		  s[2] = INT2FIXED( gray_.at<uint8_t>(i+2) );
		  s[3] = INT2FIXED( gray_.at<uint8_t>(i+3) );

		  d[0] = background.at<uint32_t>(i);
		  d[1] = background.at<uint32_t>(i+1);
		  d[2] = background.at<uint32_t>(i+2);
		  d[3] = background.at<uint32_t>(i+3);

		  background.at<uint32_t>(i)  = MULT(a, s[0]) + MULT(b, d[0]);
		  background.at<uint32_t>(i+1)  = MULT(a, s[1]) + MULT(b, d[1]);
		  background.at<uint32_t>(i+2)  = MULT(a, s[2]) + MULT(b, d[2]);
		  background.at<uint32_t>(i+3)  = MULT(a, s[3]) + MULT(b, d[3]);
	  }

#endif // VN_NEON_OPTIMIZE_FRAME_PASSES


  }
}
}
