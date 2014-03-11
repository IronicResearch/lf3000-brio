#include <VNWandTrackerPIMPL.h>
#include <opencv2/imgproc/types_c.h>

namespace LF {
namespace Vision {

  static const float kVNWandMinAreaDefault = 50.f;
  static const int kVNNoContourIndex = -1;
  static const int kVNMaxNumStepsToComputeCircle = 20;

  VNWandTrackerPIMPL::VNWandTrackerPIMPL(VNWandPIMPL* wand) :
    wand_(wand),
    minArea_(kVNWandMinAreaDefault) {  
  }
  
  VNWandTrackerPIMPL::~VNWandTrackerPIMPL(void) {
   
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
  VNWandTrackerPIMPL::Execute(cv::Mat &input, cv::Mat &output) {
    // switch to HSV color spave
    cv::cvtColor(input, hsv_, CV_BGR2HSV);

    // filter out the valid pixels based on hue, saturation and intensity
    cv::inRange(hsv_, 
		wand_->hsvMin_,
		wand_->hsvMax_,
		output);

    cv::threshold(output, 
		  output, 
		  kVNMinPixelValue, 
		  kVNMaxPixelValue, 
		  cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point> > contours;
    int index = kVNNoContourIndex;
    ComputeLargestContour(output, contours, index);

    if (index != kVNNoContourIndex) {
      cv::Point p(0,0);
      float r = 0.f;
      FitCircleToContour(contours[index], p, r);
      // insure we have at least a minimum circle area
      if (M_PI*r*r > minArea_) {
	wand_->VisibleOnScreen(p);
      } else {
	wand_->NotVisibleOnScreen();
      }
    } else {
      wand_->NotVisibleOnScreen();
    }    
  }

}
}
