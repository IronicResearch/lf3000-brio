#include "HWControllerBluetoothPIMPL.h"
#include <Hardware/HWControllerTypes.h>
#include <Vision/VNWand.h>
#include <BluetopiaIO.h>
#include <string.h>
#include <iostream> //AJL Debug
#include <dlfcn.h>

using namespace LeapFrog::Brio;

namespace LF {
namespace Hardware {

  HWControllerBluetoothPIMPL::HWControllerBluetoothPIMPL(void) :
    id_(kHWDefaultControllerID),
    mode_(kHWControllerMode) { //,
    //    updateRate_(accelerometerMPI_.GetAccelerometerRate()) {
    std::cout << "AJL: Inside HWControllerBluetoothPIMPL constructor\n";
    ZeroAccelerometerData();
    ZeroButtonData();
    ZeroAnalogStickData();

    // Dynamically load Bluetooth client lib
	dll_ = dlopen("libBluetopiaIO.so", RTLD_LAZY);
	if (dll_ != NULL) {
		pBTIO_Init_			= (pFnInit)dlsym(dll_, "BTIO_Init");
		pBTIO_Exit_			= (pFnExit)dlsym(dll_, "BTIO_Exit");
		pBTIO_SendCommand_ 	= (pFnSendCommand)dlsym(dll_, "BTIO_SendCommand");
		pBTIO_QueryStatus_	= (pFnQueryStatus)dlsym(dll_, "BTIO_QueryStatus");

		// Connect to Bluetooth client service?
		handle_ = BTIO_Init(NULL);
	}
	else {
		std::cout << "dlopen failed to load libBluetopiaIO.so, error=\n" << dlerror();
	}
  }

  HWControllerBluetoothPIMPL::~HWControllerBluetoothPIMPL(void) {
	  // Close Bluetooth client lib connection
	  if (dll_) {
		  BTIO_Exit(handle_);
		  dlclose(dll_);
	  }
  }
  
  void
  HWControllerBluetoothPIMPL::ZeroAccelerometerData(void) {
    accelerometerData_.accelX = 0;
    accelerometerData_.accelY = 0;
    accelerometerData_.accelZ = 0;
    accelerometerData_.time.seconds = 0;
    accelerometerData_.time.microSeconds = 0;
  }

  void
  HWControllerBluetoothPIMPL::ZeroButtonData(void) {
    buttonData_.buttonState = 0;
    buttonData_.buttonTransition = 0;
    buttonData_.time.seconds = 0;
    buttonData_.time.microSeconds = 0;
  }

  void
  HWControllerBluetoothPIMPL::ZeroAnalogStickData(void) {
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
  HWControllerBluetoothPIMPL::GetID(void) const {
    return id_;
  }
  
  HWControllerMode 
  HWControllerBluetoothPIMPL::GetCurrentMode(void) const {
    return mode_;
  }
  
  bool 
  HWControllerBluetoothPIMPL::IsConnected(void) const {
    //TODO:
    return dll_ != NULL;
  }
  
  LeapFrog::Brio::U32 
  HWControllerBluetoothPIMPL::GetControllerUpdateRate(void) const {
    return updateRate_;
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerBluetoothPIMPL::SetControllerUpdateRate(LeapFrog::Brio::U32 rate) {
    //TODO: error checking
    if (dll_)
    	BTIO_SendCommand(handle_, kBTIOCmdSetUpdateRate, &rate, sizeof(rate));
    updateRate_ = rate;
    return kNoErr;
  }
  
  HWControllerFunctionalityMask 
  HWControllerBluetoothPIMPL::GetFunctionality(void) const {
    // TOOD: prgrammatically check
	if (dll_) {
		U32 caps = 0;
		BTIO_QueryStatus(handle_, kBTIOCmdGetControllerCaps, &caps, sizeof(caps));
		return (HWControllerFunctionalityMask)caps;
	}
    return (kHWControllerHasAccelerometer & 
	    kHWControllerHasAnalogStick &
	    kHWControllerHasButtons &
	    kHWControllerHasLED);
  }
  
  /*!
   * Color LED Tip Methods
   */
  HWControllerLEDColorMask 
  HWControllerBluetoothPIMPL::GetAvailableLEDColors(void) const {
    //TODO: programmatically check
	if (dll_) {
		U32 caps = 0;
		BTIO_QueryStatus(handle_, kBTIOCmdGetLEDCaps, &caps, sizeof(caps));
		return (HWControllerLEDColorMask)caps;
	}
    return (kHWControllerLEDGreen    &
	    kHWControllerLEDRed      &
	    kHWControllerLEDBlue     &
	    kHWControllerLEDOrange   &
	    kHWControllerLEDTurqoise &
	    kHWControllerLEDPurple   &
	    kHWControllerLEDWhite);
  }
  
  HWControllerLEDColor 
  HWControllerBluetoothPIMPL::GetLEDColor(void) const {
    //TODO: programmatically get color
	if (dll_) {
		U32 state = 0;
		BTIO_QueryStatus(handle_, kBTIOCmdGetLEDState, &state, sizeof(state));
		return (HWControllerLEDColor)state;
	}
    // for now just one color
    return kHWControllerLEDGreen;
  }
  
  void 
  HWControllerBluetoothPIMPL::SetLEDColor(HWControllerLEDColor color) {
	if (dll_)
		BTIO_SendCommand(handle_, kBTIOCmdSetLEDState, &color, sizeof(color));
    //TODO: for now we are only using one color
  }
  
  Vision::VNPoint 
  HWControllerBluetoothPIMPL::GetLocation(void) const {
    visionMPI_.GetWandByID(id_)->GetLocation();
  }
  
  bool 
  HWControllerBluetoothPIMPL::IsVisible(void) const {
    visionMPI_.GetWandByID(id_)->IsVisible();
  }
  
  /*!
   * Button Related Methods
   */
  LeapFrog::Brio::tButtonData2 
  HWControllerBluetoothPIMPL::GetButtonData(void) const {
    //TODO: determine how to get button data for a specific controller
	if (dll_) {
		BTIO_QueryStatus(handle_, kBTIOCmdGetButtonData, (void*)&buttonData_, sizeof(buttonData_));
		return buttonData_;
	}
    return buttonData_;
  }
  
  void
  HWControllerBluetoothPIMPL::SetButtonData(const LeapFrog::Brio::tButtonData2 &data) {
    memcpy(&buttonData_, &data, sizeof(data));
  }

  HWControllerButtonMask 
  HWControllerBluetoothPIMPL::GetAvailableButtons(void) const {
    //TODO: figure out which buttons are available
	if (dll_) {
		U32 caps = 0;
		BTIO_QueryStatus(handle_, kBTIOCmdGetButtonCaps, &caps, sizeof(caps));
		return (HWControllerButtonMask)caps;
	}
    return 0;
  }
  
  /*!
   * AnalogStick Related Methods
   */
  tHWAnalogStickData 
  HWControllerBluetoothPIMPL::GetAnalogStickData(void) const {
	if (dll_) {
		BTIO_QueryStatus(handle_, kBTIOCmdGetAnalogStickData, (void*)&analogStickData_, sizeof(analogStickData_));
		return analogStickData_;
	}
    return analogStickData_;
  }
  
  void
  HWControllerBluetoothPIMPL::SetAnalogStickData(const tHWAnalogStickData &data) {
    memcpy(&analogStickData_, &data, sizeof(data));
  }

  tHWAnalogStickMode 
  HWControllerBluetoothPIMPL::GetAnalogStickMode(void) const {
	if (dll_) {
		U32 mode = 0;
		BTIO_QueryStatus(handle_, kBTIOCmdGetAnalogStickMode, &mode, sizeof(mode));
		return (tHWAnalogStickMode)mode;
	}
    return analogStickMPI_.GetAnalogStickMode(id_);
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerBluetoothPIMPL::SetAnalogStickMode(tHWAnalogStickMode mode) {
	if (dll_) {
		return BTIO_SendCommand(handle_, kBTIOCmdSetAnalogStickMode, &mode, sizeof(mode));
	}
    return analogStickMPI_.SetAnalogStickMode(mode, id_);
  }
  
  float 
  HWControllerBluetoothPIMPL::GetAnalogStickDeadZone(void) const {
	if (dll_) {
		U32 deadzone = 0;
		BTIO_QueryStatus(handle_, kBTIOCmdGetAnalogStickDeadZone, &deadzone, sizeof(deadzone));
		return (float)deadzone; // FIXME
	}
    return analogStickMPI_.GetAnalogStickDeadZone(id_);
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerBluetoothPIMPL::SetAnalogStickDeadZone(const float deadZone) {
	if (dll_) {
		U32 deadzone = (U32)deadZone; // FIXME
		return BTIO_SendCommand(handle_, kBTIOCmdSetAnalogStickDeadZone, (void*)&deadzone, sizeof(deadzone));
	}
    return analogStickMPI_.SetAnalogStickDeadZone(deadZone, id_);
  }
  
  /*!
   * Accelerometer Related Methods
   */
  LeapFrog::Brio::tAccelerometerData 
  HWControllerBluetoothPIMPL::GetAccelerometerData(void) const {
    //TODO: figure out how to do this on a per controller basis
	if (dll_) {
		BTIO_QueryStatus(handle_, kBTIOCmdGetAccelerometerData, (void*)&accelerometerData_, sizeof(accelerometerData_));
		return accelerometerData_;
	}
    return accelerometerData_;
  }
  
  void
  HWControllerBluetoothPIMPL::SetAccelerometerData(const LeapFrog::Brio::tAccelerometerData &data) {
    memcpy(&accelerometerData_, &data, sizeof(data));
  }

  LeapFrog::Brio::tAccelerometerMode 
  HWControllerBluetoothPIMPL::GetAccelerometerMode(void) const {
    //TODO: figure out how to do this on a per controller basis
	if (dll_) {
		U32 mode = 0;
		BTIO_QueryStatus(handle_, kBTIOCmdGetAccelerometerMode, &mode, sizeof(mode));
		return (LeapFrog::Brio::tAccelerometerMode)mode;
	}
    accelerometerMPI_.GetAccelerometerMode();
  }
  
  LeapFrog::Brio::tErrType 
  HWControllerBluetoothPIMPL::SetAccelerometerMode(const LeapFrog::Brio::tAccelerometerMode mode) {
    //TODO: figure out to do this on a per controller basis
	if (dll_) {
		return BTIO_SendCommand(handle_, kBTIOCmdSetAccelerometerMode, (void*)&mode, sizeof(mode));
	}
    return accelerometerMPI_.SetAccelerometerMode(mode);
  }
  
}	// namespace Hardware
}	// namespace LF
