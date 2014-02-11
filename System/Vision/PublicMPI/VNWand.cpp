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
    return pimpl_->IsVisible();
  }

  VNPoint
  VNWand::GetLocation(void) const {
    return pimpl_->GetLocation();
  }

} // namespace Vision
} // namespace LF
