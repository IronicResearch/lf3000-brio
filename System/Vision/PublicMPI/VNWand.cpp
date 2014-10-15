#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <Utility.h>

namespace LF {
namespace Vision {

  VNWand::VNWand(void) :
    pimpl_(static_cast<VNWandPIMPL*>(NULL)) {

	  if(HasPlatformCapability(kCapsVision) && HasPlatformCapability(kCapsGamePadController)) {
		  pimpl_.reset(new VNWandPIMPL());
	  } else {
		LeapFrog::Brio::CDebugMPI localDbgMPI(kGroupVision);
		localDbgMPI.DebugOut(kDbgLvlImportant, "VNWand::VNWand() called on a platform which does not support vision and hand-held controllers\n");
	  }
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
