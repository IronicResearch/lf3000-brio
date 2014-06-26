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
    pimpl_->SetRect(rect);
  }
  
  LeapFrog::Brio::tRect
  VNRectHotSpot::GetRect(void) const {
    return pimpl_->GetRect();
  }
  
  void
  VNRectHotSpot::Trigger(cv::Mat &input) const {
    pimpl_->Trigger(input, this);
  }
  
} // namespace Vision
} // namespace LF
