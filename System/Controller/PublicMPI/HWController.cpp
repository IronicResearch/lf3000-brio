#include <Hardware/HWController.h>
#include <EventMPI.h>
#include "HWControllerPIMPL.h"
#include "HWControllerBluetoothPIMPL.h"
#include <dlfcn.h>
#include <sys/stat.h>

namespace LF {
namespace Hardware {

  
  HWController::HWController(void)  {
	  struct stat stbuf;
	  if (0 == stat("/LF/Base/Brio/lib/libBluetopiaIO.so", &stbuf)) {
		  pimpl_ = boost::shared_ptr<HWControllerBluetoothPIMPL>(new HWControllerBluetoothPIMPL(this));
	  }
	  else {
		  pimpl_ = boost::shared_ptr<HWControllerPIMPL>(new HWControllerPIMPL(this));
	  }
  }
  
  HWController::~HWController(void) {
  }
 
  LeapFrog::Brio::U8 
  HWController::GetID(void) const {
    return pimpl_->GetID();
  }
    
  HWControllerMode 
  HWController::GetCurrentMode(void) const {
    return pimpl_->GetCurrentMode();
  }

  bool 
  HWController::IsConnected(void) const {
    return pimpl_->IsConnected();
  }

  LeapFrog::Brio::U32 
  HWController::GetControllerUpdateRate(void) const {
    return pimpl_->GetControllerUpdateRate();
  }

  LeapFrog::Brio::tErrType 
  HWController::SetControllerUpdateRate(LeapFrog::Brio::U32 rate) {
    return pimpl_->SetControllerUpdateRate(rate);
  }

  HWControllerFunctionalityMask 
  HWController::GetFunctionality(void) const {
    return pimpl_->GetFunctionality();
  }

  HWControllerLEDColorMask 
  HWController::GetAvailableLEDColors(void) const {
    return pimpl_->GetAvailableLEDColors();
  }

  HWControllerLEDColor 
  HWController::GetLEDColor(void) const {
    return pimpl_->GetLEDColor();
  }

  void 
  HWController::SetLEDColor(HWControllerLEDColor color) {
    pimpl_->SetLEDColor(color);
  }

  Vision::VNPoint 
  HWController::GetLocation(void) const {
    return pimpl_->GetLocation();
  }

  bool 
  HWController::IsVisible(void) const {
    return pimpl_->IsVisible();
  }
    
  LeapFrog::Brio::tButtonData2 
  HWController::GetButtonData(void) const {
    return pimpl_->GetButtonData();
  }

  HWControllerButtonMask 
  HWController::GetAvailableButtons(void) const {
    return pimpl_->GetAvailableButtons();
  }

  tHWAnalogStickData 
  HWController::GetAnalogStickData(void) const {
    return pimpl_->GetAnalogStickData();
  }

  tHWAnalogStickMode 
  HWController::GetAnalogStickMode(void) const {
    return pimpl_->GetAnalogStickMode();
  }

  LeapFrog::Brio::tErrType 
  HWController::SetAnalogStickMode(tHWAnalogStickMode mode) {
    return pimpl_->SetAnalogStickMode(mode);
  }

  float 
  HWController::GetAnalogStickDeadZone(void) const {
    return pimpl_->GetAnalogStickDeadZone();
  }

  LeapFrog::Brio::tErrType 
  HWController::SetAnalogStickDeadZone(const float deadZone) {
    return pimpl_->SetAnalogStickDeadZone(deadZone);
  }
    
  LeapFrog::Brio::tAccelerometerData 
  HWController::GetAccelerometerData(void) const {
    return pimpl_->GetAccelerometerData();
  }

  LeapFrog::Brio::tAccelerometerMode 
  HWController::GetAccelerometerMode(void) const {
    return pimpl_->GetAccelerometerMode();
  }

  LeapFrog::Brio::tErrType 
  HWController::SetAccelerometerMode(const LeapFrog::Brio::tAccelerometerMode mode) {
    return pimpl_->SetAccelerometerMode(mode);
  }
  
} // namespace Hardware
} // namespace LF
