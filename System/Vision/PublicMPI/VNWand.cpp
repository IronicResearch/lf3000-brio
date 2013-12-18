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
    return pimpl_->visible_;
  }

  VNPoint
  VNWand::GetLocation(void) const {
    return pimpl_->location_;
  }

} // namespace Vision
} // namespace LF
