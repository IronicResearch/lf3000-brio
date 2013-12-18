#ifndef __INCLUDE_VISION_VNVISIONMPI_H__
#define __INCLUDE_VISION_VNVISIONMPI_H__

#include <CoreMPI.h>
#include <VideoTypes.h>
#include <Vision/VNVisionTypes.h>
//#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declaration
  class VNHotSpot;
  class VNAlgorithm;
  class VNWand;

  /*!
   * \class VNVisionMPI
   *
   * \brief VNVisionMPI is the controller of all computer vision related activities.  This includes 
   * setting the particular vision algorithm and adding/removing hot spots.
   * The algorithm is run on a separate thread, and when active, will copy framebuffers from the camera
   * and process them.  The thread is active after the first call to \sa {Start}, and image processing
   * continues until \sa {Pause} or \sa {Stop} is called.  In between calls to \sa {Pause} and \sa {Start} the 
   * thread is active, using compute cycles, just not processing images.  A call to \sa {Stop} will destroy
   * the thread and stop all image processing.  
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
     * GetWandByID
     * \breif With no parameter, this method will return the default wand
     * \param id the unique identifier of the desired wand.
     * \return A ponter to the VNWand object associated with the id
     */
    VNWand* GetWandByID(LeapFrog::Brio::U32 id = kVNDefaultWandID) const;

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
     * \param videoSurf the video surface from which the camera capture is taken
     */
    void Start(LeapFrog::Brio::tVideoSurf& videoSurf);

    /*!
     * \breif Update performs one iteration of the current algorithm allowing
     * the image processing to occur.  This should be called in the CGameState
     * Update loop/
     */
    void Update(void);

    /*!
     * \brief Stop terminates image processing and destroys the current thread
     */
    void Stop(void);

    /*!
     * \brief Pause will pause the current thread from processing images.  This call does
     * not destroy the current thread and therefore the thread is still alive and active
     * it's just not processing the framebuffers
     */
    void Pause(void);

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
  };

} // namespace Vision
} // namespace LF

#endif
