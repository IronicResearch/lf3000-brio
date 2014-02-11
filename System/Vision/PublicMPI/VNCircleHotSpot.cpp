#include <Vision/VNCircleHotSpot.h>
#include <VNCircleHotSpotPIMPL.h>
#include <Vision/VNVisionTypes.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <EventMPI.h>

static const float kVNCircleHotSpotDefaultRadius = 0.0f;

namespace LF {
namespace Vision {

  VNCircleHotSpot::VNCircleHotSpot(void) :
    VNHotSpot(),
    pimpl_(new VNCircleHotSpotPIMPL(VNPoint(), kVNCircleHotSpotDefaultRadius)) {
    VNHotSpot::pimpl_ = pimpl_;
  }
  
  VNCircleHotSpot::VNCircleHotSpot(const VNPoint &center,
				   float radius) :
    VNHotSpot(),
    pimpl_(new VNCircleHotSpotPIMPL(center, radius)){
    VNHotSpot::pimpl_ = pimpl_;
  }
  
  VNCircleHotSpot::~VNCircleHotSpot(void) {
    
  }
  
  void
  VNCircleHotSpot::SetCenter(const VNPoint &center) {
    pimpl_->center_ = center;
  }
  
  VNPoint
  VNCircleHotSpot::GetCenter(void) const {
    return pimpl_->center_;
  }

  void
  VNCircleHotSpot::SetRadius(float radius) {
    pimpl_->SetRadius(radius);
  }

  float
  VNCircleHotSpot::GetRadius(void) const {
    return pimpl_->GetRadius();
  }

  void
  VNCircleHotSpot::Trigger(cv::Mat &input) const {
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
