#include <VNWandTrackerPIMPL.h>
#include <VNVisionMPIPIMPL.h>
#include <opencv2/imgproc/types_c.h>
#include <CameraMPI.h>
#include <DebugMPI.h>
#include <GroupEnumeration.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <stdio.h>
#include <VNRGB2HSV.h>
#include <VNIntegralImage.h>
//#define VN_PROFILE 1
#undef VN_PROFILE
#include <VNProfiler.h>
#include <VNInRange3.h>
#include <VNAlgorithmHelpers.h>
#include <VNYUYV2RGB.h>

#if defined(EMULATION)
#include <opencv2/highgui/highgui.hpp>
#endif

#define VN_USE_INTEGRAL_IMAGE_SEARCH 1

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

  VNWandTrackerPIMPL::VNWandTrackerPIMPL(VNWand* wand,
					 VNInputParameters *params) :
    wand_(wand),
    scaleInput_(false),
    wandAreaToStartScaling_(kVNDefaultAreaToStartScaling),
    minPercentToScale_(kVNDefaultMinPercentToScale),
    minArea_(kVNWandMinAreaDefault) {

    SetParams(params);
  }
  
  VNWandTrackerPIMPL::~VNWandTrackerPIMPL(void) {
  }
  
  void
  VNWandTrackerPIMPL::SetWand(VNWand *wand) {
    wand_ = wand;
  }

  void
  VNWandTrackerPIMPL::SetProcessingFrameSize(LeapFrog::Brio::U16 width,
					     LeapFrog::Brio::U16 height) {
    translator_.SetSourceFrame(cv::Rect(0,
					0,
					width,
					height));
  }

  void
  VNWandTrackerPIMPL::Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
				 LeapFrog::Brio::U16 frameProcessingHeight) {
    SetProcessingFrameSize(frameProcessingWidth,
			   frameProcessingHeight);
    LeapFrog::Brio::tCameraControls controls;
    LeapFrog::Brio::CCameraMPI cameraMPI;
    LeapFrog::Brio::CDebugMPI dbg(kGroupVision);

    LeapFrog::Brio::Boolean err = cameraMPI.GetCameraControls(controls);
    dbg.Assert(err, "VNWandTracker could get camera controls\n");
    
    // turn off autowhitebalance 
    LeapFrog::Brio::tControlInfo *awb = FindCameraControl(controls, 
							  LeapFrog::Brio::kControlTypeAutoWhiteBalance);
    if (awb) {
      printf("AutoWhiteBalance (min,max,preset,current): %li %li %li %li\n", awb->min, awb->max, awb->preset, awb->current);
      cameraMPI.SetCameraControl(awb, 0); // is a boolean, set to 0 for false 
      printf("New AutoWhiteBalance (min,max,preset,current): %li %li %li %li\n", awb->min, awb->max, awb->preset, awb->current);
    } else {
      dbg.DebugOut(kDbgLvlCritical, "null camera control for auto white balance\n");
    }

    // turn off auto exposure
    LeapFrog::Brio::tControlInfo *ae = FindCameraControl(controls, 
							 LeapFrog::Brio::kControlTypeAutoExposure);
    if (ae) {
      printf("AutoExposure (min,max,preset,current): %li %li %li %li\n", ae->min, ae->max, ae->preset, ae->current);
      cameraMPI.SetCameraControl(ae, 1); // V4L2_EXPOSURE_MANUAL == 1
      printf("New AutoExposure (min,max,preset,current): %li %li %li %li\n", ae->min, ae->max, ae->preset, ae->current);
    } else {
      dbg.DebugOut(kDbgLvlCritical, "null camera control for auto exposure\n");
    }

    // set exposure to minimum
    LeapFrog::Brio::tControlInfo *e = FindCameraControl(controls, 
							LeapFrog::Brio::kControlTypeExposure);
    if (e) {
      printf("Exposure (min,max,preset,current): %li %li %li %li\n", e->min, e->max, e->preset, e->current);
      cameraMPI.SetCameraControl(e, e->min);
      printf("New Exposure(min,max,preset,current) : %li %li %li %li\n", e->min, e->max, e->preset, e->current);
    } else {
      dbg.DebugOut(kDbgLvlCritical, "null camera control for exposure\n");
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
    if (wand_) {
      PROF_BLOCK_START("VNWandTrackerPIMPL::Execute");  		  
      // switch to HSV color spave
      

	  PROF_BLOCK_START("YUYV to RGB");
	  ConvertToRGB(input, rgb_);
	  PROF_BLOCK_END();
		
	  PROF_BLOCK_START("RGBToHSV");
      RGBToHSV(rgb_, hsv_);
      PROF_BLOCK_END();
		
      // filter out the valid pixels based on hue, saturation and intensity
      PROF_BLOCK_START("inRange");
      inRange3( hsv_, wand_->pimpl_->hsvMin_, wand_->pimpl_->hsvMax_, output );
      PROF_BLOCK_END();
      
	/* redundant from inrange???
      PROF_BLOCK_START("threshold");
      cv::threshold(output, 
		    output, 
		    kVNMinPixelValue, 
		    kVNMaxPixelValue, 
		    cv::THRESH_BINARY);
      PROF_BLOCK_END();
      */
#if defined(EMULATION)
      cv::namedWindow("threshold");
      cv::imshow("threshold", output);
#endif
      
#if VN_USE_INTEGRAL_IMAGE_SEARCH
		PROF_BLOCK_START("integral");
		//cv::integral(output, integral);
		LF::Vision::IntegralImage( output, integral_);
		PROF_BLOCK_END();
		
		PROF_BLOCK_START("findLight");
		cv::Point p;
		bool foundLight = FindLight(integral_, p);
		PROF_BLOCK_END();
		
		if ( foundLight ) {
			wand_->pimpl_->VisibleOnScreen(p);
			
			static bool report = false;
			if ( !report ) {
				printf("\n\n\a\aFOUND LIGHT (%d, %d)\n", p.x, p.y);
				report = true;
			}
		} else {
			wand_->pimpl_->NotVisibleOnScreen();
		}

#else // VN_USE_INTEGRAL_IMAGE_SEARCH
      
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
	  wand_->pimpl_->VisibleOnScreen(p);
	} else {
	  wand_->pimpl_->NotVisibleOnScreen();
	}
      } else {
	wand_->pimpl_->NotVisibleOnScreen();
      }
#endif // VN_USE_INTEGRAL_IMAGE_SEARCH
      PROF_BLOCK_END();	
    }
  }

  void
  VNWandTrackerPIMPL::SetAutomaticWandScaling(bool autoScale) {
    scaleInput_ = autoScale;
  }

  bool
  VNWandTrackerPIMPL::GetAutomaticWandScaling(void) const {
    return scaleInput_;
  }

  void
  VNWandTrackerPIMPL::ConvertToRGB(const cv::Mat& in, cv::Mat& outrgb) {

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
	
     int VNWandTrackerPIMPL::integralSum(const cv::Mat &integral, cv::Rect &roi) {
        int tl = integral.at<unsigned int>(roi.y, roi.x);
        int tr = integral.at<unsigned int>(roi.y, roi.x+roi.width);
        int bl = integral.at<unsigned int>(roi.y+roi.height, roi.x);
        int br = integral.at<unsigned int>(roi.y+roi.height, roi.x+roi.width);
        return br-bl-tr+tl;
    }

	
    bool VNWandTrackerPIMPL::FindLight(const cv::Mat &integral, cv::Point &c) {
        static const int dx = 5;
        static const int dy = 5;
        static const int xMid = 2;
        static const int yMid = 2;
        c.x = 0;
        c.y = 0;
        
        int numX = integral.cols/dx;
        int numY = integral.rows/dy;
        cv::Point result(0,0);
        int sumX = 0, sumY = 0, sum = 0;
        
        for (int i = 0; i < numX; ++i) {
            for (int j = 0; j < numY; ++j) {
                cv::Rect rect(i*dx, j*dy, dx, dy);
                int val = integralSum(integral, rect);
                if (val > 0.7*dx*dy) {
                    sum += val;
                    sumX += val*(i*dx+xMid);
                    sumY += val*(j*dy+yMid);
                }
            }
        }
        //std::cout << "sum = " << sum << std::endl;
        if (sum > 30) {
            c.x = float(sumX)/float(sum);
            c.y = float(sumY)/float(sum);
            //std::cout << "center = " << c.x << ", " << c.y << std::endl;
            return true;
        }
        return false;
    }

}
}
