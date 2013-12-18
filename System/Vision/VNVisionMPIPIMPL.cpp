#include <VNVisionMPIPIMPL.h>
#include <KernelMPI.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
#include <Vision/VNWand.h>

namespace LF {
namespace Vision {

  static const double kVNMillisecondsInASecond = 1000.0;
  static const float kVNDefaultFrameProcessingRate = 0.03125; // 32 fps
  
  VNVisionMPIPIMPL*
  VNVisionMPIPIMPL::Instance(void) {
    static VNVisionMPIPIMPL* sharedInstance = NULL;
    if (sharedInstance == NULL) {
      sharedInstance = new VNVisionMPIPIMPL();
    }
    return sharedInstance;
  }
  
  VNVisionMPIPIMPL::VNVisionMPIPIMPL(void) :
    visionAlgorithmRunning_(false),
    frameProcessingRate_(kVNDefaultFrameProcessingRate),
    algorithm_(NULL){
  }
  
  VNVisionMPIPIMPL::~VNVisionMPIPIMPL(void) {
    DeleteTask();
  }
  
  void
  VNVisionMPIPIMPL::DeleteTask(void) {
    visionAlgorithmRunning_ = false;
  }
  void
  VNVisionMPIPIMPL::Start(LeapFrog::Brio::tVideoSurf& surf) {
    if (!visionAlgorithmRunning_) {
      visionAlgorithmRunning_ = true;
      videoSurf_ = surf;
      videoCapture_ = cameraMPI_.StartVideoCapture(&videoSurf_, NULL, "");      
    }
  }
  
  void
  VNVisionMPIPIMPL::Stop(void) {
    DeleteTask();
  }
  
  void
  VNVisionMPIPIMPL::Pause(void) {
    visionAlgorithmRunning_ = false;
  }
  
  void
  VNVisionMPIPIMPL::TriggerHotSpots(void) {
    for (std::vector<const VNHotSpot*>::iterator hs = hotSpots_.begin();
	 hs != hotSpots_.end();
	 ++hs) {
      
      (*hs)->Trigger(&outputImg_);
    }
  }
  
  void
  VNVisionMPIPIMPL::Wait(double secondsToWait) const {
    boost::timer timer;
    do {
      // nothing
    }while(timer.elapsed() < secondsToWait);
  }
  
  void
  VNVisionMPIPIMPL::BeginFrameProcessing(void) {
    timer_.restart();
  }
  
  void
  VNVisionMPIPIMPL::EndFrameProcessing(void) const {
    double elapsedTimeInSecs = timer_.elapsed();
    if (elapsedTimeInSecs < frameProcessingRate_) {
      Wait(frameProcessingRate_-elapsedTimeInSecs);
    }
  }
  
  void
  VNVisionMPIPIMPL::Update(void) {

    bool ret = false;
    
    BeginFrameProcessing();

    if (visionAlgorithmRunning_) {
      if (!cameraMPI_.IsVideoCapturePaused(videoCapture_) ) {
	if (cameraMPI_.LockCaptureVideoSurface(videoCapture_)) {
	  // now do something with the buffer
	  LeapFrog::Brio::tVideoSurf* captureSurface = cameraMPI_.GetCaptureVideoSurface( videoCapture_ );
	  
	  if (captureSurface) {
	    unsigned char* buffer = captureSurface->buffer;
	    
	    if (algorithm_) {
	      cv::Mat img(cv::Size(captureSurface->width,captureSurface->height), CV_8UC3, buffer);
	      algorithm_->Execute(&img, &outputImg_);
	      TriggerHotSpots();
	    }
	    ret = cameraMPI_.UnLockCaptureVideoSurface( videoCapture_);
	  }
	}
      }
    }
    EndFrameProcessing();
  }

} // namespace Vision
} // namespace LF
