#ifndef __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
#define __VISION_INCLUDE_VNVISIONMPIPIMPL_H__

#include <VideoTypes.h>
#include <KernelTypes.h>
#include <CameraMPI.h>
#include <DebugMPI.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <Vision/VNVisionTypes.h>

#include <vector>
#include <opencv2/opencv.hpp>
#include <boost/timer.hpp>

namespace LF {
namespace Vision {

  static const LeapFrog::Brio::U16 kVNDefaultProcessingFrameWidth = kVNVGAWidth;
  static const LeapFrog::Brio::U16 kVNDefaultProcessingFrameHeight = kVNVGAHeight;

  void* VisionTask(void* pctx);

  class VNHotSpot;
  class VNAlgorithm;
  class VNWand;

  class VNVisionMPIPIMPL : public LeapFrog::Brio::IEventListener {
  public:
    static VNVisionMPIPIMPL* Instance(void);
    virtual ~VNVisionMPIPIMPL(void);

    LeapFrog::Brio::Boolean DeleteTask(void);

    LeapFrog::Brio::tErrType Start(LeapFrog::Brio::tVideoSurf *surf,
				   bool dispatchSynchronously,
				   const LeapFrog::Brio::tRect *displayRect);

    LeapFrog::Brio::Boolean Stop(void);
    LeapFrog::Brio::Boolean Pause(void);
    LeapFrog::Brio::Boolean Resume(void);

    void TriggerHotSpots(void);
    void SetCurrentWand(VNWand *wand);
    VNWand* GetCurrentWand(void) const;

    void AddHotSpot(const VNHotSpot* hotSpot);
    void RemoveHotSpot(const VNHotSpot* hotSpot);
    void RemoveHotSpotByID(const LeapFrog::Brio::U32 tag);
    void RemoveAllHotSpots(void);

    bool visionAlgorithmRunning_;
    float frameProcessingRate_;
    VNAlgorithm* algorithm_;
    LeapFrog::Brio::tVideoSurf* videoSurf_;
    LeapFrog::Brio::tVidCapHndl videoCapture_;
    LeapFrog::Brio::CCameraMPI  cameraMPI_;
    cv::Mat outputImg_;
    boost::timer timer_;

    virtual LeapFrog::Brio::tEventStatus Notify(const LeapFrog::Brio::IEventMessage& msg);

  protected:
    LeapFrog::Brio::tErrType SetCameraFormat(void);
    void SetCoordinateTranslatorFrames(const LeapFrog::Brio::tRect *displayRect);
    LeapFrog::Brio::tErrType SetCurrentCamera(void);
    void BeginFrameProcessing(void);
    void EndFrameProcessing(void) const;
    void Wait(double secondsToWait) const;
    void UpdateHotSpotVisionCoordinates(void);
    void SetFrameProcessingSize(void);

#ifdef EMULATION
    void OpenCVDebug(void);
#endif

  private:
    VNVisionMPIPIMPL(void);
    VNVisionMPIPIMPL(const VNVisionMPIPIMPL&);
    VNVisionMPIPIMPL& operator=(const VNVisionMPIPIMPL&);

    LeapFrog::Brio::CDebugMPI dbg_;
    LeapFrog::Brio::CEventMPI eventMPI_;

    // all hot spots
    std::vector<const VNHotSpot*> hotSpots_;

    //-------
    // used for updating hot spots (adding/removing)
    enum VNHotSpotUpdateKey {
      VN_ADD_HOT_SPOT,
      VN_REMOVE_HOT_SPOT
    };
    typedef std::pair<VNHotSpotUpdateKey, const VNHotSpot*> VNHotSpotUpdate;
    std::vector<VNHotSpotUpdate> hotSpotsToUpdate_;
    //------

    //mutexes
    LeapFrog::Brio::tMutex hsUpdateLock_;

    LeapFrog::Brio::U16 frameProcessingWidth_;
    LeapFrog::Brio::U16 frameProcessingHeight_;
    VNWand *currentWand_;

    // camera surface
    LeapFrog::Brio::CKernelMPI kernelMPI_;
    LeapFrog::Brio::tVideoSurf surface_;
    cv::Mat cameraSurfaceMat_;

    //Cause the VNVisionMPIPIMPL to instantiate during static construction
    static VNVisionMPIPIMPL* forceVNVisionMPIMPLToBe_;

#if defined(EMULATION)
    bool showOCVDebugOutput_;
#endif

    friend void* VisionTask(void*);
    volatile bool isFramePending_; 
    volatile bool isThreadRunning_;
    LeapFrog::Brio::tTaskHndl hndlVisionThread_;

  };

}
}

#endif // __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
