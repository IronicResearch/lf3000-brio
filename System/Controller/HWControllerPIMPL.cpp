#include "HWControllerPIMPL.h"
#include "HWControllerMPIPIMPL.h"
#include <Hardware/HWControllerTypes.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <VNVisionMPIPIMPL.h>
#include <PowerMPI.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// flat profiler
//#define VN_PROFILE 1
#undef VN_PROFILE
#include <VNProfiler.h>
#define TIME_INTERNAL_METHODS 0

using namespace LeapFrog::Brio;

static const U32 kHWDefaultEventPriority = 0;
static const U32 kHWControllerDefaultRate = 50;

#define BATTERY_STATE_COUNT_THRESHOLD 25
#define POWER_STATE_COUNT_THRESHOLD 50

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
    connected_(false),
    analogStickDeadZone_(0.14f), //Specified by EE team at +/-7%, which is -0.14..+0.14 in -1.0..+1.0 scale.  FWGLAS-779)
    analogStickMode_ (kHWAnalogStickModeAnalog),
    debugMPI_(kGroupController),
    analogStickMsg_(kHWControllerAnalogStickDataChanged, controller),
    accelerometerMsg_(kHWControllerAccelerometerDataChanged, controller),
    buttonMsg_(kHWControllerButtonStateChanged, controller) {
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

    sprintf( blueToothAddress_, "0xdeadbeef" );
  }

  HWControllerPIMPL::~HWControllerPIMPL(void) {
    if (wand_) delete wand_;
  }

  void
  HWControllerPIMPL::SetID(LeapFrog::Brio::U8 id) {
    id_ = id;
    //BADBAD: should not be accessing pimpl from here.  SetID should be a private member in Wand and HWControllerPIMPL as friend.
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
	  return connected_;
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
		// FIXME: updating wand color state is expensive
		if (color_ == color)
			return;
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
    const float centerDeadZoneThreshold = analogStickDeadZone_;
    const float cdzt2 = centerDeadZoneThreshold*centerDeadZoneThreshold;
    static const float zeroThreshold = 0.00000001f; // near machine precision for float
    static const float outerDeadZoneThreshold = 0.81f;		//0.9^2
    static const float ordinalThreshold = 0.344f;		//sin(22.5 degrees) * 0.9
    static const float ot2 = 0.118336f;                         //ordinalThreshold^2
    
    //When dead zone is set to 0.0f, disable dead zone filter and pass through raw data from analog stick
    if (cdzt2 < zeroThreshold) {
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
    
    const float x2 = theData.x*theData.x;
    const float y2 = theData.y*theData.y;
    static const float a = 0.110699f;
    
    if (has100KOhmJoystick_) {
      theData.x = a*(x2-1.f)+theData.x; 
      theData.y = a*(y2-1.f)+theData.y; 
    }
    
    //Handle the dead zone in the center of the stick
    if (x2 <= cdzt2) {
      theData.x = 0.0f;
    }
    if (y2 <= cdzt2) { 
      theData.y = 0.0f;
    }
    
    //Handle the dead zone at the outer edge of the stick
    if ((x2+y2) >= outerDeadZoneThreshold) {
      if (x2 >= ot2) {
	theData.x = (theData.x > 0.0f) ? 1.0f : -1.0f;
      } else {
	theData.x = 0.0f;
      }
      
      if (y2 >= ot2) {
	theData.y = (theData.y > 0.0f) ? 1.0f : -1.0f;
      } else {
	theData.y = 0.0f;
      }
    }
  }


  tHWAnalogStickMode
  HWControllerPIMPL::GetAnalogStickMode(void) const {
    return analogStickMode_;
  }
  
 bool
HWControllerPIMPL::ApplyAnalogStickMode(tHWAnalogStickData& theData) {
	bool retVal = true;

	switch(GetAnalogStickMode()) {
	case kHWAnalogStickModeDisable:
		theData.x = 0.0f;
		theData.y = 0.0f;
		retVal = false;
		break;
	case kHWAnalogStickModeDPad:
		ConvertAnalogStickToDpad(theData);
		theData.x = 0.0f;
		theData.y = 0.0f;
		retVal = false;
		break;
	case kHWAnalogStickModeAnalog:
		retVal = true;
		break;
	default:
		retVal = true;
		debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::ApplyAnalogStickMode - Unknown mode detected %08x\n", (unsigned int)GetAnalogStickMode());
		break;
	}

  return retVal;
}

void
HWControllerPIMPL::ConvertAnalogStickToDpad(const tHWAnalogStickData& theData) {

	U32 originalButtonState = buttonData_.buttonState;

	ThresholdAnalogStickButton(-theData.x, kButtonLeft);
	ThresholdAnalogStickButton(theData.x, kButtonRight);
	ThresholdAnalogStickButton(-theData.y, kButtonDown);
	ThresholdAnalogStickButton(theData.y, kButtonUp);

	buttonData_.buttonTransition |= (originalButtonState ^ buttonData_.buttonState);
}

void
HWControllerPIMPL::ThresholdAnalogStickButton(float stickPos, U32 buttonMask) {
	const float kHWControllerDpadOnThreshold = 0.25f;
	const float kHWControllerDpadOffThreshold = 0.15f;

	if(buttonData_.buttonState & buttonMask) {
		if(stickPos < kHWControllerDpadOffThreshold) buttonData_.buttonState &= ~buttonMask;
	}
	else {
		if(stickPos > kHWControllerDpadOnThreshold) buttonData_.buttonState |= buttonMask;
	}
}

  LeapFrog::Brio::tErrType 
  HWControllerPIMPL::SetAnalogStickMode(tHWAnalogStickMode mode) {
	if(mode != kHWAnalogStickModeDPad)
	{
		U32 originalButtonState = buttonData_.buttonState;
		buttonData_.buttonState &= ~(kButtonLeft | kButtonRight | kButtonDown | kButtonUp);
		buttonData_.buttonTransition |= (originalButtonState ^ buttonData_.buttonState);
	}
	analogStickMode_ = mode;

	debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::SetAnalogStickMode  %08x\n", (unsigned int)mode);

    return kNoErr;
  }

  float
  HWControllerPIMPL::GetAnalogStickDeadZone(void) const {
    return analogStickDeadZone_;
  }

  LeapFrog::Brio::tErrType
  HWControllerPIMPL::SetAnalogStickDeadZone(const float deadZone) {
	  analogStickDeadZone_ = deadZone;
    return kNoErr;
  }

  /*!
   * Accelerometer Related Methods
   */
  LeapFrog::Brio::tAccelerometerData
  HWControllerPIMPL::GetAccelerometerData(void) const {
    return accelerometerData_;
  }

  void
  HWControllerPIMPL::SetAccelerometerData(const LeapFrog::Brio::tAccelerometerData &data) {
    memcpy(&accelerometerData_, &data, sizeof(data));
  }

  LeapFrog::Brio::tAccelerometerMode
  HWControllerPIMPL::GetAccelerometerMode(void) const {
	return kAccelerometerModeContinuous;
  }

  LeapFrog::Brio::tErrType
  HWControllerPIMPL::SetAccelerometerMode(const LeapFrog::Brio::tAccelerometerMode mode) {
    if(mode == kAccelerometerModeContinuous)
    	return kNoErr;
    else
    	return kInvalidParamErr;
  }

  void
  HWControllerPIMPL::SetVersionNumbers(LeapFrog::Brio::U8 hw, LeapFrog::Brio::U16 fw)
  {
	  hw_version_ = hw; fw_version_ = fw;
	  debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::SetVersionNumbers hw=%08x, fw=%08x\n", (unsigned int)hw_version_, (unsigned int)fw_version_);
  }

  void
  HWControllerPIMPL::SetConnected(bool connected)
  {
	  connected_ = connected;
  }

  inline float BYTE_TO_FLOAT(U8 byte) {
  	  return (float)((int)byte - 128) / 128.0f;
  }

  inline S32 WORD_TO_SIGNED(U16 word) {
  	  return (S32)((word > 127) ? (int)word - 256 : (int)word) >> 2;
  }

  void
  HWControllerPIMPL::LocalCallback(void* context, void* data, int length) {
          PROF_BLOCK_START("LocalCallback");

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_START("setup");
#endif

	  U8* packet = static_cast<U8*>(data);
	  HWControllerMode mode = mode_;
	  tHWAnalogStickData stick = analogStickData_;
	  tAccelerometerData accel = accelerometerData_;
	  struct timeval time;
	  static U8 power_counter = 0; //FWGLAS-547: Counter for tracking when it's time to post KeepAlive
	  static U8 lowBatteryCounter = 0; //FWGLAS-547: Counter for tracking when it's time to post low battery warning
	  bool lowBatteryStatus = false; 

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("updateCounter");
#endif

	  // Moderate device update rate fixed at 50Hz
	  updateCounter_++;
	  if (updateCounter_ % updateDivider_) {	    
	    return;
	  }

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("getTime");
#endif

	  gettimeofday(&time, NULL);

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

#ifdef DEBUG
	  char str[8], buf[256] = {'\0'};
	  for (int i = 0; i < length; i++) {
		  sprintf(str, "%02x ", packet[i]);
		  strcat(buf, str);
	  }
	  debugMPI_.DebugOut(kDbgLvlVerbose, "%s\n", buf);
#endif

	  PROF_BLOCK_START("parse");
#endif

	  //Parse the packet	
	  buttonData_.buttonTransition = 0;

	  buttonData_.buttonTransition |= (packet[0]) ? kButtonA : 0;
	  buttonData_.buttonTransition ^= (buttonData_.buttonState & kButtonA);
	  buttonData_.buttonState      &= (packet[0]) ? ~0 : ~kButtonA;
	  buttonData_.buttonState      |= (packet[0]) ? kButtonA : 0;
	  
	  buttonData_.buttonTransition |= (packet[1]) ? kButtonB : 0;
	  buttonData_.buttonTransition ^= (buttonData_.buttonState & kButtonB);
	  buttonData_.buttonState      &= (packet[1]) ? ~0 : ~kButtonB;
	  buttonData_.buttonState      |= (packet[1]) ? kButtonB : 0;

	  buttonData_.buttonTransition |= (packet[2]) ? kButtonMenu : 0;
	  buttonData_.buttonTransition ^= (buttonData_.buttonState & kButtonMenu);
	  buttonData_.buttonState      &= (packet[2]) ? ~0 : ~kButtonMenu;
	  buttonData_.buttonState      |= (packet[2]) ? kButtonMenu : 0;

	  buttonData_.buttonTransition |= (packet[3]) ? kButtonHint : 0;
	  buttonData_.buttonTransition ^= (buttonData_.buttonState & kButtonHint);
	  buttonData_.buttonState      &= (packet[3]) ? ~0 : ~kButtonHint;
	  buttonData_.buttonState      |= (packet[3]) ? kButtonHint : 0;

	  if (packet[5] && !packet[4])
		  mode_ = kHWControllerMode;
	  else if (!packet[5] && packet[4])
		  mode_ = kHWControllerWandMode;
	  else
		  mode_ = kHWControllerNoMode;

	  analogStickData_.x = BYTE_TO_FLOAT(packet[6]);

	  analogStickData_.y = BYTE_TO_FLOAT(packet[7]);

	  accelerometerData_.accelX = packet[8];
	  accelerometerData_.accelX = - WORD_TO_SIGNED(accelerometerData_.accelX);

	  accelerometerData_.accelZ = packet[10];
	  accelerometerData_.accelZ = WORD_TO_SIGNED(accelerometerData_.accelZ);

	  accelerometerData_.accelY = packet[12];
	  accelerometerData_.accelY = - WORD_TO_SIGNED(accelerometerData_.accelY);

	  //ProcessLowBatteryStatus(packet[15]);
	  lowBatteryStatus = packet[15];

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("connectionEvent");
#endif

	  // Initial connection event
	  if (updateCounter_ <= updateDivider_) {
	      HWControllerEventMessage cmsg(kHWControllerConnected, controller_);
		  eventMPI_.PostEvent(cmsg, kHWDefaultEventPriority);
	  }

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("modeEvent");
#endif

	  if (mode != mode_) {
	      HWControllerEventMessage cmsg(kHWControllerModeChanged, controller_);
		  eventMPI_.PostEvent(cmsg, kHWDefaultEventPriority);
	  }

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

#if 0 // FIXME -- distinguish composite event message from ala-carte event messages?
	  // Compatibility events posted only for default controller
	  if (id_ > 0) {
	      HWControllerEventMessage cmsg(kHWControllerDataChanged, controller_);
		  eventMPI_.PostEvent(cmsg, kHWDefaultEventPriority);
		  return;
	  }
#endif

	  PROF_BLOCK_START("DeadZone");
#endif

	  DeadZoneAnalogStickData(analogStickData_);

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("ApplyAnalogStickMode");
#endif

	  if(ApplyAnalogStickMode(analogStickData_)) {
		//if(GetAnalogStickMode() == kHWAnalogStickModeAnalog) {
		  //if (memcmp(&stick, &analogStickData_, sizeof(tHWAnalogStickData)) != 0) {
		  if (stick.x != analogStickData_.x || stick.y != analogStickData_.y)
		  {
			  analogStickData_.time.seconds = time.tv_sec;
			  analogStickData_.time.microSeconds = time.tv_usec;
			  eventMPI_.PostEvent(analogStickMsg_, kHWDefaultEventPriority);
		  }
		  //}
	  }

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("accelerationEvent");
#endif

	  //if (memcmp(&accel, &accelerometerData_, sizeof(tAccelerometerData)) != 0) {
	  if(accel.accelX != accelerometerData_.accelX || accel.accelY != accelerometerData_.accelY || accel.accelZ != accelerometerData_.accelZ)
	  {
		  accelerometerData_.time.seconds = time.tv_sec;
		  accelerometerData_.time.microSeconds = time.tv_usec;
		  if (buttonData_.buttonState & kButtonB) //FWGLAS-1456: If button B is pressed, x-axis value of accelerometer changes to 1. 								   	
		    accelerometerData_.accelX = accel.accelX; //So for now, report the previous value if button B is pressed. 
		  eventMPI_.PostEvent(accelerometerMsg_, kHWDefaultEventPriority);
	  }
	  //}

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("buttonEvent");
#endif

	  if (buttonData_.buttonTransition) {
		  buttonData_.time.seconds = time.tv_sec;
		  buttonData_.time.microSeconds = time.tv_usec;
	      eventMPI_.PostEvent(buttonMsg_, kHWDefaultEventPriority);
	      power_counter++;
	  }

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();

	  PROF_BLOCK_START("batteryEvent");
#endif

	  //FWGLAS-547: Do KeepAlive and low battery checks less often.
	  //These should probably be separate methods but trying to avoid the overhead of a function call...
	  if (power_counter == BATTERY_STATE_COUNT_THRESHOLD)
	  {
		  CPowerMPI::KeepAlive();
		  power_counter = 0;
	  }
	  
	  if(lowBatteryStatus)
	  {
		  lowBatteryCounter++;
	  }
	  else
	  {
		  lowBatteryCounter = 0;
	  }
	  
	  if(lowBatteryCounter == BATTERY_STATE_COUNT_THRESHOLD)
	  {
		  HWControllerEventMessage cmsg(kHWControllerLowBattery, controller_);
		  eventMPI_.PostEvent(cmsg, 128);
		  debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::Posting  kHWControllerLowBattery\n");
	  }

#if TIME_INTERNAL_METHODS
	  PROF_BLOCK_END();
#endif
	  PROF_BLOCK_END();
	  PROF_PRINT_REPORT_AFTER_COUNT(500);
  }

    void
    HWControllerPIMPL::ProcessLowBatteryStatus(U8 batteryStatus) {
    	static bool lastLowBatteryStatus = false;
    	bool thisLowBatteryStatus = lastLowBatteryStatus;
    	static int lowBatteryCounter = 0;
    	static int goodBatteryCounter = 0;
    	const int batteryStateCountThreshold = 25;

        //Read the battery voltage and process low battery states
        //FWGLAS1296 - Both battery counters count up to 25 so we
        //are not updating power LED state either way till the
        //reading is consistent for 500ms. The counters reset
        //each other if the reading is not consistent for
        //any time less than that.
        if(batteryStatus)
        {
        	lowBatteryCounter++;
        	goodBatteryCounter = 0;
        }
        else
        {
        	lowBatteryCounter = 0;
        	goodBatteryCounter++;
        }

        if(lowBatteryCounter >= batteryStateCountThreshold)
        {
        	thisLowBatteryStatus = true;
        	lowBatteryCounter = 0;
        }

        if (goodBatteryCounter >= batteryStateCountThreshold)
        {
        	thisLowBatteryStatus = false;
        	goodBatteryCounter = 0;
        }

        if(thisLowBatteryStatus != lastLowBatteryStatus) {
        	lastLowBatteryStatus = thisLowBatteryStatus;
        	if(thisLowBatteryStatus) {
        		HWControllerEventMessage cmsg(kHWControllerLowBattery, controller_);
        		eventMPI_.PostEvent(cmsg, 128);
        		debugMPI_.DebugOut(kDbgLvlValuable, "HWControllerPIMPL::Posting  kHWControllerLowBattery\n");
        	}
        }
    }

    const char*
    HWControllerPIMPL::GetBluetoothAddress() {
        return blueToothAddress_;
    }

    void
    HWControllerPIMPL::SetBluetoothAddress( const char* btaddress ) {
      if (btaddress) {
        const int btval = *((int*)btaddress);
        memset( blueToothAddress_, 0, sizeof(blueToothAddress_) );
        sprintf( blueToothAddress_, "0x%x", btval );
        //BADBAD accessing pimpl directly
        wand_->pimpl_->SetBluetoothAddress(blueToothAddress_);
      } 
    }
}	// namespace Hardware
}	// namespace LF
