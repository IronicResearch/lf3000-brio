#ifndef __INCLUDE_HARDWARE_HWCONTROLLERPIMPL_H__
#define __INCLUDE_HARDWARE_HWCONTROLLERPIMPL_H__

#include <Hardware/HWControllerTypes.h>
#include <Vision/VNVisionMPI.h>
#include <ButtonMPI.h>
#include <AccelerometerMPI.h>
#include <Hardware/HWAnalogStickMPI.h>

namespace LF {
namespace Hardware {
  
  class HWControllerPIMPL {
  public:
    HWControllerPIMPL(HWController* controller);
    virtual ~HWControllerPIMPL(void);
    
    /*!
     * Broad Based Controller Methods
     */
    LeapFrog::Brio::U8 GetID(void) const;
    HWControllerMode GetCurrentMode(void) const;
    bool IsConnected(void) const;
    LeapFrog::Brio::U32 GetControllerUpdateRate(void) const;
    LeapFrog::Brio::tErrType SetControllerUpdateRate(LeapFrog::Brio::U32 rate);
    HWControllerFunctionalityMask GetFunctionality(void) const;

    /*!
     * Color LED Tip Methods
     */
    HWControllerLEDColorMask GetAvailableLEDColors(void) const;
    HWControllerLEDColor GetLEDColor(void) const;
    void SetLEDColor(HWControllerLEDColor color);
    Vision::VNPoint GetLocation(void) const;
    bool IsVisible(void) const;
   
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
    
  private:
    Vision::VNVisionMPI visionMPI_;
    LeapFrog::Brio::CAccelerometerMPI accelerometerMPI_;
    LeapFrog::Brio::CButtonMPI buttonMPI_;
    Hardware::HWAnalogStickMPI analogStickMPI_;

    HWController* controller_;
    LeapFrog::Brio::U8 id_;
    HWControllerMode mode_;
    LeapFrog::Brio::U32 updateRate_;

    LeapFrog::Brio::tAccelerometerData accelerometerData_;
    LeapFrog::Brio::tButtonData2 buttonData_;
    LF::Hardware::tHWAnalogStickData analogStickData_;

    void ZeroAccelerometerData(void);
    void ZeroButtonData(void);
    void ZeroAnalogStickData(void);

    friend class HWControllerBluetoothPIMPL;
  };
  
}	// namespace Hardware
}	// namespace LF

#endif // __INCLUDE_HARDWARE_HWCONTROLLERPIMPL_H__
