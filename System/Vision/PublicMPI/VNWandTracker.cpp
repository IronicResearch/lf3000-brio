#include <Vision/VNWandTracker.h>
#include <VNWandTrackerPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <VNVisionMPIPIMPL.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {


  VNWandTracker::VNWandTracker(VNInputParameters *params) :
    pimpl_(new VNWandTrackerPIMPL(VNVisionMPI().pimpl_->GetCurrentWand(),
				  params)) {
  }
  
  VNWandTracker::~VNWandTracker(void) {
  }
  
  void
  VNWandTracker::Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
			    LeapFrog::Brio::U16 frameProcessingHeight) {
    if (pimpl_) {
      pimpl_->Initialize(frameProcessingWidth,
			 frameProcessingHeight);
    }
  }

  void
  VNWandTracker::Execute(cv::Mat &input, cv::Mat &output) {
    if (pimpl_) {
      pimpl_->Execute(input, output);
    }
  }

  void
  VNWandTracker::Shutdown(void) {
    if (pimpl_) {
      pimpl_->Shutdown();
    }
  }

  void
  VNWandTracker::SetAutomaticWandScaling(bool autoScale) {
    if (pimpl_) {
      pimpl_->SetAutomaticWandScaling(autoScale);
    }
  }

  bool
  VNWandTracker::GetAutomaticWandScaling(void) const {
    if (pimpl_) {
      return pimpl_->GetAutomaticWandScaling();
    }
    return false;
  }

} // namespace Vision
} // namespace LF
