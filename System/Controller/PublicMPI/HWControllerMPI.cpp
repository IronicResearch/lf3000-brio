#include <Hardware/HWControllerMPI.h>
#include <EventMPI.h>
#include "HWControllerMPIPIMPL.h"

namespace LF {
namespace Hardware {

  class HWController;

  static const LeapFrog::Brio::CString kHWControllerMPIName("HWControllerMPI");
  static const LeapFrog::Brio::CString kHWControllerMPIModuleName("HWControllerMPI-Module");
  static const LeapFrog::Brio::CURI kHWControllerMPIURI("/LF/System/Controller");
  static const LeapFrog::Brio::tVersion kHWControllerMPIVersion(0.9);

  /*!
   * \brief Constructor
   */
  HWControllerMPI::HWControllerMPI(void) :
    LeapFrog::Brio::ICoreMPI(),
    pimpl_(HWControllerMPIPIMPL::Instance()) {
  }

  /*!
   * \brief Destructor
   */
  HWControllerMPI::~HWControllerMPI(void) {
  }

  LeapFrog::Brio::Boolean
  HWControllerMPI::IsValid(void) const {
    // TODO: determine if valid
    return true;
  }
  const LeapFrog::Brio::CString*
  HWControllerMPI::GetMPIName(void) const {
    return &kHWControllerMPIName;
  }

  LeapFrog::Brio::tVersion
  HWControllerMPI::GetModuleVersion(void) const {
    return kHWControllerMPIVersion;
  }

  const LeapFrog::Brio::CString*
  HWControllerMPI::GetModuleName(void) const {
    return &kHWControllerMPIModuleName;
  }

  const LeapFrog::Brio::CURI*
  HWControllerMPI::GetModuleOrigin(void) const {
    return &kHWControllerMPIURI;
  }

  LeapFrog::Brio::tErrType
  HWControllerMPI::RegisterEventListener(const LeapFrog::Brio::IEventListener *listener) {
    LeapFrog::Brio::CEventMPI evtmgr;
    return evtmgr.RegisterEventListener(listener);
  }

  LeapFrog::Brio::tErrType
  HWControllerMPI::UnregisterEventListener(const LeapFrog::Brio::IEventListener *listener) {
    LeapFrog::Brio::CEventMPI evtmgr;
    return evtmgr.UnregisterEventListener(listener);
  }

  HWController*
  HWControllerMPI::GetControllerByID(LeapFrog::Brio::U32 id) {
    pimpl_->GetControllerByID(id);
  }

  void
  HWControllerMPI::GetAllControllers(std::vector<HWController*> &controllers) {
    controllers.clear();
    pimpl_->GetAllControllers(controllers);
  }

  LeapFrog::Brio::U8
  HWControllerMPI::GetNumberOfConnectedControllers(void) const {
    return pimpl_->GetNumberOfConnectedControllers();
  }

  LeapFrog::Brio::tErrType
  HWControllerMPI::EnableControllerSync(bool enable) {
	  return pimpl_->EnableControllerSync(enable);
  }

  LeapFrog::Brio::U8
  HWControllerMPI::GetMaximumNumberOfControllers() {
	  return pimpl_->GetMaximumNumberOfControllers();
  }

} // namespace Hardware
} // namespace LF
