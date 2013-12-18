#include <Hardware/HWAnalogStickMPI.h>
#include <EventMPI.h>
#include "HWAnalogStickPIMPL.h"

namespace LF {
namespace Hardware {

  static const LeapFrog::Brio::CString kHWAnalogStickMPIName("HWAnalogStickMPI");
  static const LeapFrog::Brio::CString kHWAnalogStickMPIModuleName("HWAnalogStickMPI-Module");
  static const LeapFrog::Brio::CURI kHWAnalogStickMPIURI("NotSureWhatToPutHere");
  static const LeapFrog::Brio::tVersion kHWAnalogStickMPIVersion(0.9);
  
  /*!
   * \brief Constructor
   */
  HWAnalogStickMPI::HWAnalogStickMPI(void) 
    :	pimpl_(HWAnalogStickPIMPL::Instance())
  {
  }
  
  /*!
   * \brief Destructor
   */
  HWAnalogStickMPI::~HWAnalogStickMPI(void) {
  }
  
  /*!
   * \defgroup Virtual Base Class Methods
   * \brief These five methods are declared as virtual in the base class, ICoreMPI
   */
  LeapFrog::Brio::Boolean        
  HWAnalogStickMPI::IsValid(void) const {
    // TODO: determine if valid
    return true;
  }
  const LeapFrog::Brio::CString* 
  HWAnalogStickMPI::GetMPIName(void) const {
    return &kHWAnalogStickMPIName;
  }
  
  LeapFrog::Brio::tVersion       
  HWAnalogStickMPI::GetModuleVersion(void) const {
    return kHWAnalogStickMPIVersion;
  }
  
  const LeapFrog::Brio::CString* 
  HWAnalogStickMPI::GetModuleName(void) const {
    return &kHWAnalogStickMPIModuleName;
  }
  
  const LeapFrog::Brio::CURI*    
  HWAnalogStickMPI::GetModuleOrigin(void) const {
    return &kHWAnalogStickMPIURI;
  }
  
  /*!
   * \brief Register an event listener (via EventMPI) and enable continuous sampleing mode
   * \param listener The event listener HWAnalogStickMessage events will be sent to
   */
  LeapFrog::Brio::tErrType 
  HWAnalogStickMPI::RegisterEventListener(const LeapFrog::Brio::IEventListener *listener) {
    LeapFrog::Brio::CEventMPI evtmgr;
    return evtmgr.RegisterEventListener(listener);
  }
  
  /*!
   * \brief Unregister event listener (via EventMPI) and siable all sampling
   * \param listener The event listener to unregister
   */
  LeapFrog::Brio::tErrType 
  HWAnalogStickMPI::UnregisterEventListener(const LeapFrog::Brio::IEventListener *listener) {
    LeapFrog::Brio::CEventMPI evtmgr;
    return evtmgr.UnregisterEventListener(listener);
  }
  
  /*!
   * \breif A method to check for the presence of an analog stick
   * \return true if an analog stick device is present
   */
  LeapFrog::Brio::Boolean 
  HWAnalogStickMPI::IsAnalogStickPresent(void) const {
    return pimpl_->IsAnalogStickPresent();
  }
  
  /*!
   * \brief Obtains the data for the current analog stick
   * \param stickID the id associated with the analog stick to query
   * \return The data associated with the analog stick of id stickID
   */
  tHWAnalogStickData 
  HWAnalogStickMPI::GetAnalogStickData(const LeapFrog::Brio::U8 stickID) const {
    return pimpl_->GetAnalogStickData(stickID);
  }
  
  
  /*!
   * \brief Obtains the mode of the analog stick with id stickID
   * \param stickID the id associated with the analog stick to query
   * \return the mode the stick is in
   */
  tHWAnalogStickMode HWAnalogStickMPI::GetAnalogStickMode(const LeapFrog::Brio::U8 stickID) const 	{
    return pimpl_->GetAnalogStickMode(stickID);
  }
  
  /*!
   * \brief set the analog stick mode for the stick with id of stickID
   * \param stickID the id associated with the analog stick
   * \param mode the desired mode to set the analog stick with id stickID
   */
  LeapFrog::Brio::tErrType 
  HWAnalogStickMPI::SetAnalogStickMode(tHWAnalogStickMode mode,
				       LeapFrog::Brio::U8 stickID) {
    return pimpl_->SetAnalogStickMode(mode, stickID);
  }
  
  
  /*!
   * \brief returns the dead zone parameter for the analog stick with id stickID
   * \param stickID the id of the stick to query
   * \return the value for the dead zone of the stick with id stickID
   */
  float 
  HWAnalogStickMPI::GetAnalogStickDeadZone(const LeapFrog::Brio::U8 stickID) const {
    return pimpl_->GetAnalogStickDeadZone(stickID);
  }
  
  /*!
   * \brief Set the dead zone
   * The deadzone is the area at which the sticks do not report any events. 
   * The default is zero, which gives the best sensitifity but might also 
   * cause trouble in some games in that the character or camera might move 
   * without moving the stick. To fix this one has to set the value to 
   * something higher.  A normalized value between 0 .. 127
   * \param deadZone the desired dead zone value
   * \param stickID the id of the stick to set the dead zone
   * \return Returns kNoErr on success.
   */
  LeapFrog::Brio::tErrType 
  HWAnalogStickMPI::SetAnalogStickDeadZone(const float deadZone,
					   const LeapFrog::Brio::U8 stickID) {
    return pimpl_->SetAnalogStickDeadZone(deadZone, stickID);
  }
  
} // namespace Hardware
} // namespace LF
