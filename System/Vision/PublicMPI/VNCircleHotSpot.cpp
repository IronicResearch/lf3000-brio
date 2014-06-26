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
    pimpl_->SetCenter(center);
  }
  
  VNPoint
  VNCircleHotSpot::GetCenter(void) const {
    return pimpl_->GetCenter();
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
    pimpl_->Trigger(input, this);
  }
  
} // namespace Vision
} // namespace LF
