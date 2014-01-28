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
    VNRectHotSpotPIMPL(),
    center_(center),
    radius_(0),
    r2_(0) {
    
    SetRadius(radius);
  }
  
  VNCircleHotSpotPIMPL::~VNCircleHotSpotPIMPL(void) {
    
  }
  
  void
  VNCircleHotSpotPIMPL::CreateMask(void) {
    // create a mask image the size of our ROI
    mask_ = cv::Mat::zeros(2*radius_,
			   2*radius_,
			   CV_8U);
    // render a circle in the mask
    circle(mask_, 
	   cv::Point(radius_, radius_), 
	   radius_, 
	   cv::Scalar(255), 
	   CV_FILLED);
  }

  void
  VNCircleHotSpotPIMPL::SetRadius(float radius) {
    if (radius > 0) {
      radius_ = radius;
      r2_ = radius_*radius_;
      numPixels_ = static_cast<int>(M_PI*r2_);
      SetRect(center_.x - radius_, 
	      center_.x + radius_,
	      center_.y - radius_,
	      center_.y + radius_);
      CreateMask();
    }
  }

  float
  VNCircleHotSpotPIMPL::GetRadius(void) const {
    return radius_;
  }

  void
  VNCircleHotSpotPIMPL::Trigger(cv::Mat &input, const VNHotSpot *hs) {
    if (trigger_ != NULL) {
      if (input.rows > 0 && input.cols > 0) {
	// clip the rect to the intersection of the rect and 
	// the rect bounding the input image, (0,0,width,height)
	cv::Rect clippedRect = ClipRectToImage(input);

	// we need a clipping rectangle for the mask with the appropriate origin and
	// size based on the clippedRect.  To achieve this we will shift the clipped
	// rect by the origin of the original rect	
	cv::Rect adjustedClippedRect = clippedRect - rect_.tl();

	// clip the filtering image such that when we apply the filter
	// to the input image we are applying it in the correct location
	cv::Mat clippedMask = mask_(adjustedClippedRect);

	// NOTE: If we want to calculate the percentage of occlusion based only on
	// the visible portion of the hot spot we should uncomment this line of
	// code and use only the active pixels that are visible
	// because the filter is clipped we need to update the number of active
	// pixels in the final trigger image
	//UpdateNumberOfActivePixels(clippedMask);

	tmpMat_ = (input)(clippedRect);
	// only keep the pixels that are in the circle
	tmpMat_.copyTo(triggerImage_, clippedMask);
      }
      isTriggered_ = trigger_->Triggered(hs);      
    }
  }

  bool
  VNCircleHotSpotPIMPL::ContainsPoint(const VNPoint &p) const {
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
