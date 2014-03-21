#include <VNWandTrackerPIMPL.h>
#include <VNVisionMPIPIMPL.h>
#include <opencv2/imgproc/types_c.h>
#include <CameraMPI.h>
#include <DebugMPI.h>
#include <GroupEnumeration.h>
#include <stdio.h>
#include <VNRGB2HSV.h>
//#define LF_PROFILE 1
#undef LF_PROFILE
#include <VNProfiler.h>
#include <VNInRange3.h>

namespace LF {
namespace Vision {
  
  static const LeapFrog::Brio::CString kVNAreaToStartScalingKey = "VNWTAreaToStartScaling";
  static const LeapFrog::Brio::CString kVNMinPercentToScaleKey = "VNWTMinPercentToScale";
  static const LeapFrog::Brio::CString kVNMinWandAreaKey = "VNWTMinWandArea";

  static const float kVNDefaultAreaToStartScaling = 1000.0f;
  static const float kVNWandMinAreaDefault = 50.f;
  static const float kVNDefaultMinPercentToScale = 0.3f;
  static const int kVNNoContourIndex = -1;
  static const int kVNMaxNumStepsToComputeCircle = 20;

  bool SameSize(const LeapFrog::Brio::tRect &lfr, const cv::Rect & r) {
    return (lfr.left == r.x &&
	    lfr.top == r.y &&
	    (lfr.right - lfr.left) == r.width &&
	    (lfr.bottom - lfr.top) == r.height);
  }

  void
  VNWandTrackerPIMPL::SetParams(VNInputParameters *params) {

    if (params) {
      VNInputParameters::const_iterator it = params->begin();
      for ( ; it != params->end(); ++it) {
	LeapFrog::Brio::CString key = (*it).first;
	float val = (*it).second;

	// we potentially need to do more error checking on these parameters
	if (key.compare(kVNAreaToStartScalingKey) == 0) {
	  if (val > 0)
	    wandAreaToStartScaling_ = val;
	} else if (key.compare(kVNMinPercentToScaleKey) == 0) {
	  if (val > 0)
	    minPercentToScale_ = val;
	} else if (key.compare(kVNMinWandAreaKey) == 0) {
	  if (val > 0)
	    minArea_ = val;
	}
      }
    }
  }

  VNWandTrackerPIMPL::VNWandTrackerPIMPL(VNWandPIMPL* wand,
					 VNInputParameters *params) :
    wand_(wand),
    scaleInput_(false),
    wandAreaToStartScaling_(kVNDefaultAreaToStartScaling),
    minPercentToScale_(kVNDefaultMinPercentToScale),
    minArea_(kVNWandMinAreaDefault) {

    // the assumption is that the input frames to Execute are of this size
    translator_.SetSourceFrame(cv::Rect(0,
					0,
					kVNVisionProcessingFrameWidth,
					kVNVisionProcessingFrameHeight));
    SetParams(params);
  }
  
  VNWandTrackerPIMPL::~VNWandTrackerPIMPL(void) {
  }
  
  void
  VNWandTrackerPIMPL::Initialize(void) {
    LeapFrog::Brio::tCameraControls controls;
    LeapFrog::Brio::CCameraMPI cameraMPI;
    LeapFrog::Brio::CDebugMPI dbg(kGroupVision);

    LeapFrog::Brio::Boolean err = cameraMPI.GetCameraControls(controls);
    dbg.Assert(err, "VNWandTracker could get camera controls\n");
    
    bool setExposure = false;
    bool setWhiteBlaance = false;
    
    for(LeapFrog::Brio::tCameraControls::iterator i = controls.begin();
	i != controls.end();
	++i) {
      tControlInfo* c = *i;
      
      if(!c) {
	dbg.DebugOut(kDbgLvlCritical, "null camera control (tControlInfo)\n");
	continue;
      } else {
	printf("Control type: %i\n", c->type);
      }
      
      switch(c->type) {
      case kControlTypeAutoWhiteBalance:
	printf("AutoWhiteBalance: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	cameraMPI.SetCameraControl(c, 0);
	printf("New AutoWhiteBalance: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	break;
      case kControlTypeAutoExposure:
	printf("New Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	cameraMPI.SetCameraControl(c, 0);
	printf("New Auto Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	break;
      case kControlTypeExposure:
	printf("Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	cameraMPI.SetCameraControl(c, c->min);
	printf("New Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	break;
      }
    }
  }

  void
  VNWandTrackerPIMPL::ComputeLargestContour(cv::Mat &img, 
					    std::vector<std::vector<cv::Point> > &contours,
					    int &index) {
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(img, 
		     contours, 
		     hierarchy, 
		     CV_RETR_LIST, 
		     CV_CHAIN_APPROX_SIMPLE, 
		     cv::Point(0,0));

    float maxArea = 0.f;
    for (unsigned int i = 0; i < contours.size(); ++i) {
      float f = cv::contourArea(contours[i]);
      if (f > maxArea) {
	maxArea = f;
	index = i;
      }
    }
  }

  void
  VNWandTrackerPIMPL::FitCircleToContour(const std::vector<cv::Point> &contour,
					 cv::Point &center,
					 float &radius) const {
    float d = 0;
    cv::Point m1;
    cv::Point m2;
    bool found = false;
    // limits the number of point pairs we compare to compute the circle raidus
    int step = MAX(1, static_cast<int>(contour.size())/kVNMaxNumStepsToComputeCircle);

    for (unsigned int i = 0; i < contour.size(); i+=step) {
      cv::Point p1 = contour[i];
      for (unsigned int j = i+1; j < contour.size(); ++j) {
	cv::Point p2 = contour[j];
	float cd = (p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y);
	if (cd > d) {
	  d = cd;
	  m1 = p1;
	  m2 = p2;
	  found = true;
	}
      }
    }
    
    if (found) {
      center.x = 0.5*(m1.x + m2.x);
      center.y = 0.5*(m1.y + m2.y);
      radius = 0.5*sqrtf(d);
    }
  }

  void
  VNWandTrackerPIMPL::ScaleSubFrame(const cv::Mat &input,
				    const cv::Point &p,
				    const float radius) {
    float area = M_PI*radius*radius;
    float sf = 1.0f;
    if (area <= minArea_ ) {
      sf = minPercentToScale_;
    } else if (area < wandAreaToStartScaling_) {
      // a linear interpolation from 1.0 down to minPercentToScale_  
      sf = 1.0f - ((wandAreaToStartScaling_ - area)*(1.0f - minPercentToScale_)/(wandAreaToStartScaling_ - minArea_));
    }

    subFrame_.width = sf*input.cols;
    subFrame_.height = sf*input.rows;
    subFrame_.x = 0.5*(input.cols - subFrame_.width);
    subFrame_.y = 0.5*(input.rows - subFrame_.height);

    // set the destination frame for scaling
    translator_.SetDestFrame(subFrame_);
  }

  bool
  VNWandTrackerPIMPL::ScaleWandPoint(cv::Point &p) const {
    if (subFrame_.contains(p)) {
      p = translator_.FromDestToSource(p);
      return true;
    }
    return false;
  }

  void
  VNWandTrackerPIMPL::Execute(cv::Mat &input, cv::Mat &output) {
    PROF_BLOCK_START("VNWandTrackerPIMPL::Execute");  		  
    // switch to HSV color spave
    PROF_BLOCK_START("RGBToHSV");
    RGBToHSV(input, hsv_);    
    PROF_BLOCK_END();
    // filter out the valid pixels based on hue, saturation and intensity
    PROF_BLOCK_START("inRange");
    inRange3( hsv_, wand_->hsvMin_, wand_->hsvMax_, output );
    PROF_BLOCK_END();

    PROF_BLOCK_START("threshold");
    cv::threshold(output, 
		  output, 
		  kVNMinPixelValue, 
		  kVNMaxPixelValue, 
		  cv::THRESH_BINARY);
    PROF_BLOCK_END();

    PROF_BLOCK_START("ComputeLargestContour");
    std::vector<std::vector<cv::Point> > contours;
    int index = kVNNoContourIndex;
    ComputeLargestContour(output, contours, index);
    PROF_BLOCK_END();

    if (index != kVNNoContourIndex) {
      cv::Point p(0,0);
      float r = 0.f;

      PROF_BLOCK_START("FitCircleToContour");
      FitCircleToContour(contours[index], p, r);
      PROF_BLOCK_END();

      bool inFrame = true;
      if (scaleInput_) {
	// insure the size of the input image matches the translator source frame size
	assert(SameSize(translator_.GetSourceFrame(), cv::Rect(0,0,input.cols,input.rows)));
	
	PROF_BLOCK_START("ScaleSubFrame");
	ScaleSubFrame(input,p,r);
	PROF_BLOCK_END();
	
	PROF_BLOCK_START("ScaleWandPoint");
	inFrame = ScaleWandPoint(p);
	PROF_BLOCK_END();
      }
      // insure we have at least a minimum circle area
      if (inFrame && M_PI*r*r > minArea_) {
	wand_->VisibleOnScreen(p);
      } else {
	wand_->NotVisibleOnScreen();
      }
    } else {
      wand_->NotVisibleOnScreen();
    }
    
    PROF_BLOCK_END();	
  }

  void
  VNWandTrackerPIMPL::SetAutomaticWandScaling(bool autoScale) {
    scaleInput_ = autoScale;
  }

  bool
  VNWandTrackerPIMPL::GetAutomaticWandScaling(void) const {
    return scaleInput_;
  }

}
}
