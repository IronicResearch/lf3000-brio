#ifndef __INCLUDE_VISION_VNVISIONMPI_H__
#define __INCLUDE_VISION_VNVISIONMPI_H__

#include <CoreMPI.h>
#include <VideoTypes.h>
#include <Vision/VNVisionTypes.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

	class VNHotSpot;
	class VNAlgorithm;

  /*!
   * \class VNVisionMPI
   * \brief
   * VNVisionMPI is the controller of all computer vision
   * related activities.  This includes setting the particular
   * vision algorithm and adding/removing hot spots.
   */
  class VNVisionMPIPIMPL;
  class VNVisionMPI : public LeapFrog::Brio::ICoreMPI {
  public:
    VNVisionMPI(void);
    virtual ~VNVisionMPI(void);

    virtual LeapFrog::Brio::Boolean        IsValid(void) const;
    virtual const LeapFrog::Brio::CString* GetMPIName(void) const;
    virtual LeapFrog::Brio::tVersion       GetModuleVersion(void) const;
    virtual const LeapFrog::Brio::CString* GetModuleName(void) const;
    virtual const LeapFrog::Brio::CURI*    GetModuleOrigin(void) const;

    /*!
     * Note: The application calling this method is responsible for the memory management
     * of the VNAlgorithm* passed in.  VNVisionMPI does not delete the pointer
     */
    void SetAlgorithm(VNAlgorithm* algorithm);
    VNAlgorithm* GetAlgorithm(void) const;

    /*!
     * Note: The application calling this method is responsible for the memory management
     * of the VNHotSpot* passed in.  VNVisionMPI does not delete the pointer.
     */
    void AddHotSpot(const VNHotSpot* hotSpot);
    void RemoveHotSpot(const VNHotSpot* hotSpot);

    void Start(LeapFrog::Brio::tVideoSurf& videoSurf);
    void Stop(void);
    void Pause(void);
    bool IsRunning(void) const;

  private:
    boost::shared_ptr<VNVisionMPIPIMPL> pimpl_;

    /*!
     * explicitly disable copy semantics
     */
    VNVisionMPI(const VNVisionMPI&);
    VNVisionMPI& operator=(const VNVisionMPI&);
  };

} // namespace Vision
} // namespace LF

#endif
