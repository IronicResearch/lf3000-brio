#ifndef __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
#define __VISION_INCLUDE_VNVISIONMPIPIMPL_H__

#include <VideoTypes.h>
#include <KernelTypes.h>
#include <CameraMPI.h>
#include <DebugMPI.h>

#include <vector>
#include <opencv2/opencv.hpp>
#include <boost/timer.hpp>

namespace LF {
namespace Vision {

  static const LeapFrog::Brio::U16 kVNVisionProcessingFrameWidth = 640;
  static const LeapFrog::Brio::U16 kVNVisionProcessingFrameHeight = 480;  

  class VNHotSpot;
  class VNAlgorithm;
  class VNWand;

  class VNVisionMPIPIMPL {
  public:
    static VNVisionMPIPIMPL* Instance(void);
    virtual ~VNVisionMPIPIMPL(void);
    
    LeapFrog::Brio::Boolean DeleteTask(void);
    
    LeapFrog::Brio::tErrType Start(LeapFrog::Brio::tVideoSurf* surf,
				   bool dispatchSynchronously);
    void Update(void);
    LeapFrog::Brio::Boolean Stop(void);
    LeapFrog::Brio::Boolean Pause(void);
    LeapFrog::Brio::Boolean Resume(void);
    
    void TriggerHotSpots(void);
    
    bool visionAlgorithmRunning_;
    float frameProcessingRate_;
    std::vector<const VNHotSpot*> hotSpots_;
    LeapFrog::Brio::tTaskHndl taskHndl_;
    VNAlgorithm* algorithm_;
    LeapFrog::Brio::tVideoSurf* videoSurf_;
    LeapFrog::Brio::tVidCapHndl videoCapture_;
    LeapFrog::Brio::CCameraMPI  cameraMPI_;
    cv::Mat outputImg_;
    boost::timer timer_;

  protected:
    LeapFrog::Brio::tErrType SetCameraFormat(void);
    LeapFrog::Brio::tErrType DispatchVisionThread(void);
    void SetCoordinateTranslatorFrames(void);
    LeapFrog::Brio::tErrType SetCurrentCamera(void);
    void BeginFrameProcessing(void);
    void EndFrameProcessing(void) const;
    void Wait(double secondsToWait) const;
    void CreateRGBImage(LeapFrog::Brio::tVideoSurf *surf,
			cv::Mat &img) const;
    void UpdateHotSpotVisionCoordinates(void);

    static void* CameraCaptureTask(void* args);

#ifdef EMULATION
    void OpenCVDebug(void);
#endif

  private:
    VNVisionMPIPIMPL(void);
    VNVisionMPIPIMPL(const VNVisionMPIPIMPL&);
    VNVisionMPIPIMPL& operator=(const VNVisionMPIPIMPL&);
    
    LF_ADD_BRIO_NAMESPACE(CDebugMPI) dbg_;
    
    time_t frameTime_;
    int frameCount_;
  };

}
}

#endif // __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
