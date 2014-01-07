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
    pimpl_->rect_ = rect;
  }
  
  LeapFrog::Brio::tRect&
  VNRectHotSpot::GetRect(void) const {
    return pimpl_->rect_;
  }
  
  void
  VNRectHotSpot::Trigger(void *input) const {
    bool wasTriggered = pimpl_->isTriggered_;
    pimpl_->Trigger(input, this);
    
    // send appropriate notifications
    if (pimpl_->isTriggered_) {
      VNHotSpotEventMessage *msg = new VNHotSpotEventMessage(LF::Vision::kVNHotSpotTriggeredEvent, this);
      LeapFrog::Brio::CEventMPI eventMPI;
      eventMPI.PostEvent(*msg, 0);
    }
    if (wasTriggered != pimpl_->isTriggered_) {
      VNHotSpotEventMessage *msg = new VNHotSpotEventMessage(LF::Vision::kVNHotSpotTriggerChangeEvent, this);
      LeapFrog::Brio::CEventMPI eventMPI;
      eventMPI.PostEvent(*msg, 0);
    }
  }
  
} // namespace Vision
} // namespace LF
