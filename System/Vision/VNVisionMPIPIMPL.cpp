#include <VNVisionMPIPIMPL.h>
#include <KernelMPI.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
#include <DisplayTypes.h>
#include <Vision/VNWand.h>

namespace LF {
namespace Vision {

  static const double kVNMillisecondsInASecond = 1000.0;
  static const float kVNDefaultFrameProcessingRate = 0.03125; // 32 fps
  static const LeapFrog::Brio::U32 kVNDefaultStackSize = 134217728; // 128 MB in bytes

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
    taskHndl_(LeapFrog::Brio::kInvalidTaskHndl),
    algorithm_(NULL){
  }
  
  VNVisionMPIPIMPL::~VNVisionMPIPIMPL(void) {
    DeleteTask();
  }
  
  void
  VNVisionMPIPIMPL::DeleteTask(void) {
    cameraMPI_.StopVideoCapture(videoCapture_);
    visionAlgorithmRunning_ = false;
    if (taskHndl_ != LeapFrog::Brio::kInvalidTaskHndl) {
      LeapFrog::Brio::CKernelMPI kernel;
      kernel.CancelTask(taskHndl_);
    }
  }
  void
  VNVisionMPIPIMPL::Start(LeapFrog::Brio::tVideoSurf& surf,
			  bool dispatchSynchronously) {
    if (!visionAlgorithmRunning_) {
#ifdef EMULATION
      if (cameraMPI_.SetCurrentCamera(LeapFrog::Brio::kCameraDefault) == kNoErr) 
#else
      if (cameraMPI_.SetCurrentCamera(LeapFrog::Brio::kCameraFront) == kNoErr) 
#endif      
      {
	videoSurf_ = surf;
	LeapFrog::Brio::tCaptureMode* mode = cameraMPI_.GetCurrentFormat();
	mode->width = surf.width;
	mode->height = surf.height;
	cameraMPI_.SetCurrentFormat(mode);
	videoCapture_ = cameraMPI_.StartVideoCapture(&videoSurf_, NULL, "");
	visionAlgorithmRunning_ = true;

	if (dispatchSynchronously) {
	  LeapFrog::Brio::CKernelMPI kernel;
	  LeapFrog::Brio::tTaskProperties props;
	  props.TaskMainFcn = &VNVisionMPIPIMPL::CameraCaptureTask;
	  props.taskMainArgCount = 1;
	  props.pTaskMainArgValues = static_cast<LeapFrog::Brio::tPtr>(this);
	  props.stackSize = kVNDefaultStackSize;
	  kernel.CreateTask(taskHndl_, props, NULL);
	}
      }
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
      
      (*hs)->Trigger(outputImg_);
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
  VNVisionMPIPIMPL::CreateRGBImage(LeapFrog::Brio::tVideoSurf& surf,
				   cv::Mat& img) const {

    LeapFrog::Brio::U32 width = surf.width;
    LeapFrog::Brio::U32 height = surf.height;

    if (surf.format == LeapFrog::Brio::kPixelFormatYUV420) {
      

    } else if (surf.format == LeapFrog::Brio::kPixelFormatRGB888) {
      img = cv::Mat(cv::Size(width, height), CV_8UC3, surf.buffer);
    }
    
  }

  void
  VNVisionMPIPIMPL::Update(void) {
    if (visionAlgorithmRunning_) {
      BeginFrameProcessing();

      if (!cameraMPI_.IsVideoCapturePaused(videoCapture_) ) {

#ifdef EMULATION
	if (cameraMPI_.LockCaptureVideoSurface(videoCapture_)) {
	  LeapFrog::Brio::tVideoSurf *surf = cameraMPI_.GetCaptureVideoSurface(videoCapture_);
	  unsigned char *buffer = NULL;
	  if (surf)
	    buffer = surf->buffer;
#else
	if (cameraMPI_.GetFrame(videoCapture_, &videoSurf_)) {	  
	  unsigned char* buffer = videoSurf_.buffer;
#endif
	  if (buffer && algorithm_) {
	    cv::Mat img(cv::Size(videoSurf_.width, 
				 videoSurf_.height), 
			CV_8UC3, 
			buffer);
	    algorithm_->Execute(img, outputImg_);
	    TriggerHotSpots();
	  }
	}
#ifdef EMULATION
	cameraMPI_.UnLockCaptureVideoSurface(videoCapture_);
#endif
      }
      EndFrameProcessing();
    }
  }

  void*
  VNVisionMPIPIMPL::CameraCaptureTask(void* args) {
    VNVisionMPIPIMPL* me = static_cast<VNVisionMPIPIMPL*>(args);
    me->visionAlgorithmRunning_ = true;		

    while (true) {
      me->Update();
    }
    
    me->visionAlgorithmRunning_ = false;
    //TODO: investigate what we should return here.
    return NULL;
  }
} // namespace Vision
} // namespace LF
