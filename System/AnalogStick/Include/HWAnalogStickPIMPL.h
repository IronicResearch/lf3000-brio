#ifndef __INCLUDE_HARDWARE_HWAnalogStickPIMPLEMUL_H__
#define __INCLUDE_HARDWARE_HWAnalogStickPIMPLEMUL_H__

#include <Hardware/HWAnalogStickTypes.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Hardware {


  /*!    
   * \class HWAnalogStickPIMPL
   *
   * HWAnalogStickPIMPL x86 linux emulation implementation of analog stick
   */
  class HWAnalogStickPIMPL {
  public:
    static boost::shared_ptr<HWAnalogStickPIMPL> Instance(void);
    virtual ~HWAnalogStickPIMPL(void);
    
    
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
     * \brief Obtains the data for the current analog stick
     * \param stickID the id associated with the analog stick to query
     * \return The data associated with the analog stick of id stickID
     */
    tHWAnalogStickData SetAnalogStickData(const tHWAnalogStickData& data);
    
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
     * something higher.  A normalized value between 0 .. 127
     * \param deadZone the desired dead zone value
     * \param stickID the id of the stick to set the dead zone
     * \return Returns kNoErr on success.
     */
    LeapFrog::Brio::tErrType SetAnalogStickDeadZone(const float deadZone,
						    const LeapFrog::Brio::U8 stickID = kHWDefaultAnalogStickID);
    
    
    
  private:
    HWAnalogStickPIMPL(void);
    HWAnalogStickPIMPL(const HWAnalogStickPIMPL&);
    HWAnalogStickPIMPL& operator=(const HWAnalogStickPIMPL&);
    
    static boost::shared_ptr<HWAnalogStickPIMPL> forceHWAnalogStickMPIMPLToBe_;

    tHWAnalogStickMode mode_;
    tHWAnalogStickData data_;
    float              deadZone_;   
  };
  
}	// namespace Hardware
}	// namespace LF

#endif // __INCLUDE_HARDWARE_HWAnalogStickPIMPLEMUL_H__
