#ifndef __INCLUDE_VISION_VNVISIONMPI_H__
#define __INCLUDE_VISION_VNVISIONMPI_H__

#include <CoreMPI.h>
#include <VideoTypes.h>
#include <Vision/VNVisionTypes.h>
//#include <boost/shared_ptr.hpp>

namespace LF {

namespace Hardware {
  class HWControllerPIMPL;
}

namespace Vision {

  // forward declaration
  class VNHotSpot;
  class VNAlgorithm;
  class VNWandTracker;
  class VNHotSpotPIMPL;
  class VNWandTrackerPIMPL;
  class VNWand;

  /*!
   * \class VNVisionMPI
   *
   * NOTE: For use with LeapTV applications ONLY.
   *
   * \brief VNVisionMPI is the controller of all computer vision related activities.  This includes
   * setting the particular vision algorithm and adding/removing hot spots.
   * The algorithm is run on a separate thread, and when active, will copy framebuffers from the camera
   * and process them.  The thread is active after the first call to \sa {Start}, and image processing
   * continues until \sa {Pause} or \sa {Stop} is called.  In between calls to \sa {Pause} and \sa {Start} the
   * thread is active, using compute cycles, just not processing images.  A call to \sa {Stop} will destroy
   * the thread and stop all image processing.
   *
   * To turn on OpenCV debug output in emulation follow these steps on your development machine:
   *  1) sudo mkdir /flags
   *  2) sudo touch /flags/showocv
   *  3) run you app as normal
   * By adding this flag VNVisionMPI will display both the input image to the vision algorithm as well
   * as the image used to trigger hot spots.  If you'd like to turn this off in emulation then simply
   * remove the file /flags/showocv
   *
   * It is possible to enable the use of qVGA mode for vision processing as a means to more rapidly test
   * out game mechanics that use vision.  To do this you must add the qvga_vision_mode flag to the flags
   * directory.  Keep in mind, this reduction in resolution will decrease the fidelity of the vision
   * algorithm output and this potentially have a negative impact on the game itself.  The intention of
   * this flag is for use during on device debugging.
   */
  class VNVisionMPIPIMPL;
  class VNVisionMPI : public LeapFrog::Brio::ICoreMPI {
  public:

    /*!
     * \brief Constructor
     */
    VNVisionMPI(void);

    /*!
     * \brief Destructor
     */
    virtual ~VNVisionMPI(void);

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
     * \brief SetAlgorithm allows the developer to set what computer vision algorithm
     * the mpi should use. The application calling this method is responsible for the
     * memory management of the VNAlgorithm* passed in.  VNVisionMPI does not delete the pointer
     * \param algorithm the specific VNAlgorithm to use
     */
    void SetAlgorithm(VNAlgorithm* algorithm);

    /*!
     * \brief GetAlgorithm returns the current algorithm
     * \return A pointer to the current VNAlgorithm
     */
    VNAlgorithm* GetAlgorithm(void) const;

    /*!
     * DEPRECATED - this method only returns NULL and is begin removed in
     * an upcomgin release.
     * GetWandByID
     * \breif With no parameter, this method will return the default wand
     * \param id the unique identifier of the desired wand.
     * \return A ponter to the VNWand object associated with the id
     */
    VNWand* GetWandByID(LeapFrog::Brio::U32 id = 0) const;

    /*!
     * \brief AddHotSpot adds a hot spot to the mpi for tracking/triggering.
     * The application calling this method is responsible for the memory management
     * of the VNHotSpot* passed in.  VNVisionMPI does not delete the pointer.
     * \param hotSpot A Pointer to the VNHotSpot object being added
     */
    void AddHotSpot(const VNHotSpot* hotSpot);

    /*!
     * \brief RemoveHotSpot removes the specific hotSpot from the mpi
     * \param hotSpot A pointer to the hot spot to remove
     */
    void RemoveHotSpot(const VNHotSpot* hotSpot);

    /*!
     * \brief RemoveHotSpotByID removes the hot spot, or hot spots, that match the
     * id passed in to thie mehtod
     * \param tag the identification tag used to match existing hot spots
     */
    void RemoveHotSpotByID(const LeapFrog::Brio::U32 tag);

    /*!
     * \brief RemoveAllHotSpots removes all hot spots
     */
    void RemoveAllHotSpots(void);

    /*!
     * \brief Start begins the video capture process necessary for video processing
     * \param videoSurf Is an optional parameter.  If a video surface is passed in the
     * video capture will be displayed to this surfae
     * \param dispatchSynchronously DEPRECATED This input parameter is now deprecated as it
     * is no longer necessary or adventageous to launch an asynchronous vision update.  In future
     * releases of the API/SDK the signature of this method will change to reflect this.
     * the VNVisionMPI::Update method once per state update.
     * \param displayRect an optional parameter that specifies the display rectangle the application
     * code intends to use.  If pass in, the vision library will use this as the basis for
     * scaling between the vision processing coordinate system and the display coordinate
     * system.  If this is not passed in VNVisionMPI will use the full screen resolution
     * as the display frame to scale to.
     * \return kNoErr if started successfully, the appropriate error otherwise
     */
    LeapFrog::Brio::tErrType Start(LeapFrog::Brio::tVideoSurf* videoSurf = NULL,
				   bool dispatchSynchronously = false, /* DEPRECATED */
				   const LeapFrog::Brio::tRect *displayRect = NULL);

    /*!
     * **DEPRECATED**
     * This method is now deprecated and is a no-op if called.  In a future relase of the SDK
     * this method signature will be removed from the API.
     */
    void Update(void);

    /*!
     * \brief Stop terminates image processing and destroys the current thread, if one exists
     * \return true if successful
     */
    LeapFrog::Brio::Boolean Stop(void);

    /*!
     * \brief Pause will pause the vision processing video capture. If Start was called
     * with dispatchSynchronously set to true this call will not destroy the current
     * thread and therefore the thread is still alive and active it's just not processing
     * the framebuffers
     * \return true if successful
     */
    LeapFrog::Brio::Boolean Pause(void);

    /*!
     * \brief Resume will resume the video capture process.  If the vision processing is
     * in it's own thread, the thread will now start processing images again.
     * \return true if successful
     */
    LeapFrog::Brio::Boolean Resume(void);

    /*!
     * \brief IsRunning
     * \return true if the thread is processing images and false otherwise
     */
    bool IsRunning(void) const;

    /*!
     * \brief SetFrameProcessingRate Acts as a frame limiter.  Only grabs frames to process at
     * 1.0/frameProcessingRate frames per second
     * The default is 32 frames per second, frameProcessingRate = 0.03125
     * \param frameProcessingRate the maximum rate at which the algorithm will process frames
     */
    void SetFrameProcessingRate(float frameProcessingRate);

    /*!
     * \brief GetFrameProcessingRate
     * \return the rate at which the algorithm will process frames
     */
    float GetFrameProcessingRate(void) const;

  private:
    VNVisionMPIPIMPL* pimpl_;

    /*!
     * explicitly disable copy semantics
     */
    VNVisionMPI(const VNVisionMPI&);
    VNVisionMPI& operator=(const VNVisionMPI&);

    friend class VNWandTracker;
    friend class VNHotSpotPIMPL;
    friend class VNWandTrackerPIMPL;
    friend class LF::Hardware::HWControllerPIMPL;
  };

} // namespace Vision
} // namespace LF

#endif
