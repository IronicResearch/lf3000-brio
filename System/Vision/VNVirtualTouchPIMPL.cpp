#include <VNVirtualTouchPIMPL.h>
#include "VNRGB2Gray.h"
#undef LF_PROFILE 
#include <VNProfiler.h>
#include <VNAlgorithmHelpers.h>
#include <GroupEnumeration.h>
#include <CameraMPI.h>
#include <DebugMPI.h>
#include <stdio.h>

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

	  PROF_BLOCK_START("down scale");	  
	  // scale down to half size
	  cv::Mat resizedInput;
	  cv::resize(input, resizedInput, cv::Size(), kVNDownScale, kVNDownScale, cv::INTER_LINEAR);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("RGB to GRAY");
	  fast_rgb_to_gray( resizedInput, gray_ );
	  PROF_BLOCK_END();
	  // initialize background to first frame
	  if (background_.empty())
		  gray_.convertTo(background_, CV_32F);

	  PROF_BLOCK_START("convertTo 8U");
	  // convert background to 8U
	  background_.convertTo(backImage_, CV_8U);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("absdiff");
	  // compute difference between current image and background
	  cv::absdiff(backImage_, gray_, foreground_);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("threshold");
	  // apply threshold to foreground image
	  cv::threshold(foreground_, output, threshold_, 255, cv::THRESH_BINARY);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("up scale");
	  // scale output back to fullsize
	  cv::resize(output, output, cv::Size(), kVNUpScale, kVNUpScale, cv::INTER_LINEAR);
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("accumulateWeighted");
	  // accumulate the background
	  cv::accumulateWeighted(gray_, background_, learningRate_);
	  PROF_BLOCK_END();

	  PROF_BLOCK_END();
  }

}
}
