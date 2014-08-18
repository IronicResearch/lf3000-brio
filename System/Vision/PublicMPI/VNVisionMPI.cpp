#include <Vision/VNVisionMPI.h>
#include <VNVisionMPIPIMPL.h>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <algorithm>

static const LeapFrog::Brio::CString kVNVisionMPIName("VNVisionMPI");
static const LeapFrog::Brio::CString kVNVisionMPIModuleName("VNVisionMPI-Module");
static const LeapFrog::Brio::CURI kVNVisionMPIURI("NotSureWhatToPutHere");
static const LeapFrog::Brio::tVersion kVNVisionMPIVersion(0.9);

namespace LF {
namespace Vision {

  /*!
   * Constructor and Destructor
   */
  VNVisionMPI::VNVisionMPI(void) :
    pimpl_(VNVisionMPIPIMPL::Instance()) {
  }
  
  VNVisionMPI::~VNVisionMPI(void) {
    
  }
  
  /*!
   * Base class virtual methods
   */
  LeapFrog::Brio::Boolean
  VNVisionMPI::IsValid(void) const {
    //TODO: verify it's valid
    return 1;
  }
  
  const LeapFrog::Brio::CString*
  VNVisionMPI::GetMPIName(void) const {
    return &kVNVisionMPIName;
  }
  
  LeapFrog::Brio::tVersion
  VNVisionMPI::GetModuleVersion(void) const {
    return kVNVisionMPIVersion;
  }
  
  const LeapFrog::Brio::CString*
  VNVisionMPI::GetModuleName(void) const {
    return &kVNVisionMPIModuleName;
  }
  
  const LeapFrog::Brio::CURI*
  VNVisionMPI::GetModuleOrigin(void) const {
    return &kVNVisionMPIURI;
  }
  
  /*!
   * Sets/Gets the computer vision algorithm
   */
  void
  VNVisionMPI::SetAlgorithm(VNAlgorithm* algorithm) {
    if (pimpl_) {
      pimpl_->algorithm_ = algorithm;
    }
  }
  
  VNAlgorithm*
  VNVisionMPI::GetAlgorithm(void) const {
    if (pimpl_) {
      return pimpl_->algorithm_;
    }
    return NULL;
  }
  
  VNWand*
  VNVisionMPI::GetWandByID(LeapFrog::Brio::U32 id) const {
    return NULL;
  }

  /*!
   * Add/Remove hot spots
   */
  void
  VNVisionMPI::AddHotSpot(const VNHotSpot* hotSpot) {
    if (pimpl_) {
      pimpl_->AddHotSpot(hotSpot);
    }
  }
  
  void
  VNVisionMPI::RemoveHotSpot(const VNHotSpot* hotSpot) {
    if (pimpl_) {
      pimpl_->RemoveHotSpot(hotSpot);
    }
  }
  
  void
  VNVisionMPI::RemoveHotSpotByID(const LeapFrog::Brio::U32 tag) {
    if (pimpl_) {
      pimpl_->RemoveHotSpotByID(tag);
    }
  }

  void
  VNVisionMPI::RemoveAllHotSpots(void) {
    if (pimpl_) {
      pimpl_->RemoveAllHotSpots();
    }
  }

  /*!
   * Controls the execution of frame processing
   */
  LeapFrog::Brio::tErrType
  VNVisionMPI::Start(LeapFrog::Brio::tVideoSurf* surf,
		     bool dispatchSynchronously,
		     const LeapFrog::Brio::tRect *displayRect) {
    if (pimpl_) {
      return pimpl_->Start(surf, dispatchSynchronously, displayRect);
    }
    return kNoErr;
  }
  
  void
  VNVisionMPI::Update(void) {
    // do nothing
    // this method has been deprecated and will/should be removed
  }

  LeapFrog::Brio::Boolean
  VNVisionMPI::Stop(void) {
    if (pimpl_) {
      return pimpl_->Stop();
    }
    return static_cast<LeapFrog::Brio::Boolean>(1);
  }
  
  LeapFrog::Brio::Boolean
  VNVisionMPI::Pause(void) {
    if (pimpl_) {
      return pimpl_->Pause();
    }
    return static_cast<LeapFrog::Brio::Boolean>(1);
  }
  
  LeapFrog::Brio::Boolean
  VNVisionMPI::Resume(void) {
    if (pimpl_) {
      return pimpl_->Resume();
    }
    return static_cast<LeapFrog::Brio::Boolean>(1);
  }

  bool
  VNVisionMPI::IsRunning(void) const {
    if (pimpl_) {
      return pimpl_->visionAlgorithmRunning_;
    }
    return false;
  }

  void
  VNVisionMPI::SetFrameProcessingRate(float frameProcessingRate) {
    if (pimpl_) {
      pimpl_->frameProcessingRate_ = frameProcessingRate;
    }
  }
  
  float
  VNVisionMPI::GetFrameProcessingRate(void) const {
    if (pimpl_) {
      return pimpl_->frameProcessingRate_;
    }
    return 0.0f;
  }
  
} // namespace Vision
} // namespace LF
