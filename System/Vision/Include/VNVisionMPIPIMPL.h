#ifndef __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
#define __VISION_INCLUDE_VNVISIONMPIPIMPL_H__

#include <VideoTypes.h>
#include <KernelTypes.h>
#include <CameraMPI.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <boost/timer.hpp>

namespace LF {
namespace Vision {

  class VNHotSpot;
  class VNAlgorithm;
  class VNWand;

  class VNVisionMPIPIMPL {
  public:
    static VNVisionMPIPIMPL* Instance(void);
    virtual ~VNVisionMPIPIMPL(void);
    
    void DeleteTask(void);
    
    void Start(LeapFrog::Brio::tVideoSurf& surf,
	       bool dispatchSynchronously);
    void Update(void);
    void Stop(void);
    void Pause(void);
    
    void TriggerHotSpots(void);
    
    void BeginFrameProcessing(void);
    void EndFrameProcessing(void) const;
    void Wait(double secondsToWait) const;
    void CreateRGBImage(LeapFrog::Brio::tVideoSurf &surf,
			cv::Mat &img) const;
    
    static void* CameraCaptureTask(void* args);

#ifdef EMULATION
    void OpenCVDebug(void);
#endif

    bool visionAlgorithmRunning_;
    float frameProcessingRate_;
    std::vector<const VNHotSpot*> hotSpots_;
    LeapFrog::Brio::tTaskHndl taskHndl_;
    VNAlgorithm* algorithm_;
    LeapFrog::Brio::tVideoSurf videoSurf_;
    LeapFrog::Brio::tVidCapHndl videoCapture_;
    LeapFrog::Brio::CCameraMPI  cameraMPI_;
    cv::Mat outputImg_;
    boost::timer timer_;

  private:
    VNVisionMPIPIMPL(void);
    VNVisionMPIPIMPL(const VNVisionMPIPIMPL&);
    VNVisionMPIPIMPL& operator=(const VNVisionMPIPIMPL&);
  };

}
}

#endif // __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
