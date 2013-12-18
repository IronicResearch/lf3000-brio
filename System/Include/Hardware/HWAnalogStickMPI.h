#ifndef __INCLUDE_HARDWARE_HWANALOGSTICKMPI_H__
#define __INCLUDE_HARDWARE_HWANALOGSTICKMPI_H__

#include <CoreMPI.h>
#include <Hardware/HWAnalogStickTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Hardware {

  class HWAnalogStickPIMPL;

  /*!    
   * \class HWAnalogStickMPI
   *
   * HWAnalogStickMPI class to support posting HWAnalogStickMessage events
   * to registered listeners of kGroupAnalogStick type. Options to get/set
   * dead zone and modes.
   */
  
  class HWAnalogStickMPI : public LeapFrog::Brio::ICoreMPI {
  public:	
    /*!
     * \brief Constructor
     */
    HWAnalogStickMPI(void);
    
    /*!
     * \brief Destructor
     */
    virtual ~HWAnalogStickMPI(void);
    
    /*!
     * \defgroup Virtual Base Class Methods
     * \brief These five methods are declared as virtual in the base class, ICoreMPI
     */
    virtual LeapFrog::Brio::Boolean        IsValid(void) const;
    virtual const LeapFrog::Brio::CString* GetMPIName(void) const;
    virtual LeapFrog::Brio::tVersion       GetModuleVersion(void) const;
    virtual const LeapFrog::Brio::CString* GetModuleName(void) const;
    virtual const LeapFrog::Brio::CURI*    GetModuleOrigin(void) const;
    
    
    /*!
     * \brief Register an event listener (via EventMPI) and enable continuous sampleing mode
     * \param listener The event listener HWAnalogStickMessage events will be sent to
     */
    LeapFrog::Brio::tErrType RegisterEventListener(const LeapFrog::Brio::IEventListener *listener);
    
    /*!
     * \brief Unregister event listener (via EventMPI) and siable all sampling
     * \param listener The event listener to unregister
     */
    LeapFrog::Brio::tErrType UnregisterEventListener(const LeapFrog::Brio::IEventListener *listener);
    
    /*!
     * \breif A method to check for the presence of an analog stick
     * \return true if an analog stick device is present
     */
    LeapFrog::Brio::Boolean IsAnalogStickPresent(void) const;
    
    /*!
     * \brief Obtains the data for the current analog stick
     * \param stickID the id associated with the analog stick to query
     * \return The data associated with the analog stick of id stickID
     */
    tHWAnalogStickData GetAnalogStickData(const LeapFrog::Brio::U8 stickID = kHWDefaultAnalogStickID) const;
    
    
    /*!
     * \brief Obtains the mode of the analog stick with id stickID
     * \param stickID the id associated with the analog stick to query
     * \return the mode the stick is in
     */
    tHWAnalogStickMode GetAnalogStickMode(const LeapFrog::Brio::U8 stickID = kHWDefaultAnalogStickID) const;
    
    /*!
     * \brief set the analog stick mode for the stick with id of stickID
     * \param stickID the id associated with the analog stick
     * \param mode the desired mode to set the analog stick with id stickID
     */
    LeapFrog::Brio::tErrType SetAnalogStickMode(tHWAnalogStickMode mode,
						LeapFrog::Brio::U8 stickID = kHWDefaultAnalogStickID);
    
    
    /*!
     * \brief returns the dead zone parameter for the analog stick with id stickID
     * \param stickID the id of the stick to query
     * \return the value for the dead zone of the stick with id stickID
     */
    float GetAnalogStickDeadZone(const LeapFrog::Brio::U8 stickID = kHWDefaultAnalogStickID) const;
    
    /*!
     * \brief Set the dead zone
     * The deadzone is the area at which the sticks do not report any events. 
     * The default is zero, which gives the best sensitifity but might also 
     * cause trouble in some games in that the character or camera might move 
     * without moving the stick. To fix this one has to set the value to 
     * something higher.  A normalized value between 0 .. 1
     * \param deadZone the desired dead zone value
     * \param stickID the id of the stick to set the dead zone
     * \return Returns kNoErr on success.
     */
    LeapFrog::Brio::tErrType SetAnalogStickDeadZone(const float deadZone,
						    const LeapFrog::Brio::U8 stickID = kHWDefaultAnalogStickID);
    
  private:
    boost::shared_ptr<HWAnalogStickPIMPL> pimpl_;
    
    /*!
     * Explicitly disable copy semantics
     */
    HWAnalogStickMPI(const HWAnalogStickMPI&);
    HWAnalogStickMPI& operator =(const HWAnalogStickMPI&);
   
  };

} // namespace Hardware
} // namespace LF

#endif // __INCLUDE_HARDWARE_HWANALOGSTICKMPI_H__
