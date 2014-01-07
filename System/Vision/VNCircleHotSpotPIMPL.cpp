#include <VNCircleHotSpotPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNWand.h>
#include <Vision/VNTrigger.h>
#include <opencv2/opencv.hpp>
#include <cmath>

namespace LF {
namespace Vision {

  VNCircleHotSpotPIMPL::VNCircleHotSpotPIMPL(const VNPoint &center,
					     float radius) :
    VNHotSpotPIMPL(),
    center_(center),
    radius_(0),
    r2_(0),
    numPixels_(0) {
    
    SetRadius(radius);
  }
  
  VNCircleHotSpotPIMPL::~VNCircleHotSpotPIMPL(void) {
    
  }
  
  void
  VNCircleHotSpotPIMPL::SetRadius(float radius) {
    if (radius > 0) {
      radius_ = radius;
      r2_ = radius_*radius_;
      numPixels_ = static_cast<int>(M_PI*r2_);
    }
  }

  float
  VNCircleHotSpotPIMPL::GetRadius(void) const {
    return radius_;
  }

  void
  VNCircleHotSpotPIMPL::Trigger(void *input, const VNHotSpot *hs) {
    if (trigger_ != NULL) {
      cv::Rect imgRect(center_.x - radius_,
		       center_.y - radius_,
		       2*radius_,
		       2*radius_);
      cv::Mat *img = static_cast<cv::Mat*>(input);
      if (img && img->rows > 0 && img->cols > 0) {
	triggerImage_ = (*img)(imgRect);
	// create a mask image the size of our ROI
	cv::Mat mask = cv::Mat::zeros(triggerImage_.rows, 
				      triggerImage_.cols, 
				      CV_8UC1);
	// render a circle in the mask
	circle(mask, 
	       cv::Point(0.5*radius_, 0.5*radius_), 
	       radius_, 
	       cv::Scalar(255,255,255), 
	       CV_FILLED);
	// only keep the pixels that are in the circle
	triggerImage_.copyTo(triggerImage_, mask);
      }
      isTriggered_ = trigger_->Triggered(hs);      
    }
  }

  bool
  VNCircleHotSpotPIMPL::ContainsPoint(VNPoint &p) {
    float xd = p.x - center_.x;
    float yd = p.y - center_.y;
    return ((xd*xd + yd*yd) <= r2_);
  }

  int
  VNCircleHotSpotPIMPL::GetTriggerImage(cv::Mat &img) {
    img = triggerImage_;
    return numPixels_;
  }

} // namespace Vision
} // namespace LF
