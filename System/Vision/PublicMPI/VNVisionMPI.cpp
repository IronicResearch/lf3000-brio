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
    pimpl_->algorithm_ = algorithm;
  }
  
  VNAlgorithm*
  VNVisionMPI::GetAlgorithm(void) const {
    return pimpl_->algorithm_;
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
    pimpl_->AddHotSpot(hotSpot);
  }
  
  void
  VNVisionMPI::RemoveHotSpot(const VNHotSpot* hotSpot) {
    pimpl_->RemoveHotSpot(hotSpot);
  }
  
  void
  VNVisionMPI::RemoveHotSpotByID(const LeapFrog::Brio::U32 tag) {
    pimpl_->RemoveHotSpotByID(tag);
  }

  void
  VNVisionMPI::RemoveAllHotSpots(void) {
    pimpl_->RemoveAllHotSpots();
  }

  /*!
   * Controls the execution of frame processing
   */
  LeapFrog::Brio::tErrType
  VNVisionMPI::Start(LeapFrog::Brio::tVideoSurf* surf,
		     bool dispatchSynchronously,
		     const LeapFrog::Brio::tRect *displayRect) {
    return pimpl_->Start(surf, dispatchSynchronously, displayRect);
  }
  
  void
  VNVisionMPI::Update(void) {
    pimpl_->Update();
  }

  LeapFrog::Brio::Boolean
  VNVisionMPI::Stop(void) {
    return pimpl_->Stop();
  }
  
  LeapFrog::Brio::Boolean
  VNVisionMPI::Pause(void) {
    return pimpl_->Pause();
  }
  
  LeapFrog::Brio::Boolean
  VNVisionMPI::Resume(void) {
    return pimpl_->Resume();
  }

  bool
  VNVisionMPI::IsRunning(void) const {
    return pimpl_->visionAlgorithmRunning_;
  }

  void
  VNVisionMPI::SetFrameProcessingRate(float frameProcessingRate) {
    pimpl_->frameProcessingRate_ = frameProcessingRate;
  }
  
  float
  VNVisionMPI::GetFrameProcessingRate(void) const {
    return pimpl_->frameProcessingRate_;
  }
  
} // namespace Vision
} // namespace LF
