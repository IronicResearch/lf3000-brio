#include <Vision/VNRectHotSpot.h>
#include <VNRectHotSpotPIMPL.h>
#include <Vision/VNVisionTypes.h>

#include <stdio.h>

namespace LF {
namespace Vision {

  VNRectHotSpot::VNRectHotSpot(void) :
    VNHotSpot(),
    pimpl_(new VNRectHotSpotPIMPL() ){
    VNHotSpot::pimpl_ = pimpl_;
  }
  
  VNRectHotSpot::VNRectHotSpot(const LeapFrog::Brio::tRect& rect) :
    VNHotSpot(),
    pimpl_(new VNRectHotSpotPIMPL(rect) ){
    VNHotSpot::pimpl_ = pimpl_;
  }
  
  VNRectHotSpot::~VNRectHotSpot(void) {
    
  }
  
  void
  VNRectHotSpot::SetRect(const LeapFrog::Brio::tRect& rect) {
    if (pimpl_) {
      pimpl_->SetRect(rect);
    }
  }
  
  LeapFrog::Brio::tRect
  VNRectHotSpot::GetRect(void) const {
    if (pimpl_) {
      return pimpl_->GetRect();
    }
    return LeapFrog::Brio::tRect();
  }
  
  void
  VNRectHotSpot::Trigger(cv::Mat &input) const {
    if (pimpl_) {
      pimpl_->Trigger(input, this);
    }
  }
  
} // namespace Vision
} // namespace LF
