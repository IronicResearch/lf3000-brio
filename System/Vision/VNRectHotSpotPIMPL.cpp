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
  VNRectHotSpotPIMPL::Trigger(void *input) {
    if (trigger_ != NULL) {
      cv::Rect imgRect(rect_.left,
		       rect_.top,
		       fabs(rect_.right - rect_.left),
		       fabs(rect_.bottom - rect_.top));
      cv::Mat *img = static_cast<cv::Mat*>(input);
      cv::Mat triggerImg;
      if (img && img->rows > 0 && img->cols > 0) {
	triggerImg = (*img)(imgRect);
      }
      trigger_->SetInputData(VNVisionMPI().GetWandByID()->GetLocation(),
			     imgRect,
			     triggerImg);
      isTriggered_ = trigger_->Triggered();      
    }
  }
} // namespace Vision
} // namespace LF
