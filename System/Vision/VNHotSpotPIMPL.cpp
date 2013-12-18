#include <VNHotSpotPIMPL.h>
#include <Vision/VNVisionMPI.h>

LeapFrog::Brio::U32 LF::Vision::VNHotSpotPIMPL::instanceCounter_ = 0;

namespace LF {
namespace Vision {

  VNHotSpotPIMPL::VNHotSpotPIMPL(void) :
    trigger_(NULL),
    tag_(VNHotSpotPIMPL::instanceCounter_++),
    isTriggered_(false) {

  }
  
  VNHotSpotPIMPL::~VNHotSpotPIMPL(void) {
    
  }

  void
  VNHotSpotPIMPL::Trigger(void *input) {
    // do nothing in this method
  }

} // namespace Vision
} // namespace LF
