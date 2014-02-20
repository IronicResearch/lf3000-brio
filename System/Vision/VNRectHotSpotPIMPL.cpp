#include <VNRectHotSpotPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNWand.h>
#include <Vision/VNTrigger.h>
#include <opencv2/opencv.hpp>
#include <cmath>

namespace LF {
namespace Vision {

  VNRectHotSpotPIMPL::VNRectHotSpotPIMPL(void) {
    SetRect(0,0,0,0);
  }
  
  VNRectHotSpotPIMPL::VNRectHotSpotPIMPL(const LeapFrog::Brio::tRect& rect) {
    SetRect(rect);
  }
  
  VNRectHotSpotPIMPL::~VNRectHotSpotPIMPL(void) {
    
  }
  
  void
  VNRectHotSpotPIMPL::SetRect(LeapFrog::Brio::S16 left,
			      LeapFrog::Brio::S16 right,
			      LeapFrog::Brio::S16 top,
			      LeapFrog::Brio::S16 bottom) {
    lfRect_.left = left;
    lfRect_.right = right;
    lfRect_.top = top;
    lfRect_.bottom = bottom;
    rect_.x = lfRect_.left;
    rect_.y = lfRect_.top;
    rect_.width = fabs(lfRect_.right - lfRect_.left);
    rect_.height = fabs(lfRect_.bottom - lfRect_.top);
  }

  void
  VNRectHotSpotPIMPL::SetRect(const LeapFrog::Brio::tRect &rect) {
    SetRect(rect.left,
	    rect.right,
	    rect.top,
	    rect.bottom);
  }
  
  LeapFrog::Brio::tRect
  VNRectHotSpotPIMPL::GetRect(void) const {
    return lfRect_;
  }

  cv::Rect
  VNRectHotSpotPIMPL::ClipRectToImage(cv::Mat &img) const {
    cv::Rect imgRect(0,0,img.cols,img.rows);
    return rect_ & imgRect;
  }

  void
  VNRectHotSpotPIMPL::UpdateNumberOfActivePixels(const cv::Mat &img) {
    numPixels_ = 0;
    for (int i = 0; i < img.rows; ++i) {
      const VNPixelType* rowData = img.ptr<VNPixelType>(i);
      for (int j = 0; j < img.cols; ++j) {
	if (rowData[j] == kVNMaxPixelValue) {
	  numPixels_++;
	}
      }
    }
  }


  void
  VNRectHotSpotPIMPL::Trigger(cv::Mat &input, const VNHotSpot *hs) {
    if (trigger_ != NULL) {
      if (input.rows > 0 && input.cols > 0) {
	cv::Rect clippedRect = ClipRectToImage(input);
	// TODO: Error check on clipped rect to insure it is inside of input image
	triggerImage_ = (input)(clippedRect);
      }
      isTriggered_ = trigger_->Triggered(hs);      
    }
  }

  bool
  VNRectHotSpotPIMPL::ContainsPoint(const VNPoint &p) const {
    return rect_.contains(cv::Point(p.x, p.y));
  }

  int
  VNRectHotSpotPIMPL::GetTriggerImage(cv::Mat &img) {
    img = triggerImage_;
    // NOTE: If we want to calculate the percentage of occlusion based only on
    // the visible portion of the hot spot we should switch these two lines of
    // code and use only the active pixels that are visible
    //numPixels_ = triggerImage_.cols*triggerImage_.rows;
    numPixels_ = rect_.width*rect_.height;
    return numPixels_;
  }

} // namespace Vision
} // namespace LF
