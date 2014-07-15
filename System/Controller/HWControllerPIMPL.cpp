#include "HWControllerPIMPL.h"
#include "HWControllerMPIPIMPL.h"
#include <Hardware/HWControllerTypes.h>
#include <Hardware/HWControllerEventMessage.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <VNVisionMPIPIMPL.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <math.h>

using namespace LeapFrog::Brio;

static const U32 kHWControllerDefaultRate = 50;

namespace LF {
namespace Hardware {

  HWControllerPIMPL::HWControllerPIMPL(HWController* controller) :
    controller_(controller),
    wand_(NULL),
    id_(kHWDefaultControllerID),
    mode_(kHWControllerMode),
    color_(kHWControllerLEDOff),
    updateRate_(kHWControllerDefaultRate),
    updateDivider_(1),
    updateCounter_(0),
    has100KOhmJoystick_(1), // Production defaults to 100K; FEP had 10K
    debugMPI_(kGroupController) {
#ifdef DEBUG
	debugMPI_.SetDebugLevel(kDbgLvlVerbose);
#endif
    debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL constructor for %p\n", controller_);
    wand_ = new Vision::VNWand();
    assert(wand_);
    ZeroAllData();

    // Read flag to override default joystick type
    struct stat st;
    if (stat("/flags/joystick-100k", &st) == 0) { /* file exists */
        has100KOhmJoystick_ = 1;
	debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL found /flags/joystick-100k\n");
    }
    if (stat("/flags/joystick-10k", &st) == 0) { /* file exists */
	has100KOhmJoystick_ = 0;
	debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL found /flags/joystick-10k\n");
    }
    debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL setting has100KOhmJoystick_ to %d\n", has100KOhmJoystick_);
  }

  HWControllerPIMPL::~HWControllerPIMPL(void) {
    if (wand_) delete wand_;
  }
  
  void
  HWControllerPIMPL::SetID(LeapFrog::Brio::U8 id) {
    id_ = id;
    wand_->pimpl_->SetID(id_);
  }
  
  void
  HWControllerPIMPL::ZeroAllData(void) {
	    ZeroAccelerometerData();
	    ZeroButtonData();
	    ZeroAnalogStickData();
	    ZeroVersionData();
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

  void HWControllerPIMPL::ZeroVersionData() {
	  hw_version_ = 0;
	  fw_version_ = 0;
  }

  /*!
   * Broad Based Controller Methods
   */
  LeapFrog::Brio::U8 
  HWControllerPIMPL::GetID(void) const {
    return id_;
  }
  
  LeapFrog::Brio::U8
  HWControllerPIMPL::GetHwVersion(void) const {
	  return hw_version_;
  }

  LeapFrog::Brio::U16
  HWControllerPIMPL::GetFwVersion(void) const {
	  return fw_version_;
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
    // error checking
    if (rate >= kHWControllerDefaultRate) {
    	updateRate_ = kHWControllerDefaultRate;
    	updateDivider_ = 1;
    }
    else if (rate > 0) {
    	updateDivider_ = kHWControllerDefaultRate / rate;
    	updateRate_ = kHWControllerDefaultRate / updateDivider_;
    }
    else
    	return kInvalidParamErr;

    return kNoErr;
  }
  
  HWControllerFunctionalityMask 
  HWControllerPIMPL::GetFunctionality(void) const {
    // TOOD: prgrammatically check
    return (kHWControllerHasAccelerometer |
	    kHWControllerHasAnalogStick |
	    kHWControllerHasButtons |
	    kHWControllerHasLED);
  }
  
  /*!
   * Color LED Tip Methods
   */
  HWControllerLEDColorMask 
  HWControllerPIMPL::GetAvailableLEDColors(void) const {
    //TODO: programmatically check
    return (kHWControllerLEDGreen    |
	    kHWControllerLEDRed      |
	    kHWControllerLEDBlue     |
	    kHWControllerLEDYellow   |
	    kHWControllerLEDCyan     |
	    kHWControllerLEDMagenta);
  }
  
  HWControllerLEDColor 
  HWControllerPIMPL::GetLEDColor(void) const {
    return color_;
  }
  
  void 
  HWControllerPIMPL::SetLEDColor(HWControllerLEDColor color) {
	debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::SetLEDColor %08x\n", (unsigned int)color);
	if( (color == kHWControllerLEDOff) || (color & GetAvailableLEDColors())) {
		HWControllerMPIPIMPL::Instance()->SendCommand(controller_, kBTIOCmdSetLEDState, &color, sizeof(color));
		color_ = color;
		wand_->pimpl_->SetColor(color);
	}
	else {
		debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::SetLEDColor Color not available %08x\n", (unsigned int)color);
	}
  }
  
  Vision::VNPoint 
  HWControllerPIMPL::GetLocation(void) const {
    return wand_->GetLocation();
  }
  
  bool 
  HWControllerPIMPL::IsVisible(void) const {
    wand_->IsVisible();
  }


#if defined(EMULATION)

  LeapFrog::Brio::tErrType
  HWControllerPIMPL::StartTracking(HWControllerLEDColor color) {
    SetLEDColor(LF::Hardware::kHWControllerLEDGreen);
    visionMPI_.pimpl_->SetCurrentWand(wand_);
    return kNoErr;
  }

#else
  
  LeapFrog::Brio::tErrType
  HWControllerPIMPL::StartTracking(HWControllerLEDColor color) {
    if (GetCurrentMode() == kHWControllerWandMode) {
      SetLEDColor(color);
      visionMPI_.pimpl_->SetCurrentWand(wand_);
      return kNoErr;
    }
    return kHWControllerNotInWandModeForTracking;
  }

#endif
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
    return (kButtonA | kButtonB | kButtonMenu | kButtonHint);
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

  void
  HWControllerPIMPL::DeadZoneAnalogStickData(tHWAnalogStickData& theData) {
	  const float centerDeadZoneThreshold = GetAnalogStickDeadZone();
	  const float outerDeadZoneThreshold = 0.81f;		//0.9^2
	  const float ordinalThreshold = 0.344f;			//sin(22.5 degrees) * 0.9

	  //When dead zone is set to 0.0f, disable dead zone filter and pass through raw data from analog stick
	  if(centerDeadZoneThreshold == 0.0f) {
		  return;
	  }

	  //Compensate for a systematic non-linear error in joystick
	  // data on production (100K Ohm) joysticks which would cause
	  // the joystick rest position to be outside of the EE
	  // specified center dead zone.  Right now, the average
	  // production joystick X and Y at rest are (142-128)/128 =
	  // +0.109.  We want a function f(raw)=cooked, where
	  // f(-1)=-1, f(0.109)=0, f(1)=1.  We can fit a quadratic
	  // through these three points, ax^2+bx+c. We get a=0.110699,
	  // b=1, c=-a, which simplifies things a bit.  We can use
	  // f(x)=a(x^2-1)+x

#define LINEARIZE_100K(x) 0.110699*(x*x-1)+x
 	  if (has100KOhmJoystick_)
	  {
		  theData.x = LINEARIZE_100K(theData.x);
		  theData.y = LINEARIZE_100K(theData.y);
	  }

	  //Handle the dead zone in the center of the stick
	  if(fabsf(theData.x) <= centerDeadZoneThreshold) theData.x = 0.0f;
	  if(fabsf(theData.y) <= centerDeadZoneThreshold) theData.y = 0.0f;

	  //Handle the dead zone at the outer edge of the stick
	  float stickR2 = (theData.x * theData.x) + (theData.y * theData.y);
	  if(stickR2 >= outerDeadZoneThreshold) {
		  if(fabsf(theData.x) >= ordinalThreshold) {
			  theData.x = (theData.x > 0.0f) ? 1.0f : -1.0f;
		  }
		  else {
			  theData.x = 0.0f;
		  }

		  if(fabsf(theData.y) >= ordinalThreshold) {
			  theData.y = (theData.y > 0.0f) ? 1.0f : -1.0f;
		  }
		  else {
			  theData.y = 0.0f;
		  }
	  }
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
  
  void
  HWControllerPIMPL::SetVersionNumbers(LeapFrog::Brio::U8 hw, LeapFrog::Brio::U16 fw)
  {
	  hw_version_ = hw; fw_version_ = fw;
	  debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::SetVersionNumbers hw=%08x, fw=%08x\n", (unsigned int)hw_version_, (unsigned int)fw_version_);
  }

  inline float BYTE_TO_FLOAT(U8 byte) {
  	  return (float)((int)byte - 128) / 128.0f;
  }

  inline S32 WORD_TO_SIGNED(U16 word) {
  	  return (S32)((word > 127) ? (int)word - 256 : (int)word) >> 2;
  }

  void
  HWControllerPIMPL::LocalCallback(void* context, void* data, int length) {
	  HWControllerPIMPL* pModule = this; //(HWControllerBluetoothPIMPL*)context;
	  U8* packet = (U8*)data;
	  HWControllerMode mode = pModule->mode_;
	  tHWAnalogStickData stick = pModule->analogStickData_;
	  tAccelerometerData accel = pModule->accelerometerData_;
	  struct timeval time;

	  // Moderate device update rate fixed at 50Hz
	  updateCounter_++;
	  if (updateCounter_ % updateDivider_)
		  return;

	  gettimeofday(&time, NULL);

#ifdef DEBUG
	  char str[8], buf[256] = {'\0'};
	  for (int i = 0; i < length; i++) {
		  sprintf(str, "%02x ", packet[i]);
		  strcat(buf, str);
	  }
	  debugMPI_.DebugOut(kDbgLvlVerbose, "%s\n", buf);
#endif

	  pModule->buttonData_.buttonTransition = 0;

	  for (int i = 0; i < length; i++) {
		  switch (i) {
		  case 0:
			  pModule->buttonData_.buttonTransition |= (packet[i]) ? kButtonA : 0;
			  pModule->buttonData_.buttonTransition ^= (pModule->buttonData_.buttonState & kButtonA);
			  pModule->buttonData_.buttonState      &= (packet[i]) ? ~0 : ~kButtonA;
			  pModule->buttonData_.buttonState      |= (packet[i]) ? kButtonA : 0;
			  break;
		  case 1:
			  pModule->buttonData_.buttonTransition |= (packet[i]) ? kButtonB : 0;
			  pModule->buttonData_.buttonTransition ^= (pModule->buttonData_.buttonState & kButtonB);
			  pModule->buttonData_.buttonState      &= (packet[i]) ? ~0 : ~kButtonB;
			  pModule->buttonData_.buttonState      |= (packet[i]) ? kButtonB : 0;
			  break;
		  case 2:
			  pModule->buttonData_.buttonTransition |= (packet[i]) ? kButtonMenu : 0;
			  pModule->buttonData_.buttonTransition ^= (pModule->buttonData_.buttonState & kButtonMenu);
			  pModule->buttonData_.buttonState      &= (packet[i]) ? ~0 : ~kButtonMenu;
			  pModule->buttonData_.buttonState      |= (packet[i]) ? kButtonMenu : 0;
			  break;
		  case 3:
			  pModule->buttonData_.buttonTransition |= (packet[i]) ? kButtonHint : 0;
			  pModule->buttonData_.buttonTransition ^= (pModule->buttonData_.buttonState & kButtonHint);
			  pModule->buttonData_.buttonState      &= (packet[i]) ? ~0 : ~kButtonHint;
			  pModule->buttonData_.buttonState      |= (packet[i]) ? kButtonHint : 0;
			  break;
		  case 4:
			  break;
		  case 5:
			  if (packet[i] && !packet[i-1])
				  pModule->mode_ = kHWControllerMode;
			  else if (!packet[i] && packet[i-1])
				  pModule->mode_ = kHWControllerWandMode;
			  else
				  pModule->mode_ = kHWControllerNoMode;
			  break;
		  case 6:
			  pModule->analogStickData_.x = BYTE_TO_FLOAT(packet[i]);
			  break;
		  case 7:
			  pModule->analogStickData_.y = BYTE_TO_FLOAT(packet[i]);
			  break;
		  case 8:
			  pModule->accelerometerData_.accelX = packet[i];
			  break;
		  case 9:
			  pModule->accelerometerData_.accelX |= (packet[i] << 8);
			  pModule->accelerometerData_.accelX = - WORD_TO_SIGNED(pModule->accelerometerData_.accelX);
			  break;
		  case 10:
			  pModule->accelerometerData_.accelZ = packet[i];
			  break;
		  case 11:
			  pModule->accelerometerData_.accelZ |= (packet[i] << 8);
			  pModule->accelerometerData_.accelZ = WORD_TO_SIGNED(pModule->accelerometerData_.accelZ);
			  break;
		  case 12:
			  pModule->accelerometerData_.accelY = packet[i];
			  break;
		  case 13:
			  pModule->accelerometerData_.accelY |= (packet[i] << 8);
			  pModule->accelerometerData_.accelY = - WORD_TO_SIGNED(pModule->accelerometerData_.accelY);
			  break;
		  }
	  }

	  // Initial connection event
	  if (updateCounter_ <= updateDivider_) {
	      HWControllerEventMessage cmsg(kHWControllerConnected, pModule->controller_);
		  pModule->eventMPI_.PostEvent(cmsg, 128);
	  }

	  if (mode != pModule->mode_) {
	      HWControllerEventMessage cmsg(kHWControllerModeChanged, pModule->controller_);
		  pModule->eventMPI_.PostEvent(cmsg, 128);
	  }

#if 0 // FIXME -- distinguish composite event message from ala-carte event messages?
	  // Compatibility events posted only for default controller
	  if (pModule->id_ > 0) {
	      HWControllerEventMessage cmsg(kHWControllerDataChanged, pModule->controller_);
		  pModule->eventMPI_.PostEvent(cmsg, 128);
		  return;
	  }
#endif

	  DeadZoneAnalogStickData(pModule->analogStickData_);
	  if (memcmp(&stick, &analogStickData_, sizeof(tHWAnalogStickData)) != 0) {
		  pModule->analogStickData_.time.seconds = time.tv_sec;
		  pModule->analogStickData_.time.microSeconds = time.tv_usec;
		  if (pModule->id_ > 0) {
		      HWControllerEventMessage cmsg(kHWControllerAnalogStickDataChanged, pModule->controller_);
			  pModule->eventMPI_.PostEvent(cmsg, 128);
		  }
		  else {
			  HWAnalogStickMessage amsg(pModule->analogStickData_);
			  pModule->eventMPI_.PostEvent(amsg, 128);
		  }
	  }

	  if (memcmp(&accel, &accelerometerData_, sizeof(tAccelerometerData)) != 0) {
		  pModule->accelerometerData_.time.seconds = time.tv_sec;
		  pModule->accelerometerData_.time.microSeconds = time.tv_usec;
		  if (pModule->id_ > 0) {
		      HWControllerEventMessage cmsg(kHWControllerAccelerometerDataChanged, pModule->controller_);
			  pModule->eventMPI_.PostEvent(cmsg, 128);
		  }
		  else {
			  CAccelerometerMessage xmsg(pModule->accelerometerData_);
			  pModule->eventMPI_.PostEvent(xmsg, 128);
		  }
	  }

	  if (pModule->buttonData_.buttonTransition) {
		  pModule->buttonData_.time.seconds = time.tv_sec;
		  pModule->buttonData_.time.microSeconds = time.tv_usec;
		  if (pModule->id_ > 0) {
		      HWControllerEventMessage cmsg(kHWControllerButtonStateChanged, pModule->controller_);
			  pModule->eventMPI_.PostEvent(cmsg, 128);
		  }
		  else {
			  CButtonMessage bmsg(pModule->buttonData_);
			  pModule->eventMPI_.PostEvent(bmsg, 128);
		  }
	  }
  }

}	// namespace Hardware
}	// namespace LF
