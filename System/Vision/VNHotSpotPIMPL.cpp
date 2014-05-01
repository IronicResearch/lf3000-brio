#include <VNHotSpotPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <VNVisionMPIPIMPL.h>

LeapFrog::Brio::U32 LF::Vision::VNHotSpotPIMPL::instanceCounter_ = 0;

namespace LF {
namespace Vision {

  cv::Mat *VNHotSpotPIMPL::integralImage_ = NULL;

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
  VNHotSpotPIMPL::Trigger(cv::Mat &input, const VNHotSpot *hs) {
    // do nothing in this method
  }

  bool
  VNHotSpotPIMPL::ContainsPoint(const VNPoint &p) const {
    // the base class does not know about point containment
    return false;
  }

  VNPoint
  VNHotSpotPIMPL::WandPoint(void) {
    static VNPoint p(kVNNoWandLocationX, kVNNoWandLocationY);
    VNVisionMPI mpi;
    VNWand *wand = mpi.pimpl_->GetCurrentWand();
    if (wand)
      return wand->GetLocation();
    return p;
  }

  int
  VNHotSpotPIMPL::GetTriggerImage(cv::Mat &img) {
    // set the image to size zero
    img = triggerImage_;
    return 0;
  }

  bool
  VNHotSpotPIMPL::GetIntegralImage(cv::Mat &img) {
    // default is no integlra image
    return false;
  }

  void
  VNHotSpotPIMPL::UpdateVisionCoordinates(void) {
    // do nothing
  }

  cv::Rect
  VNHotSpotPIMPL::GetBoundingBox(void) const {
    static cv::Rect tmp(0,0,0,0);
    return tmp;
  }
} // namespace Vision
} // namespace LF
