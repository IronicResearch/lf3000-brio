#ifndef __INCLUDE_HARDWARE_HWCONTROLLERMPI_H__
#define __INCLUDE_HARDWARE_HWCONTROLLERMPI_H__

#include <CoreMPI.h>
#include <Hardware/HWControllerTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace LF {
namespace Hardware {

  class HWControllerMPIPIMPL;

  /*!
   * \class HWControllerMPI
   *
   * NOTE: For use with LeapTV applications ONLY.
   * HWControllerMPI class to support communication about controller objects.  Controllers
   * are physical devices that can connect to the LF platform and contain button events,
   * analog stick input, color end effects and accelerometer data.  This class facilitates
   * in acquiring HWController objects to interact with during game play.
   *
   * Each physical controller object will notify the SDK that it is on and trying to
   * connect.  Therefore, there is no progrmmatic need to attempt to connect to the
   * physical device.  This class faciliates the communication about all controllers.
   * Communication to a specific controller should be done via the HWController class and
   * the objects returned from GetControllerByID.
   */

  class HWControllerMPI : public LeapFrog::Brio::ICoreMPI {
  public:
    /*!
     * \brief Constructor
     */
    HWControllerMPI(void);

    /*!
     * \brief Destructor
     */
    virtual ~HWControllerMPI(void);

    /*!
     * \defgroup Virtual Base Class Methods
     * \brief These five methods are declared as virtual in the base class, ICoreMPI
     */
    virtual LeapFrog::Brio::Boolean        IsValid(void) const;
    virtual const LeapFrog::Brio::CString* GetMPIName(void) const;
    virtual LeapFrog::Brio::tVersion       GetModuleVersion(void) const;
    virtual const LeapFrog::Brio::CString* GetModuleName(void) const;
    virtual const LeapFrog::Brio::CURI*    GetModuleOrigin(void) const;


    /*!
     * \brief Register an event listener (via EventMPI) and enable continuous sampleing mode
     * \param listener The event listener HWControllerMessage events will be sent to
     */
    LeapFrog::Brio::tErrType RegisterEventListener(const LeapFrog::Brio::IEventListener *listener);

    /*!
     * \brief Unregister event listener (via EventMPI) and siable all sampling
     * \param listener The event listener to unregister
     */
    LeapFrog::Brio::tErrType UnregisterEventListener(const LeapFrog::Brio::IEventListener *listener);

    /*!
     * \brief This method returns a controller object associated with the
     * specified controller id.  If no controller exists with the passed in
     * id, NULL is returned.  Due to controller being disconnected, the list of
     * connected controllers may not represent a contiguous list of controller ids.
     * \return The controller associated with the controller id, id
     */
    HWController* GetControllerByID(LeapFrog::Brio::U32 id = kHWDefaultControllerID);

    /*!
     * \brief This method is used to get all controllers that are currently on and
     * connected to the platform.
     * \param controllers output vector of all currently connected controllers this
     * vector will be cleared out and then filled.  Any data that is present in the
     * vector at the time of method execution will be removed.
     */
    void GetAllControllers(std::vector<HWController*> &controllers);

    /*!
     * \brief Returns the current number of controllers present and connected
     * to the system
     * \return The number of controllers currently connected to the LF device
     */
    LeapFrog::Brio::U8 GetNumberOfConnectedControllers(void) const;

    /*!
     * \brief begin or abort the controller sync/pairing operation
     * \param enable If true will start a sync operation, if false abort the sync operation in process
     */
    LeapFrog::Brio::tErrType EnableControllerSync(bool enable);

    /*!
     * \brief return the maximum number of simultaneously connected controllers
     */
    LeapFrog::Brio::U8 GetMaximumNumberOfControllers();

    /*!
     * \brief Issue an explicit disconnect to each controller currently connected
     */
    void DisconnectAllControllers();


  private:
    boost::shared_ptr<HWControllerMPIPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     */
    HWControllerMPI(const HWControllerMPI&);
    HWControllerMPI& operator =(const HWControllerMPI&);

  };

} // namespace Hardware
} // namespace LF

#endif // __INCLUDE_HARDWARE_HWCONTROLLERMPI_H__
