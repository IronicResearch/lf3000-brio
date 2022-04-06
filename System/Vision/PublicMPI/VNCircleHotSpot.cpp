#include <Vision/VNCircleHotSpot.h>
#include <VNCircleHotSpotPIMPL.h>
#include <Vision/VNVisionTypes.h>

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
    if (pimpl_) {
      pimpl_->SetCenter(center);
    }
  }
  
  VNPoint
  VNCircleHotSpot::GetCenter(void) const {
    if (pimpl_) {
      return pimpl_->GetCenter();
    }
    return VNPoint();
  }

  void
  VNCircleHotSpot::SetRadius(float radius) {
    if (pimpl_) {
      pimpl_->SetRadius(radius);
    }
  }

  float
  VNCircleHotSpot::GetRadius(void) const {
    if (pimpl_) {
      return pimpl_->GetRadius();
    }
    return kVNCircleHotSpotDefaultRadius;
  }

  void
  VNCircleHotSpot::Trigger(cv::Mat &input) const {
    if (pimpl_) {
      pimpl_->Trigger(input, this);
    }
  }
  
} // namespace Vision
} // namespace LF
