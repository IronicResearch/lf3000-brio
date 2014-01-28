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
  VNWandTracker::Execute(cv::Mat &input, cv::Mat &output) {
    pimpl_->Execute(input, output);
  }

} // namespace Vision
} // namespace LF
