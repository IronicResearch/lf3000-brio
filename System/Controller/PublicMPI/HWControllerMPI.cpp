#include <Hardware/HWControllerMPI.h>
#include <EventMPI.h>
#include "HWControllerMPIPIMPL.h"
#include <SystemErrors.h>

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
    if(pimpl_)
    	return true;
    else
    	return false;
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
	  if(pimpl_.get())
		  return pimpl_->GetControllerByID(id);
	  else
		  return NULL;
  }

  void
  HWControllerMPI::GetAllControllers(std::vector<HWController*> &controllers) {
    controllers.clear();
    if(pimpl_.get()) pimpl_->GetAllControllers(controllers);
  }

  LeapFrog::Brio::U8
  HWControllerMPI::GetNumberOfConnectedControllers(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetNumberOfConnectedControllers();
	  else
		  return 0;
  }

  LeapFrog::Brio::tErrType
  HWControllerMPI::EnableControllerSync(bool enable) {
	  if(pimpl_.get())
		  return pimpl_->EnableControllerSync(enable);
	  else
		  return LeapFrog::Brio::kNoImplErr;
  }

  LeapFrog::Brio::U8
  HWControllerMPI::GetMaximumNumberOfControllers() {
	  if(pimpl_.get())
		  return pimpl_->GetMaximumNumberOfControllers();
	  else
		  return 0;
  }

  void
  HWControllerMPI::DisconnectAllControllers() {
	  if(pimpl_.get()) pimpl_->DisconnectAllControllers();
  }

} // namespace Hardware
} // namespace LF
