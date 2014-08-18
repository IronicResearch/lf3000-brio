#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>

namespace LF {
namespace Vision {

  VNWand::VNWand(void) :
    pimpl_(new VNWandPIMPL()) {
  }

  VNWand::~VNWand(void) {
  }

  bool
  VNWand::IsVisible(void) const {
    if (pimpl_) {
      return pimpl_->IsVisible();
    }
    return false;
  }

  VNPoint
  VNWand::GetLocation(void) const {
    if (pimpl_) {
      return pimpl_->GetLocation();
    }
    return VNPoint();
  }

} // namespace Vision
} // namespace LF
