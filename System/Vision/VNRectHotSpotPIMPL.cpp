#include <VNRectHotSpotPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNWand.h>
#include <Vision/VNTrigger.h>
#include <opencv2/opencv.hpp>
#include <cmath>

namespace LF {
namespace Vision {

  VNRectHotSpotPIMPL::VNRectHotSpotPIMPL(void) {
    rect_.left = 0;
    rect_.right = 0;
    rect_.top = 0;
    rect_.bottom = 0;
  }
  
  VNRectHotSpotPIMPL::VNRectHotSpotPIMPL(const LeapFrog::Brio::tRect& rect) :
    rect_(rect) {
    
  }
  
  VNRectHotSpotPIMPL::~VNRectHotSpotPIMPL(void) {
    
  }
  
  void
  VNRectHotSpotPIMPL::Trigger(void *input, const VNHotSpot *hs) {
    if (trigger_ != NULL) {
      cv::Rect imgRect(rect_.left,
		       rect_.top,
		       fabs(rect_.right - rect_.left),
		       fabs(rect_.bottom - rect_.top));
      cv::Mat *img = static_cast<cv::Mat*>(input);
      if (img && img->rows > 0 && img->cols > 0) {
	triggerImage_ = (*img)(imgRect);
      }
      isTriggered_ = trigger_->Triggered(hs);      
    }
  }

  bool
  VNRectHotSpotPIMPL::ContainsPoint(VNPoint &p) {
    return (p.x >= rect_.left &&
	    p.x <= rect_.right &&
	    p.y >= rect_.top &&
	    p.y <= rect_.bottom);
  }

  int
  VNRectHotSpotPIMPL::GetTriggerImage(cv::Mat &img) {
    img = triggerImage_;
    return triggerImage_.cols*triggerImage_.rows;
  }

} // namespace Vision
} // namespace LF
