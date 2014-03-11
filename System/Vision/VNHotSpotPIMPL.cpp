#include <VNHotSpotPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNWand.h>

LeapFrog::Brio::U32 LF::Vision::VNHotSpotPIMPL::instanceCounter_ = 0;

namespace LF {
namespace Vision {

  VNHotSpotPIMPL::VNHotSpotPIMPL(void) :
    trigger_(NULL),
    tag_(VNHotSpotPIMPL::instanceCounter_++),
    isTriggered_(false),
    triggerImage_(cv::Size(0,0), CV_8U),
    numPixels_(0) {

  }
  
  VNHotSpotPIMPL::~VNHotSpotPIMPL(void) {
    
  }

  void
  VNHotSpotPIMPL::Trigger(void *input, const VNHotSpot *hs) {
    // do nothing in this method
  }

  bool
  VNHotSpotPIMPL::ContainsPoint(const VNPoint &p) const {
    // the base class does not know about point containment
    return false;
  }

  VNPoint
  VNHotSpotPIMPL::WandPoint(void) {
    VNVisionMPI mpi;
    return mpi.GetWandByID()->GetLocation();
  }

  int
  VNHotSpotPIMPL::GetTriggerImage(cv::Mat &img) {
    // set the image to size zero
    img = triggerImage_;
    return 0;
  }

  void
  VNHotSpotPIMPL::UpdateVisionCoordinates(void) {
    // do nothing
  }
} // namespace Vision
} // namespace LF
