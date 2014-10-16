#include <Hardware/HWController.h>
#include <EventMPI.h>
#include "HWControllerPIMPL.h"
#include <dlfcn.h>
#include <sys/stat.h>
#include <Utility.h>
#include <string.h>

namespace LF {
namespace Hardware {


  HWController::HWController(void)  {
	  if(HasPlatformCapability(kCapsGamePadController)) {
		  pimpl_ = boost::shared_ptr<HWControllerPIMPL>(new HWControllerPIMPL(this));
	  } else {
		  LeapFrog::Brio::CDebugMPI localDebugMPI(kGroupController);
		  localDebugMPI.DebugOut(kDbgLvlImportant, "HWController::HWController() called on a platform which does not support hand-held controllers\n");
		  pimpl_ = boost::shared_ptr<HWControllerPIMPL>((HWControllerPIMPL*)(NULL));;
	  }
  }

  HWController::~HWController(void) {
  }

  LeapFrog::Brio::U8
  HWController::GetHwVersion(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetHwVersion();
	  else
		  return 0;
  }

  LeapFrog::Brio::U8
  HWController::GetFwVersion(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetFwVersion();
	  else
		  return 0;
  }

  LeapFrog::Brio::U8
  HWController::GetID(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetID();
	  else
		  return 0;
  }

  HWControllerMode
  HWController::GetCurrentMode(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetCurrentMode();
	  else
		  return kHWControllerNoMode;
  }

  bool
  HWController::IsConnected(void) const {
	  if(pimpl_.get())
		  return pimpl_->IsConnected();
	  else
		  return false;
  }

  LeapFrog::Brio::U32
  HWController::GetControllerUpdateRate(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetControllerUpdateRate();
	  else
		  return 0;
  }

  LeapFrog::Brio::tErrType
  HWController::SetControllerUpdateRate(LeapFrog::Brio::U32 rate) {
	  if(pimpl_.get())
		  return pimpl_->SetControllerUpdateRate(rate);
	  else
		  return LeapFrog::Brio::kNoImplErr;
  }

  HWControllerFunctionalityMask
  HWController::GetFunctionality(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetFunctionality();
	  else
		  return 0;
  }

  HWControllerLEDColorMask
  HWController::GetAvailableLEDColors(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetAvailableLEDColors();
	  else
		  return 0;
  }

  HWControllerLEDColor
  HWController::GetLEDColor(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetLEDColor();
	  else
		  return LF::Hardware::kHWControllerLEDOff;
  }

  void
  HWController::SetLEDColor(HWControllerLEDColor color) {
    if(pimpl_.get()) pimpl_->SetLEDColor(color);
  }

  Vision::VNPoint
  HWController::GetLocation(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetLocation();
	  else
		  return Vision::VNPoint(0,0);
  }

  bool
  HWController::IsVisible(void) const {
	  if(pimpl_.get())
		  return pimpl_->IsVisible();
	  else
		  return false;
  }

  LeapFrog::Brio::tErrType
  HWController::StartTracking(HWControllerLEDColor color) {
	  if(pimpl_.get())
		  return pimpl_->StartTracking(color);
	  else
		  return LeapFrog::Brio::kNoImplErr;
  }

  LeapFrog::Brio::tButtonData2
  HWController::GetButtonData(void) const {
	  if(pimpl_.get()) {
		  return pimpl_->GetButtonData();
	  } else {
		  tButtonData2 fakeButton;
		  fakeButton.buttonState = 0;
		  fakeButton.buttonTransition = 0;
		  fakeButton.time.seconds = 0;
		  fakeButton.time.microSeconds = 0;
		  return fakeButton;
	  }
  }

  HWControllerButtonMask
  HWController::GetAvailableButtons(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetAvailableButtons();
	  else
		  return 0;
  }

  tHWAnalogStickData
  HWController::GetAnalogStickData(void) const {
	  if(pimpl_.get()) {
		  return pimpl_->GetAnalogStickData();
	  } else {
		  tHWAnalogStickData fakeStick;
		  fakeStick.x = 0.0f;
		  fakeStick.y = 0.0f;
		  fakeStick.id = 0;
		  fakeStick.time.seconds = 0;
		  fakeStick.time.microSeconds = 0;
		  return fakeStick;
	  }
  }

  tHWAnalogStickMode
  HWController::GetAnalogStickMode(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetAnalogStickMode();
	  else
		  return kHWAnalogStickModeDisable;
  }

  LeapFrog::Brio::tErrType
  HWController::SetAnalogStickMode(tHWAnalogStickMode mode) {
	  if(pimpl_.get())
		  return pimpl_->SetAnalogStickMode(mode);
	  else
		  return LeapFrog::Brio::kNoImplErr;
  }

  float
  HWController::GetAnalogStickDeadZone(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetAnalogStickDeadZone();
	  else
		  return 0.0f;
  }

  LeapFrog::Brio::tErrType
  HWController::SetAnalogStickDeadZone(const float deadZone) {
	  if(pimpl_.get())
		  return pimpl_->SetAnalogStickDeadZone(deadZone);
	  else
		  return LeapFrog::Brio::kNoImplErr;
  }

  LeapFrog::Brio::tAccelerometerData
  HWController::GetAccelerometerData(void) const {
	  if(pimpl_.get()) {
		  return pimpl_->GetAccelerometerData();
	  } else {
		  tAccelerometerData fakeAccel;
		  fakeAccel.accelX = 0;
		  fakeAccel.accelY = 0;
		  fakeAccel.accelZ = 0;
		  fakeAccel.time.seconds = 0;
		  fakeAccel.time.microSeconds = 0;
		  return fakeAccel;
	  }
  }

  LeapFrog::Brio::tAccelerometerMode
  HWController::GetAccelerometerMode(void) const {
	  if(pimpl_.get())
		  return pimpl_->GetAccelerometerMode();
	  else
		  return kAccelerometerModeDisabled;
  }

  LeapFrog::Brio::tErrType
  HWController::SetAccelerometerMode(const LeapFrog::Brio::tAccelerometerMode mode) {
	  if(pimpl_.get())
		  return pimpl_->SetAccelerometerMode(mode);
	  else
		  return LeapFrog::Brio::kNoImplErr;
  }

  const char*
  HWController::GetBluetoothAddress() {
	  static const int btLength = 64;
	  static char fakeBlueToothAddress[btLength];		//This variable is setup to mirror HWControllerPIMPL::blueToothAddress

	  if(pimpl_.get()) {
		  return pimpl_->GetBluetoothAddress();
	  } else {
		  memset(fakeBlueToothAddress, 0, btLength);
		  return fakeBlueToothAddress;
	  }
  }
} // namespace Hardware
} // namespace LF
