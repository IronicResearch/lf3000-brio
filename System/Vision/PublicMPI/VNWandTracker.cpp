#include <Vision/VNWandTracker.h>
#include <VNWandTrackerPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {


  VNWandTracker::VNWandTracker(void) :
    pimpl_(new VNWandTrackerPIMPL(&*(VNVisionMPI().GetWandByID()->pimpl_))) {
  }
  
  VNWandTracker::~VNWandTracker(void) {  
  }
  
  void
  VNWandTracker::Execute(void *input, void *output) {
    pimpl_->Execute(static_cast<cv::Mat*>(input), static_cast<cv::Mat*>(output));
  }

} // namespace Vision
} // namespace LF
