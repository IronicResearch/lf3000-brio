#include "HWControllerPIMPL.h"
#include <Hardware/HWControllerTypes.h>
#include <Vision/VNWand.h>
#include <string.h>
#include <iostream> //AJL Debug

using namespace LeapFrog::Brio;

namespace LF {
namespace Hardware {

  HWControllerPIMPL::HWControllerPIMPL(void) :
    id_(kHWDefaultControllerID),
    mode_(kHWControllerMode) { //,
    //    updateRate_(accelerometerMPI_.GetAccelerometerRate()) {
    std::cout << "AJL: Inside HWControllerPIMPL constructor\n";
    ZeroAccelerometerData();
    ZeroButtonData();
    ZeroAnalogStickData();
  }

  HWControllerPIMPL::~HWControllerPIMPL(void) {

  }
  
  void
  HWControllerPIMPL::ZeroAccelerometerData(void) {
    accelerometerData_.accelX = 0;
    accelerometerData_.accelY = 0;
    accelerometerData_.accelZ = 0;
    accelerometerData_.time.seconds = 0;
    accelerometerData_.time.microSeconds = 0;
  }

  void
  HWControllerPIMPL::ZeroButtonData(void) {
    buttonData_.buttonState = 0;
    buttonData_.buttonTransition = 0;
    buttonData_.time.seconds = 0;
    buttonData_.time.microSeconds = 0;
  }

  void
  HWControllerPIMPL::ZeroAnalogStickData(void) {
    analogStickData_.x = 0.f;
    analogStickData_.y = 0.f;
    analogStickData_.id = id_;
    analogStickData_.time.seconds = id_;
    analogStickData_.time.microSeconds = id_;    
  }

  /*!
   * Broad Based Controller Methods
   */
  LeapFrog::Brio::U8 
  HWControllerPIMPL::GetID(void) const {
    return id_;
  }
  
  HWControllerMode 
  HWControllerPIMPL::GetCurrentMode(void) const {
    return mode_;
  }
  
  bool 
  HWControllerPIMPL::IsConnected(void) const {
    //TODO:
    return true;
  }
  
  LeapFrog::Brio::U32 
  HWControllerPIMPL::GetControllerUpdateRate(void) const {
    return updateRate_;
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerPIMPL::SetControllerUpdateRate(LeapFrog::Brio::U32 rate) {
    //TODO: error checking
    updateRate_ = rate;

    return kNoErr;
  }
  
  HWControllerFunctionalityMask 
  HWControllerPIMPL::GetFunctionality(void) const {
    // TOOD: prgrammatically check
    return (kHWControllerHasAccelerometer & 
	    kHWControllerHasAnalogStick &
	    kHWControllerHasButtons &
	    kHWControllerHasLED);
  }
  
  /*!
   * Color LED Tip Methods
   */
  HWControllerLEDColorMask 
  HWControllerPIMPL::GetAvailableLEDColors(void) const {
    //TODO: programmatically check
    return (kHWControllerLEDGreen    &
	    kHWControllerLEDRed      &
	    kHWControllerLEDBlue     &
	    kHWControllerLEDOrange   &
	    kHWControllerLEDTurqoise &
	    kHWControllerLEDPurple   &
	    kHWControllerLEDWhite);
  }
  
  HWControllerLEDColor 
  HWControllerPIMPL::GetLEDColor(void) const {
    //TODO: programmatically get color
    // for now just one color
    return kHWControllerLEDGreen;
  }
  
  void 
  HWControllerPIMPL::SetLEDColor(HWControllerLEDColor color) {
    //TODO: for now we are only using one color
  }
  
  Vision::VNPoint 
  HWControllerPIMPL::GetLocation(void) const {
    visionMPI_.GetWandByID(id_)->GetLocation();
  }
  
  bool 
  HWControllerPIMPL::IsVisible(void) const {
    visionMPI_.GetWandByID(id_)->IsVisible();
  }
  
  /*!
   * Button Related Methods
   */
  LeapFrog::Brio::tButtonData2 
  HWControllerPIMPL::GetButtonData(void) const {
    //TODO: determine how to get button data for a specific controller
    return buttonData_;
  }
  
  void
  HWControllerPIMPL::SetButtonData(const LeapFrog::Brio::tButtonData2 &data) {
    memcpy(&buttonData_, &data, sizeof(data));
  }

  HWControllerButtonMask 
  HWControllerPIMPL::GetAvailableButtons(void) const {
    //TODO: figure out which buttons are available
    return 0;
  }
  
  /*!
   * AnalogStick Related Methods
   */
  tHWAnalogStickData 
  HWControllerPIMPL::GetAnalogStickData(void) const {
    return analogStickData_;
  }
  
  void
  HWControllerPIMPL::SetAnalogStickData(const tHWAnalogStickData &data) {
    memcpy(&analogStickData_, &data, sizeof(data));
  }

  tHWAnalogStickMode 
  HWControllerPIMPL::GetAnalogStickMode(void) const {
    return analogStickMPI_.GetAnalogStickMode(id_);
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerPIMPL::SetAnalogStickMode(tHWAnalogStickMode mode) {
    return analogStickMPI_.SetAnalogStickMode(mode, id_);
  }
  
  float 
  HWControllerPIMPL::GetAnalogStickDeadZone(void) const {
    return analogStickMPI_.GetAnalogStickDeadZone(id_);
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerPIMPL::SetAnalogStickDeadZone(const float deadZone) {
    return analogStickMPI_.SetAnalogStickDeadZone(deadZone, id_);
  }
  
  /*!
   * Accelerometer Related Methods
   */
  LeapFrog::Brio::tAccelerometerData 
  HWControllerPIMPL::GetAccelerometerData(void) const {
    //TODO: figure out how to do this on a per controller basis
    return accelerometerData_;
  }
  
  void
  HWControllerPIMPL::SetAccelerometerData(const LeapFrog::Brio::tAccelerometerData &data) {
    memcpy(&accelerometerData_, &data, sizeof(data));
  }

  LeapFrog::Brio::tAccelerometerMode 
  HWControllerPIMPL::GetAccelerometerMode(void) const {
    //TODO: figure out how to do this on a per controller basis
    accelerometerMPI_.GetAccelerometerMode();
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerPIMPL::SetAccelerometerMode(const LeapFrog::Brio::tAccelerometerMode mode) {
    //TODO: figure out to do this on a per controller basis
    accelerometerMPI_.SetAccelerometerMode(mode);
  }
  
}	// namespace Hardware
}	// namespace LF
