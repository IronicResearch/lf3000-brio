#include <Vision/VNVisionMPI.h>
#include <VNVisionMPIPIMPL.h>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
#include <Vision/VNWand.h>
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
    pimpl_(VNVisionMPIPIMPL::Instance()){
    
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
  
  /*!
   * Return a Wand by ID
   */
  VNWand*
  VNVisionMPI::GetWandByID(LeapFrog::Brio::U32 id) const {
    // for now we have just a single default wand, regardless of the id
    static VNWand* wandInstance = NULL;
    if (wandInstance == NULL) {
      wandInstance = new VNWand();
    }
    return wandInstance;
  }

  /*!
   * Add/Remove hot spots
   */
  void
  VNVisionMPI::AddHotSpot(const VNHotSpot* hotSpot) {
    if (std::find(pimpl_->hotSpots_.begin(),
		  pimpl_->hotSpots_.end(),
		  hotSpot) == (pimpl_->hotSpots_.end())) {
      pimpl_->hotSpots_.push_back(hotSpot);
    }
  }
  
  void
  VNVisionMPI::RemoveHotSpot(const VNHotSpot* hotSpot) {
    std::vector<const VNHotSpot*>::iterator it;
    it = std::find(pimpl_->hotSpots_.begin(), pimpl_->hotSpots_.end(), hotSpot);
    if (it != pimpl_->hotSpots_.end()) {
      pimpl_->hotSpots_.erase(it);
    }
  }
  
  void
  VNVisionMPI::RemoveHotSpotByID(const LeapFrog::Brio::U32 tag) {
    std::vector<const VNHotSpot*>::iterator it = pimpl_->hotSpots_.begin();
    for ( ; it != pimpl_->hotSpots_.end(); ++it) {
      if ((*it)->GetTag() == tag) {	
	it = pimpl_->hotSpots_.erase(it);
	// because erase returns an iterator to the "next" spot we need 
	// to decrement the iterator since we icrement it at the end of each loop
	it--;
      }
    }
  }

  void
  VNVisionMPI::RemoveAllHotSpots(void) {
    pimpl_->hotSpots_.clear();
  }

  /*!
   * Controls the execution of frame processing
   */
  void
  VNVisionMPI::Start(LeapFrog::Brio::tVideoSurf& surf,
		     bool dispatchSynchronously) {
    pimpl_->Start(surf, dispatchSynchronously);
  }
  
  void
  VNVisionMPI::Update(void) {
    pimpl_->Update();
  }

  void
  VNVisionMPI::Stop(void) {
    pimpl_->Stop();
  }
  
  void
  VNVisionMPI::Pause(void) {
    pimpl_->Pause();
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
