#include <Vision/VNVisionMPI.h>
#include <VNVisionMPIPIMPL.h>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
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

	/*!
	 * Controls the execution of frame processing
	 */
	void
	VNVisionMPI::Start(LeapFrog::Brio::tVideoSurf& surf) {
		pimpl_->Start(surf);
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
} // namespace Vision
} // namespace LF
