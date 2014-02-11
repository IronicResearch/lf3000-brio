#include <Vision/VNRectHotSpot.h>
#include <VNRectHotSpotPIMPL.h>
#include <Vision/VNVisionTypes.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <EventMPI.h>

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
    bool wasTriggered = pimpl_->isTriggered_;
    pimpl_->Trigger(input, this);
    
    // send appropriate notifications
    if (pimpl_->isTriggered_) {
      VNHotSpotEventMessage msg(LF::Vision::kVNHotSpotTriggeredEvent, this);
      LeapFrog::Brio::CEventMPI eventMPI;
      eventMPI.PostEvent(msg, 0);
    }
    if (wasTriggered != pimpl_->isTriggered_) {
      VNHotSpotEventMessage msg(LF::Vision::kVNHotSpotTriggerChangeEvent, this);
      LeapFrog::Brio::CEventMPI eventMPI;
      eventMPI.PostEvent(msg, 0);
    }
  }
  
} // namespace Vision
} // namespace LF
