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
    radius_(0),
    radiusX_(0),
    radiusY_(0) {

    SetRadius(radius);
    SetCenter(center);
  }
  
  VNCircleHotSpotPIMPL::~VNCircleHotSpotPIMPL(void) {
    
  }
  
  void
  VNCircleHotSpotPIMPL::CreateMask(void) {
    if (radiusX_ > 0 && radiusY_ > 0) {
      // create a mask image the size of our ROI
      mask_ = cv::Mat::zeros(2*radiusY_, // number of rows - height
			     2*radiusX_, // number of cols - width
			     CV_8U);
      // render an ellipse in the mask
      ellipse(mask_, 
	     cv::Point(radiusX_, radiusY_), 
	     cv::Size(radiusX_, radiusY_),
	     0,   // rotation angle of ellipse
	     0,   // start angle of ellipse
	     360, // end angle of ellipse
	     cv::Scalar(255), 
	     CV_FILLED);
    }
  }

  void
  VNCircleHotSpotPIMPL::UpdateVisionCoordinates(void) {
    // there is some redundant behavior in calling UpdateVisionCoordinates on
    // the parent class and then setting the rect (in the parent class) however
    // we would miss another update if one gets added to the parent class 
    //inside of UpdateVisionCoordinates
    VNRectHotSpotPIMPL::UpdateVisionCoordinates();

    // set rect in display coordinates...VNRectHotSpotPIMPL handles
    // the transformation from display to vision coordinates
    SetRect(center_.x - radius_, 
	    center_.x + radius_,
	    center_.y - radius_,
	    center_.y + radius_);

    visionCenter_ = translator_->FromDisplayToVision(center_);
    radiusX_ = translator_->FromDisplayToVisionAlongX(radius_);
    radiusY_ = translator_->FromDisplayToVisionAlongY(radius_);
    
    numPixels_ = static_cast<int>(M_PI*radiusX_*radiusY_);

    CreateMask();
  }

  void
  VNCircleHotSpotPIMPL::SetCenter(const VNPoint &c) {
    center_ = c;
    UpdateVisionCoordinates();
  }

  VNPoint
  VNCircleHotSpotPIMPL::GetCenter(void) const {
    return center_;
  }

  void
  VNCircleHotSpotPIMPL::SetRadius(float radius) {
    if (radius > 0) {
      // store radius in display coordinates
      radius_ = radius;
      UpdateVisionCoordinates();
    }
  }

  float
  VNCircleHotSpotPIMPL::GetRadius(void) const {
    return radius_;
  }

  void
  VNCircleHotSpotPIMPL::UpdateTrigger(cv::Mat &input, const VNHotSpot *hs) {
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
	// TODO: Error check on adjustedClippedRect to insure it's inside of mask_ bounds
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
      if (trigger_) {
	isTriggered_ = trigger_->Triggered(hs);
      }
    }
  }

  bool
  VNCircleHotSpotPIMPL::ContainsPoint(const VNPoint &p) const {
    float xd = p.x - visionCenter_.x;
    float yd = p.y - visionCenter_.y;
    return ((((xd*xd)/(radiusX_*radiusX_)) + ((yd*yd)/(radiusY_*radiusY_))) < 1.0f);
  }

  int
  VNCircleHotSpotPIMPL::GetTriggerImage(cv::Mat &img) {
    img = triggerImage_;
    return numPixels_;
  }

  bool
  VNCircleHotSpotPIMPL::GetIntegralImage(cv::Mat &img) {
    // do not use integral image
    return false;
  }

} // namespace Vision
} // namespace LF
