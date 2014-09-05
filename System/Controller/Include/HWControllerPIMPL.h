#ifndef __INCLUDE_HARDWARE_HWCONTROLLERPIMPL_H__
#define __INCLUDE_HARDWARE_HWCONTROLLERPIMPL_H__

#include <Hardware/HWControllerTypes.h>
#include <Vision/VNVisionMPI.h>
#include <ButtonMPI.h>
#include <AccelerometerMPI.h>
#include <Hardware/HWAnalogStickMPI.h>
#include <DebugMPI.h>

namespace LF {

namespace Vision {
  class VNWand;
}

namespace Hardware {

  class HWControllerPIMPL {
  public:
    HWControllerPIMPL(HWController* controller);
    virtual ~HWControllerPIMPL(void);

    /*!
     * Broad Based Controller Methods
     */
    LeapFrog::Brio::U8 GetID(void) const;
    LeapFrog::Brio::U8 GetHwVersion(void) const;
    LeapFrog::Brio::U16 GetFwVersion(void) const;
    HWControllerMode GetCurrentMode(void) const;
    bool IsConnected(void) const;
    LeapFrog::Brio::U32 GetControllerUpdateRate(void) const;
    LeapFrog::Brio::tErrType SetControllerUpdateRate(LeapFrog::Brio::U32 rate);
    HWControllerFunctionalityMask GetFunctionality(void) const;
    void ZeroAllData(void);

    /*!
     * Color LED Tip Methods
     */
    HWControllerLEDColorMask GetAvailableLEDColors(void) const;
    HWControllerLEDColor GetLEDColor(void) const;
    void SetLEDColor(HWControllerLEDColor color);
    Vision::VNPoint GetLocation(void) const;
    bool IsVisible(void) const;
    LeapFrog::Brio::tErrType StartTracking(HWControllerLEDColor color);

    /*!
     * Button Related Methods
     */
    LeapFrog::Brio::tButtonData2 GetButtonData(void) const;
    void SetButtonData(const LeapFrog::Brio::tButtonData2 &data);
    HWControllerButtonMask GetAvailableButtons(void) const;

    /*!
     * AnalogStick Related Methods
     */
    tHWAnalogStickData GetAnalogStickData(void) const;
    void SetAnalogStickData(const tHWAnalogStickData &data);
    tHWAnalogStickMode GetAnalogStickMode(void) const;
    LeapFrog::Brio::tErrType SetAnalogStickMode(tHWAnalogStickMode mode);
    float GetAnalogStickDeadZone(void) const;
    LeapFrog::Brio::tErrType SetAnalogStickDeadZone(const float deadZone);

    /*!
     * Accelerometer Related Methods
     */
    LeapFrog::Brio::tAccelerometerData GetAccelerometerData(void) const;
    void SetAccelerometerData(const LeapFrog::Brio::tAccelerometerData &data);
    LeapFrog::Brio::tAccelerometerMode GetAccelerometerMode(void) const;
    LeapFrog::Brio::tErrType SetAccelerometerMode(const LeapFrog::Brio::tAccelerometerMode mode);

    /*!
    * Configuration Related Methods
    */
    void LocalCallback(void*, void*, int);
    void SetID(LeapFrog::Brio::U8 id);
    void SetVersionNumbers(LeapFrog::Brio::U8 hw, LeapFrog::Brio::U16 fw);
    void SetBluetoothAddress(const char* btaddress);
    const char* GetBluetoothAddress();
    void SetConnected(bool connected);

 private:
    Vision::VNVisionMPI visionMPI_;
    LeapFrog::Brio::CAccelerometerMPI accelerometerMPI_;
    LeapFrog::Brio::CButtonMPI buttonMPI_;
    Hardware::HWAnalogStickMPI analogStickMPI_;
    LeapFrog::Brio::CEventMPI eventMPI_;
    LeapFrog::Brio::CDebugMPI debugMPI_;

    HWController* controller_;
    LF::Vision::VNWand* wand_;
    LeapFrog::Brio::U8 id_;
    LeapFrog::Brio::U8 hw_version_;
    LeapFrog::Brio::U16 fw_version_;
    HWControllerMode mode_;
    HWControllerLEDColor color_;
    LeapFrog::Brio::U32 updateRate_;
    LeapFrog::Brio::U32 updateDivider_;
    LeapFrog::Brio::U32 updateCounter_;

    LeapFrog::Brio::U8 has100KOhmJoystick_;

    LeapFrog::Brio::tAccelerometerData accelerometerData_;
    LeapFrog::Brio::tButtonData2 buttonData_;
    LF::Hardware::tHWAnalogStickData analogStickData_;
    LF::Hardware::tHWAnalogStickMode analogStickMode_;
    float analogStickDeadZone_;

    char blueToothAddress_[64];
    bool connected_;

    void ZeroAccelerometerData(void);
    void ZeroButtonData(void);
    void ZeroAnalogStickData(void);
    void ZeroVersionData(void);
    void DeadZoneAnalogStickData(tHWAnalogStickData& theData);
    bool ApplyAnalogStickMode(tHWAnalogStickData& theData);
    void ConvertAnalogStickToDpad(const tHWAnalogStickData& theData);
    void ThresholdAnalogStickButton(float stickPos, U32 buttonMask);
    void ProcessLowBatteryStatus(U8 batteryStatus);

  };

}	// namespace Hardware
}	// namespace LF

#endif // __INCLUDE_HARDWARE_HWCONTROLLERPIMPL_H__
