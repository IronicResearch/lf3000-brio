#ifndef __INCLUDE_HARDWARE_HWCONTROLLER_H__
#define __INCLUDE_HARDWARE_HWCONTROLLER_H__

#include <Hardware/HWControllerTypes.h>
#include <Vision/VNVisionTypes.h>
#include <Hardware/HWAnalogStickTypes.h>
#include <Hardware/HWControllerTypes.h>
#include <AccelerometerTypes.h>
#include <ButtonTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Hardware {

  // forward declarations
  class HWControllerPIMPL;
  class HWControllerMPIPIMPL;

  /*!
   * \class HWController
   *
   * NOTE: For use with LeapTV applications ONLY.
   * HWController class is the software interface to the physical controller
   * on Glasgow devices.  Controllers have the following functionality:
   * Button input
   * Analog stick input
   * Accelerometer
   * Color wand pointer
   *
   * This class allows accessing information for each of these input types.  Developers can
   * register for event notifications for controllers using the HWControllerMPI class.
   *
   * HWController classes are created by the LF SDK and handed out via the HWControllerMPI.
   * Physical controllers communicate with the Glasgow device to notify the device that they
   * are ready to interact.  The device then creates an instance of a HWController class.
   */

  class HWController {
  public:
    /*!
     * \brief Returns the ID of the controller
     */
    LeapFrog::Brio::U8 GetID(void) const;

    /*!
     * \brief Returns the hardware version/revision of the controller
     */
    LeapFrog::Brio::U8 GetHwVersion(void) const;

    /*!
     * \brief Returns the firmware version/revision of the controller
     */
    LeapFrog::Brio::U8 GetFwVersion(void) const;

    /*!
     * Obtain the current mode of the controller
     * \return the current controller mode
     */
    HWControllerMode GetCurrentMode(void) const;

    /*!
     * Check to see if the controller is connected to the console
     * \return true if the controller is connected to console, false otherwise
     */
    bool IsConnected(void) const;

    /*!
     * \return The frequency at which the controller is sending data
     * back to the console.
     */
    LeapFrog::Brio::U32 GetControllerUpdateRate(void) const;

    /*!
     * Sets the sampling rate for the controller data transmission via bluetooth
     * \param rate The sampling rate.
     * \return An error code if one is present, otherwise LeapFrog::Brio::kNoErr
     */
    LeapFrog::Brio::tErrType SetControllerUpdateRate(LeapFrog::Brio::U32 rate);

    /*!
     * \brief GetFunctionality used to determine what hardware functionality
     * is available on this controller
     * \returns a HWControllerFunctionalityMask with the bits flipped on for
     * each piece of hardware functionality that is available
     */
    HWControllerFunctionalityMask GetFunctionality(void) const;

    //-----------------------------------------------------------------------
    /*!
     * Methods associated with the LED color wand tip
     */
    /*!
     * \brief returns the available colors in the form of a bit mask
     * \return a bit mask, HWControllerLEDColorMask, with bits of available
     * colors flipped on.  See HWControllerTypes.h for list of possible colors.
     */
    HWControllerLEDColorMask GetAvailableLEDColors(void) const;

    /*!
     * \brief Obtain the current color of the LED.  A value of
     * kHWControllerLEDOff indicates the LED is currently off or
     * the controller does not support LED.  This can be checked
     * using \saGetFunctionality
     * \return the current LED color of the wand pointer
     */
    HWControllerLEDColor GetLEDColor(void) const;

    /*!
     * \brief sets the wand pointer color.  This method will turn the LED
     * on if it is currently off and the value of color passed in is not
     * kHWControllerLEDOff.
     * \param color The color from the HWControllerLEDColor enumeration
     */
    void SetLEDColor(HWControllerLEDColor color);

    /*!
     * \brief Used in conjunction with the VNWandTrackingAlgorithm for
     * determining the x,y coordinates of the color tip on the screen.
     */
    Vision::VNPoint GetLocation(void) const;

    /*!
     * \return true if the LED wand tip is currently visible in the camera viewport
     */
    bool IsVisible(void) const;

    /*!
     * \brief Tells the system that this is the controller to start tracking
     * inside of the VisionMPI.  This method should only be called when the
     * controller is in HWControllerWandMode.  If called while in any other
     * mode it will be a no-op and an error will be returned.
     * \param color This is the suggested color of the wand for light tracking.
     * This is only a suggestion as the VNWandTracker algorithm may choose
     * a different color based on environmental conditions.
     * \return kNoErr if successful
     */
    LeapFrog::Brio::tErrType StartTracking(HWControllerLEDColor color = kHWControllerLEDDefaultColor);

    //-----------------------------------------------------------------------
    /*!
     * Methods associated with the buttons
     */
    /*!
     * \brief Returns the current button data associated with this controller
     * \return the button data, including the current state and transition
     * of the buttons associated with this controller.  If the controller is
     * not connected this data be zeroed out.
     */
    LeapFrog::Brio::tButtonData2 GetButtonData(void) const;


    //-----------------------------------------------------------------------
    /*!
     * Methods associated with the analog stick
     */
    /*!
     * \breif HWControllerButtonMask GetAvailableButtons(void) const;
     * \return a bitmask representing all available buttons on this controller
     * See ButtonTypes.h to see the list of possible buttons.
     */
    HWControllerButtonMask GetAvailableButtons(void) const;

    /*!
     * \brief Obtains the data for the analog stick associated with this
     * controller
     * \return The data associated with the analog stick.  If the controller
     * is not connected this data will be zeroed out.
     */
    tHWAnalogStickData GetAnalogStickData(void) const;

    /*!
     * \brief Obtains the mode of the analog stick associated with this controller
     * \return the mode the stick is in
     */
    tHWAnalogStickMode GetAnalogStickMode(void) const;

    /*!
     * \brief set the analog stick mode for the stick associated with this controller
     * \param mode the desired mode to set the analog stick with id stickID
     */
    LeapFrog::Brio::tErrType SetAnalogStickMode(tHWAnalogStickMode mode);

    /*!
     * \brief returns the dead zone parameter for the analog stick associated with
     * this controller
     * \return the value for the dead zone of the stick with id stickID.  If the
     * controller is not connected this value will be zero;
     */
    float GetAnalogStickDeadZone(void) const;

    /*!
     * \brief Set the dead zone
     * The dead zone is the area at which the sticks do not report any events.
     * The default is 0.14, which provides a balance between sensitivity
     * and no movement when the stick is centered. Decreasing the value
     * will increase sensitivity. Increasing the value will prevent movement.
     * The range is a normalized value between 0 .. 1
     * \param deadZone the desired dead zone value
     * \return Returns kNoErr on success.
     */
    LeapFrog::Brio::tErrType SetAnalogStickDeadZone(const float deadZone);

    //-----------------------------------------------------------------------
    /*!
     * Methods associated with the accelerometer
     */
    /*!
     * \return The current data from the accelerometer associated with this controller
     * IF the controller is not connected this data will be zerod out.
     */
    LeapFrog::Brio::tAccelerometerData GetAccelerometerData(void) const;

    /*!
     * \return the current accelerometer mode, currently only continuous sampling
     * is supported.
     */
    LeapFrog::Brio::tAccelerometerMode GetAccelerometerMode(void) const;

    /*!
     * \brief sets the mode of the accelerometer associated with this controller
     * \param mode The mode to set the accelerometer, currently only kAccelerometerModeContinuous
     * is supported. Any other mode will return kInvalidParamErr.
     */
    LeapFrog::Brio::tErrType SetAccelerometerMode(const LeapFrog::Brio::tAccelerometerMode mode);

    /*!
     * \brief Gets the bluetooth address of this controller.
     * \return character string of bluetooth address.
     */
     const char* GetBluetoothAddress();

  private:
    boost::shared_ptr<HWControllerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     * default constructor and destructor
     */
    HWController(const HWController&);
    HWController& operator =(const HWController&);
    HWController(void);
    virtual ~HWController(void);

    friend class HWControllerMPIPIMPL;
  };

} // namespace Hardware
} // namespace LF

#endif // __INCLUDE_HARDWARE_HWCONTROLLER_H__
