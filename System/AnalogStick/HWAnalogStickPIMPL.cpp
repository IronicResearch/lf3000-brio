#include "HWAnalogStickPIMPL.h"

namespace LF {
namespace Hardware {

  LeapFrog::Brio::tMutex HWAnalogStickPIMPL::instanceMutex_ = PTHREAD_MUTEX_INITIALIZER;

  boost::shared_ptr<HWAnalogStickPIMPL>
  HWAnalogStickPIMPL::Instance(void) {
    static boost::shared_ptr<HWAnalogStickPIMPL> sharedInstance;
    if (sharedInstance == NULL) {
      LeapFrog::Brio::CKernelMPI kernelMPI;
      kernelMPI.LockMutex(HWAnalogStickPIMPL::instanceMutex_);
      if (sharedInstance == NULL) {
	sharedInstance.reset(new HWAnalogStickPIMPL());
      }
      kernelMPI.UnlockMutex(HWAnalogStickPIMPL::instanceMutex_);
    }
    return sharedInstance;
  }
  
  HWAnalogStickPIMPL::HWAnalogStickPIMPL(void) 
    :	deadZone_(0.14f) //Specified by EE team at +/-7%, which is -0.14..+0.14 in -1.0..+1.0 scale.  FWGLAS-779
    ,	mode_(kHWAnalogStickModeAnalog)
  {
  }
  
  HWAnalogStickPIMPL::~HWAnalogStickPIMPL() {
  }
  
  HWAnalogStickPIMPL::HWAnalogStickPIMPL(const HWAnalogStickPIMPL&) {
  }
  
  HWAnalogStickPIMPL& HWAnalogStickPIMPL::operator=(const HWAnalogStickPIMPL&) {
  }
  
  /*!
   * \breif A method to check for the presence of an analog stick
   * \return true if an analog stick device is present
   */
  LeapFrog::Brio::Boolean HWAnalogStickPIMPL::IsAnalogStickPresent(void) const {
    return true;	//TODO:
  }
  
  /*!
   * \brief Obtains the data for the current analog stick
   * \param stickID the id associated with the analog stick to query
   * \return The data associated with the analog stick of id stickID
   */
  tHWAnalogStickData HWAnalogStickPIMPL::GetAnalogStickData(const LeapFrog::Brio::U8 stickID) const {
    return data_;
  }
  
  
  /*!
   * \brief Obtains the data for the current analog stick
   * \param stickID the id associated with the analog stick to query
   * \return The data associated with the analog stick of id stickID
   */
  tHWAnalogStickData HWAnalogStickPIMPL::SetAnalogStickData(const tHWAnalogStickData& data) {
    data_ = data;
  }
  
  /*!
   * \brief Obtains the mode of the analog stick with id stickID
   * \param stickID the id associated with the analog stick to query
   * \return the mode the stick is in
   */
  tHWAnalogStickMode HWAnalogStickPIMPL::GetAnalogStickMode(const LeapFrog::Brio::U8 stickID) const {
    return mode_;
  }
  
  /*!
   * \brief set the analog stick mode for the stick with id of stickID
   * \param stickID the id associated with the analog stick
   * \param mode the desired mode to set the analog stick with id stickID
   */
  LeapFrog::Brio::tErrType HWAnalogStickPIMPL::SetAnalogStickMode(tHWAnalogStickMode mode,
								  LeapFrog::Brio::U8 stickID) {
    mode_ = mode;
    
    return kNoErr;					
  }
  
  
  /*!
   * \brief returns the dead zone parameter for the analog stick with id stickID
   * \param stickID the id of the stick to query
   * \return the value for the dead zone of the stick with id stickID
   */
  float HWAnalogStickPIMPL::GetAnalogStickDeadZone(const LeapFrog::Brio::U8 stickID) const {
    return deadZone_;
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
  LeapFrog::Brio::tErrType HWAnalogStickPIMPL::SetAnalogStickDeadZone(const float deadZone,
								      const LeapFrog::Brio::U8 stickID) {
    deadZone_ = deadZone;
    
    return kNoErr;						
  }
  
}	// namespace Hardware
}	// namespace LF
