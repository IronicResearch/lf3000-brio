#include <VNVirtualTouchPIMPL.h>
#include "VNRGB2Gray.h"
#undef VN_PROFILE 
#include <VNProfiler.h>
#include <VNAlgorithmHelpers.h>
#include <GroupEnumeration.h>
#include <CameraMPI.h>
#include <DebugMPI.h>
#include <stdio.h>

#define VN_OPTIMIZE_FIXEDPOINT 1
#if VN_OPTIMIZE_FIXEDPOINT
#define VN_OPTIMIZE_FRAME_PASSES 1
#include "VNAccumulate.h"
#include "VNFixedPoint.h"
#endif

namespace LF {
namespace Vision {

  static const int kVNDefaultVirtualTouchThreshold = 10;
  static const float kVNDownScale = 0.5f;
  static const float kVNUpScale = 1.0f/kVNDownScale;

  VNVirtualTouchPIMPL::VNVirtualTouchPIMPL(float learningRate) :
    learningRate_(learningRate),
    threshold_(kVNDefaultVirtualTouchThreshold) {
    
  }
  
  VNVirtualTouchPIMPL::~VNVirtualTouchPIMPL(void) {
    
  }

  void
  VNVirtualTouchPIMPL::Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
				  LeapFrog::Brio::U16 frameProcessingHeight) {
    LeapFrog::Brio::tCameraControls controls;
    LeapFrog::Brio::CCameraMPI cameraMPI;
    LeapFrog::Brio::CDebugMPI dbg(LeapFrog::Brio::kGroupVision);
    
    LeapFrog::Brio::Boolean err = cameraMPI.GetCameraControls(controls);
    dbg.Assert(err, "VNWandTracker could get camera controls\n");
    
    // turn on autowhitebalance 
    LeapFrog::Brio::tControlInfo *awb = FindCameraControl(controls, 
							  LeapFrog::Brio::kControlTypeAutoWhiteBalance);
    if (awb) {
      printf("AutoWhiteBalance (min,max,preset,current): %li %li %li %li\n", awb->min, awb->max, awb->preset, awb->current);
      cameraMPI.SetCameraControl(awb, 1); // is a boolean, set to 0 for false 
      printf("New AutoWhiteBalance (min,max,preset,current): %li %li %li %li\n", awb->min, awb->max, awb->preset, awb->current);
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for auto white balance\n");
    }

    // set exposure to default value
    LeapFrog::Brio::tControlInfo *e = FindCameraControl(controls, 
							LeapFrog::Brio::kControlTypeExposure);
    if (e) {
      printf("Exposure (min,max,preset,current): %li %li %li %li\n", e->min, e->max, e->preset, e->current);
      cameraMPI.SetCameraControl(e, e->preset);
      printf("New Exposure(min,max,preset,current) : %li %li %li %li\n", e->min, e->max, e->preset, e->current);
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for exposure\n");
    }

    // turn on auto exposure
    LeapFrog::Brio::tControlInfo *ae = FindCameraControl(controls, 
							 LeapFrog::Brio::kControlTypeAutoExposure);
    if (ae) {
      printf("AutoExposure (min,max,preset,current): %li %li %li %li\n", ae->min, ae->max, ae->preset, ae->current);
      cameraMPI.SetCameraControl(ae, 0); // V4L2_EXPOSURE_AUTO = 0
      printf("New AutoExposure (min,max,preset,current): %li %li %li %li\n", ae->min, ae->max, ae->preset, ae->current);
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for auto exposure\n");
    }
  }

  void
  VNVirtualTouchPIMPL::Execute(cv::Mat &input, cv::Mat &output) {

	  PROF_BLOCK_START("Execute");
	  PROF_BLOCK_START("RGB to GRAY");
	  fast_rgb_to_gray( input, gray_ );
	  PROF_BLOCK_END();
	  // initialize background to first frame
	  if (background_.empty()) {
#if VN_OPTIMIZE_FIXEDPOINT
		  gray_.convertTo(background_, CV_32S);
#else 
		  gray_.convertTo(background_, CV_32F);
#endif 
	  }

	  PROF_BLOCK_START("convertTo 8U");
	  // convert background to 8U
#if VN_OPTIMIZE_FIXEDPOINT
#if VN_OPTIMIZE_FRAME_PASSES == 0
	  LF::Vision::convertFixedPointToU8(background_, backImage_);
#endif
#else
	  background_.convertTo(backImage_, CV_8U);
#endif
	  PROF_BLOCK_END();
#if VN_OPTIMIZE_FRAME_PASSES
	  PROF_BLOCK_START("AbsDifferenceThreshold");
	  AbsDifferenceThreshold( background_, gray_, output, threshold_, learningRate_);
	  PROF_BLOCK_END();
#else // VN_OPTIMIZE_FRAME_PASSES
	  PROF_BLOCK_START("absdiff");
	  // compute difference between current image and background
	  cv::absdiff(backImage_, gray_, foreground_);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("threshold");
	  // apply threshold to foreground image
	  cv::threshold(foreground_, output, threshold_, 255, cv::THRESH_BINARY);
	  PROF_BLOCK_END();
#endif // VN_OPTIMIZE_FRAME_PASSES 
	  PROF_BLOCK_START("accumulateWeighted");
	  // accumulate the background
#if VN_OPTIMIZE_FIXEDPOINT
#if VN_OPTIMIZE_FRAME_PASSES == 0
	  LF::Vision::accumulateWeightedFixedPoint(gray_, background_, learningRate_);
#endif
#else 
	  cv::accumulateWeighted(gray_, background_, learningRate_);
#endif
	  PROF_BLOCK_END();

	  PROF_BLOCK_END(); // Execute
  }


void 
  VNVirtualTouchPIMPL::AbsDifferenceThreshold(cv::Mat& background, cv::Mat &gray, cv::Mat& output, int threshold, float alpha) {

	  if( output.empty() ) {
		  printf("\a output.create\n");
		  output.create(background.size(), CV_8U);
	  }

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
		  foreground[0] = std::abs(backImage[0] - gray.at<uint8_t>(i));
		  foreground[1] = std::abs(backImage[1] - gray.at<uint8_t>(i+1));
		  foreground[2] = std::abs(backImage[2] - gray.at<uint8_t>(i+2));
		  foreground[3] = std::abs(backImage[3] - gray.at<uint8_t>(i+3));

		  // apply threshold to foreground image
		  // cv::threshold
		  output.at<uint8_t>(i) = (foreground[0] > threshold) * 255;//? 255 : 0;
		  output.at<uint8_t>(i+1) = (foreground[1] > threshold) * 255;//? 255 : 0;
		  output.at<uint8_t>(i+2) = (foreground[2] > threshold) * 255;//? 255 : 0;
		  output.at<uint8_t>(i+3) = (foreground[3] > threshold) * 255;//? 255 : 0;


		  s[0] = INT2FIXED( gray.at<uint8_t>(i) );
		  s[1] = INT2FIXED( gray.at<uint8_t>(i+1) );
		  s[2] = INT2FIXED( gray.at<uint8_t>(i+2) );
		  s[3] = INT2FIXED( gray.at<uint8_t>(i+3) );

		  d[0] = background.at<uint32_t>(i);
		  d[1] = background.at<uint32_t>(i+1);
		  d[2] = background.at<uint32_t>(i+2);
		  d[3] = background.at<uint32_t>(i+3);

		  background.at<uint32_t>(i)  = MULT(a, s[0]) + MULT(b, d[0]);
		  background.at<uint32_t>(i+1)  = MULT(a, s[1]) + MULT(b, d[1]);
		  background.at<uint32_t>(i+2)  = MULT(a, s[2]) + MULT(b, d[2]);
		  background.at<uint32_t>(i+3)  = MULT(a, s[3]) + MULT(b, d[3]);
	  }
  }
}
}
