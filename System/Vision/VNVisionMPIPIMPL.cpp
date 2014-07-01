#include <VNVisionMPIPIMPL.h>
#include <KernelMPI.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNCircleHotSpot.h>
#include <Vision/VNArbitraryShapeHotSpot.h>
#include <DisplayTypes.h>
#include <DisplayMPI.h>
#include <Vision/VNWand.h>
#include <VNHotSpotPIMPL.h>
#include <Vision/VNWandTracker.h>
#include <VNWandTrackerPIMPL.h>
#include <VNCoordinateTranslator.h>
#ifdef EMULATION
#include <opencv2/highgui/highgui.hpp>
#endif
#include <DisplayTypes.h>
#include <GroupEnumeration.h>
#include <KernelMPI.h>
#include <Utility.h>
#include <sys/stat.h>
#undef VN_PROFILE
#include <VNProfiler.h>
#include <VNIntegralImage.h>
#include <Vision/VNVisionTypes.h>
#include <Vision/VNHotSpotEventMessage.h>

#define VN_USE_FAST_INTEGRAL_IMAGE 1
#define VN_USE_IMAGE_PROCESS_THREAD 1

const LeapFrog::Brio::tEventType gVNEventKey[] = {LeapFrog::Brio::kCaptureFrameEvent};

namespace LF {
namespace Vision {

  static const double kVNMillisecondsInASecond = 1000.0;
  static const float kVNDefaultFrameProcessingRate = 0.03125; // 32 fps
  static const LeapFrog::Brio::U32 kVNDefaultBufferSize = 2*kVNDefaultProcessingFrameWidth*kVNDefaultProcessingFrameHeight;
  static const std::string kVNQVGAFlagPath = "/flags/qvga_vision_mode";
  static const std::string kVNDebugOCVFlagPath = "/flags/showocv";

  bool
  FlagExists(const char *fileName) {
    struct stat buf;
    // checking for file status, will return 0 if file exists and all is well
    // returns -1 if file does not exist or there is an error getting it's stats
    // simple means to test for file existence
    int i = stat(fileName, &buf);
    if (i == 0) return true;
    return false;
  }

  VNVisionMPIPIMPL*
  VNVisionMPIPIMPL::Instance(void) {
    static VNVisionMPIPIMPL* sharedInstance = NULL;
    if (sharedInstance == NULL) {
      sharedInstance = new VNVisionMPIPIMPL();
    }
    return sharedInstance;
  }

  void
  VNVisionMPIPIMPL::SetCurrentWand(VNWand *wand) {
    currentWand_ = wand;
    VNWandTracker *wt = dynamic_cast<VNWandTracker*>(algorithm_);
    if (wt) {
      wt->pimpl_->SetWand(wand);
    }
  }

  VNWand*
  VNVisionMPIPIMPL::GetCurrentWand(void) const {
    return currentWand_;
  }

  void
  VNVisionMPIPIMPL::SetFrameProcessingSize(void) {
    // check for qvga flag and set size base don this
    if (FlagExists(kVNQVGAFlagPath.c_str())) {
      frameProcessingWidth_ = kVNQVGAWidth;
      frameProcessingHeight_ = kVNQVGAHeight;
    } else {
      frameProcessingWidth_ = kVNVGAWidth;
      frameProcessingHeight_ = kVNVGAHeight;
    }
  }

  VNVisionMPIPIMPL::VNVisionMPIPIMPL(void) :
    IEventListener(gVNEventKey, 5),
    dbg_(kGroupVision),
    visionAlgorithmRunning_(false),
    frameProcessingRate_(kVNDefaultFrameProcessingRate),
    algorithm_(NULL),
    frameTime_(time(0)),
    frameCount_(0),
    frameProcessingWidth_(kVNDefaultProcessingFrameWidth),
    frameProcessingHeight_(kVNDefaultProcessingFrameHeight),
    currentWand_(NULL),
    isFramePending_(false),
    isThreadRunning_(false),
    hndlVisionThread_(kInvalidTaskHndl) {

#if defined(EMULATION)
    showOCVDebugOutput_ = false;
    if (FlagExists(kVNDebugOCVFlagPath.c_str()))
      showOCVDebugOutput_ = true;
#endif

    surface_.width = 0;
    surface_.height = 0;
    surface_.pitch = 0;
    surface_.buffer = NULL;
    surface_.format = LeapFrog::Brio::kPixelFormatYUYV422; // initialize to something

    SetFrameProcessingSize();

    dbg_.SetDebugLevel(kDbgLvlVerbose);
    dbg_.DebugOut(kDbgLvlImportant, "VNVisionMPI [ctor]\n");
  }

  VNVisionMPIPIMPL::~VNVisionMPIPIMPL(void) {
    DeleteTask();
    dbg_.DebugOut(kDbgLvlImportant, "VNVisionMPI [dtor]\n");
  }

  LeapFrog::Brio::Boolean
  VNVisionMPIPIMPL::DeleteTask(void) {
    LeapFrog::Brio::Boolean result = static_cast<LeapFrog::Brio::Boolean>(true);
    if (videoCapture_ != kInvalidVidCapHndl)
      result = cameraMPI_.StopVideoCapture(videoCapture_);
    visionAlgorithmRunning_ = false;

#if VN_USE_IMAGE_PROCESS_THREAD
    // Signal vision thread to exit
    if (hndlVisionThread_ != kInvalidTaskHndl) {
       void* retval = NULL;
       isThreadRunning_ = false;
       kernelMPI_.JoinTask(hndlVisionThread_, retval);
       hndlVisionThread_ = kInvalidTaskHndl;
    }
#endif

    if (surface_.buffer) delete surface_.buffer;
    surface_.buffer = NULL;
return result;
  }

  LeapFrog::Brio::tErrType
  VNVisionMPIPIMPL::SetCameraFormat(void) {
    LeapFrog::Brio::tCaptureModes modeList;
    LeapFrog::Brio::tErrType error = cameraMPI_.EnumFormats(modeList);

    if (error == kNoErr) {
      error = kVNCameraDoesNotSupportRequiredVisionFormat;
      for (tCaptureModes::iterator it = modeList.begin(); it != modeList.end(); ++it) {
	LeapFrog::Brio::tCaptureMode* mode = *it;

	// Glasgow camera only has three settings
	if (mode->width  == frameProcessingWidth_ &&
	    mode->height == frameProcessingHeight_) {

	  // the pixel format supported by the glasgow camera is RAWYUYV
	  mode->pixelformat = LeapFrog::Brio::kCaptureFormatRAWYUYV;
	  
	  mode->fps_numerator = 1;
	  // if we are in VGA mode set the fps to 30, if in QVGA mode set it to 60
	  mode->fps_denominator = (frameProcessingWidth_ == kVNVGAWidth) ? 30 : 60;

	  std::cout << "Setting capture mode to:\n"
		    << "    pixelformat = " << mode->pixelformat << std::endl
		    << "          width = " << mode->width << std::endl
		    << "         height = " << mode->height << std::endl
		    << "  fps_numerator = " << mode->fps_numerator << std::endl
		    << "fps_denominator = " << mode->fps_denominator << std::endl;
	  error = cameraMPI_.SetCurrentFormat(mode);
	  break;
	}
      }
    }
    return error;
  }

  void
  VNVisionMPIPIMPL::UpdateHotSpotVisionCoordinates(void) {
    for (std::vector<const VNHotSpot*>::iterator it = hotSpots_.begin();
	 it != hotSpots_.end(); ++it) {
      (*it)->pimpl_->UpdateVisionCoordinates();
    }
    for (std::vector<const VNHotSpot*>::iterator it = rectHotSpots_.begin();
	 it != rectHotSpots_.end(); ++it) {
      (*it)->pimpl_->UpdateVisionCoordinates();
    }
  }

  LeapFrog::Brio::tErrType
  VNVisionMPIPIMPL::SetCurrentCamera(void) {
    LeapFrog::Brio::tCameraDevice useCamera = LeapFrog::Brio::kCameraDefault;
    if(GetPlatformName() == "CABO") {
      useCamera = LeapFrog::Brio::kCameraFront;
    }

    if (useCamera == LeapFrog::Brio::kCameraDefault)
      dbg_.DebugOut(kDbgLvlValuable, "VNVisionMPIPIMPL: selected default camera\n");
    else if (useCamera == LeapFrog::Brio::kCameraFront)
      dbg_.DebugOut(kDbgLvlValuable, "VNVisionMPIPIMPL: selected front camera\n");

    return cameraMPI_.SetCurrentCamera(useCamera);
  }

  void
  VNVisionMPIPIMPL::SetCoordinateTranslatorFrames(const LeapFrog::Brio::tRect *displayRect) {
    VNCoordinateTranslator *translator = VNCoordinateTranslator::Instance();
    LeapFrog::Brio::tRect visionFrame;
    visionFrame.left = visionFrame.top = 0;
    visionFrame.right = frameProcessingWidth_;
    visionFrame.bottom = frameProcessingHeight_;
    translator->SetVisionFrame(visionFrame);

    //initialize frame
    LeapFrog::Brio::tRect displayFrame;
    displayFrame.left = 0;
    displayFrame.top = 0;
    displayFrame.right = 0;
    displayFrame.bottom = 0;

    if (displayRect) {
      displayFrame.left = displayRect->left;
      displayFrame.right = displayRect->right;
      displayFrame.top = displayRect->top;
      displayFrame.bottom = displayRect->bottom;
    } else if (videoSurf_) {
      displayFrame.right = videoSurf_->width;
      displayFrame.bottom = videoSurf_->height;
    } else {
      const LeapFrog::Brio::tDisplayScreenStats*
	screenStats = LeapFrog::Brio::CDisplayMPI().GetScreenStats(0);
      if (screenStats) {
	displayFrame.right = screenStats->width;
	displayFrame.bottom = screenStats->height;
      }
    }
    translator->SetDisplayFrame(displayFrame);
    UpdateHotSpotVisionCoordinates();
  }

  LeapFrog::Brio::tErrType
  VNVisionMPIPIMPL::Start(LeapFrog::Brio::tVideoSurf *surf,
			  bool dispatchSynchronously, // DEPRECATED...WILL REMOVE
			  const LeapFrog::Brio::tRect *displayRect) {
    LeapFrog::Brio::tErrType error = kNoErr;

    // allocate buffers
    surface_.buffer = new LeapFrog::Brio::U8[kVNDefaultBufferSize];

    if (!visionAlgorithmRunning_) {
      frameCount_ = 0;
      frameTime_ = time(0);

      // make sure the surface size is at least as big as the processing size
      // TODO: This may/should go away once we resolve issues around different
      // sized between processing and display
      if (surf &&
	  (surf->width < frameProcessingWidth_ ||
	   surf->height < frameProcessingHeight_)) {
	return kVNVideoSurfaceNotOfCorrectSizeForVisionCapture;
      }

       if (error == kNoErr) {
	videoSurf_ = surf;

	SetCoordinateTranslatorFrames(displayRect);
	error = SetCameraFormat();

	if (error == kNoErr) {
	  videoCapture_ = cameraMPI_.StartVideoCapture(videoSurf_, this);

	  if (videoCapture_ == LeapFrog::Brio::kInvalidVidCapHndl) {
	    error = kVNVideoCaptureFailed;
	    dbg_.DebugOut(kDbgLvlCritical, "Failed to start vision video capture\n");
	  } else {
	    dbg_.DebugOut(kDbgLvlImportant, "VNVision [started video capture.\n");

	    // this initialization needs to happen after starting the video capture
	    // as some algorithms, like the wand tracking, adjust camera controls that
	    // can only be set once the camera is up and running
	    if (algorithm_)
	      algorithm_->Initialize(frameProcessingWidth_,
				     frameProcessingHeight_);

	    visionAlgorithmRunning_ = true;

#if VN_USE_IMAGE_PROCESS_THREAD
	    // Create separate vision processing thread to decouple from camera & event threads
	    tTaskProperties	prop;
	    prop.TaskMainFcn        = VisionTask;
	    prop.taskMainArgCount   = 1;
	    prop.pTaskMainArgValues = this;
	    prop.stackSize          = 4194304;
	    isThreadRunning_        = true;
	    kernelMPI_.CreateTask( hndlVisionThread_, prop, NULL );
#endif
	  }
	}
      }
    } else {
      dbg_.DebugOut(kDbgLvlCritical, "Failed to set current camera\n");
    }
    return error;
  }

  LeapFrog::Brio::Boolean
  VNVisionMPIPIMPL::Stop(void) {
    return DeleteTask();
  }

  LeapFrog::Brio::Boolean
  VNVisionMPIPIMPL::Pause(void) {
    LeapFrog::Brio::Boolean result = static_cast<LeapFrog::Brio::Boolean>(true);
    if (videoCapture_ != kInvalidVidCapHndl)
      result = cameraMPI_.PauseVideoCapture(videoCapture_);
    visionAlgorithmRunning_ = false;
    return result;
  }

  LeapFrog::Brio::Boolean
  VNVisionMPIPIMPL::Resume(void) {
    LeapFrog::Brio::Boolean result = static_cast<LeapFrog::Brio::Boolean>(true);
    if (videoCapture_ != kInvalidVidCapHndl)
      result = cameraMPI_.ResumeVideoCapture(videoCapture_);
    visionAlgorithmRunning_ = true;
    return result;
  }

  void
  VNVisionMPIPIMPL::TriggerHotSpots(void) {
    std::vector<const VNHotSpot*> triggeredHotSpots;
    std::vector<const VNHotSpot*> changedHotSpots;

    bool wasTriggered = false;
    PROF_BLOCK_START("triggerHotSpots");
    for (std::vector<const VNHotSpot*>::iterator hs = hotSpots_.begin();
	 hs != hotSpots_.end();
	 ++hs) {
      const VNHotSpot* hotSpot = *hs;
      wasTriggered = hotSpot->IsTriggered();

      hotSpot->Trigger(outputImg_);
    
      // add hot spot to appropriate group of hot spots
      if (hotSpot->IsTriggered())
	triggeredHotSpots.push_back(hotSpot);
      if (wasTriggered != hotSpot->IsTriggered())
	changedHotSpots.push_back(hotSpot);
    }
    PROF_BLOCK_END();

    if (rectHotSpots_.size() > 0) {
      PROF_BLOCK_START("integralImage");
      // compute integral image
      static cv::Mat integralImg;

#if VN_USE_FAST_INTEGRAL_IMAGE
	  LF::Vision::IntegralImage(outputImg_, integralImg);
#else
      cv::integral(outputImg_, integralImg);
#endif

      VNHotSpotPIMPL::SetIntegralImage(&integralImg);
      PROF_BLOCK_END();

      PROF_BLOCK_START("triggerRectHotSpots");
      for (std::vector<const VNHotSpot*>::iterator rhs = rectHotSpots_.begin();
	   rhs != rectHotSpots_.end();
	   ++rhs) {

	const VNHotSpot* hotSpot = *rhs;
	wasTriggered = hotSpot->IsTriggered();

	hotSpot->Trigger(outputImg_);

	// add hot spot to appropriate group of hot spots
	if (hotSpot->IsTriggered())
	  triggeredHotSpots.push_back(hotSpot);
	if (wasTriggered != hotSpot->IsTriggered())
	  changedHotSpots.push_back(hotSpot);
      }
      PROF_BLOCK_END();
    }
    VNHotSpotPIMPL::SetIntegralImage(NULL);

    if (triggeredHotSpots.size() > 0) {
      VNHotSpotEventMessage msg(LF::Vision::kVNHotSpotGroupTriggeredEvent, triggeredHotSpots);
      eventMPI_.PostEvent(msg, 0);
    }

    if (changedHotSpots.size() > 0) {
      VNHotSpotEventMessage msg(LF::Vision::kVNHotSpotGroupTriggerChangeEvent, changedHotSpots);
      eventMPI_.PostEvent(msg, 0);
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

#if VN_USE_IMAGE_PROCESS_THREAD
  void* VisionTask(void* pctx)
  {
	VNVisionMPIPIMPL* pthis = (VNVisionMPIPIMPL*)pctx;

	while (pthis->isThreadRunning_) {

	  if (pthis->isFramePending_) {
	      PROF_BLOCK_START("Vision::Task");

	      PROF_BLOCK_START("algorithm");
	      pthis->algorithm_->Execute(pthis->cameraSurfaceMat_, pthis->outputImg_);
	      PROF_BLOCK_END();

	      pthis->isFramePending_ = false;

#if defined(EMULATION)
	      pthis->OpenCVDebug();
#endif
	      
	      pthis->TriggerHotSpots();

	      PROF_BLOCK_END();
	      
	      ++pthis->frameCount_;
	      if (pthis->frameCount_ % 30 == 0) {
	          std::cout << "FPS = " << pthis->frameCount_ / ((float)(time(0) - pthis->frameTime_)) << std::endl;
	      }
	  } else {
	      pthis->kernelMPI_.TaskSleep(1); // yield
	  }

	}
	
	return pctx; // optional
  }
#endif

  LeapFrog::Brio::tEventStatus
  VNVisionMPIPIMPL::Notify(const LeapFrog::Brio::IEventMessage &msg) {
    if (algorithm_) {
      const LeapFrog::Brio::CCameraEventMessage *cemsg =
	dynamic_cast<const LeapFrog::Brio::CCameraEventMessage*>(&msg);
      if (cemsg) {
	if (cemsg->GetEventType() == LeapFrog::Brio::kCaptureFrameEvent) {
	  
	  LeapFrog::Brio::tCaptureFrameMsg frameMsg = cemsg->data.framed;
	  LeapFrog::Brio::tVideoSurf *surf = cameraMPI_.GetCaptureVideoSurface(frameMsg.vhndl);
	  
	  if (surf) {
	    unsigned char *buffer = surf->buffer;
	    if (buffer) {
	      //BeginFrameProcessing();
	      
	      PROF_BLOCK_START("Vision::Update");
	      
	      surface_.width  = surf->width;
	      surface_.height = surf->height;
	      surface_.pitch  = surf->pitch;
	      surface_.format = surf->format;
	      memcpy(surface_.buffer,
		     buffer,
		     surface_.height * surface_.pitch);
	      
	      
	      // create a camera surface cv::Mat
	      switch(surf->format) {
	      case LeapFrog::Brio::kPixelFormatRGB888:
		cameraSurfaceMat_ = cv::Mat(cv::Size(surface_.width, surface_.height),
					    CV_8UC3,
					    surface_.buffer);
		break;
	      case LeapFrog::Brio::kPixelFormatYUYV422:
		cameraSurfaceMat_ = cv::Mat(cv::Size(surface_.width, surface_.height),
					    CV_8UC2,
					    surface_.buffer,
					    surface_.pitch);
		break;
	      default:
		assert( !"unsupported surface format" );
	      }
	      
#if VN_USE_IMAGE_PROCESS_THREAD
	      // Signal vision thread to process frame
	      isFramePending_ = true;
#else
	      PROF_BLOCK_START("algorithm");
	      algorithm_->Execute(cameraSurfaceMat_, outputImg_);
	      PROF_BLOCK_END();

#if defined(EMULATION)
	      OpenCVDebug();
#endif
	      
	      TriggerHotSpots();
	      
	      PROF_BLOCK_END();
	      
	      ++frameCount_;
	      if (frameCount_ % 30 == 0) {
		std::cout << "FPS = " << frameCount_ / ((float)(time(0) - frameTime_)) << std::endl;
	      }
#endif
	      //EndFrameProcessing();
	    }
	  }
	  return LeapFrog::Brio::kEventStatusOKConsumed;
	}
      } else {
	std::cout << "DID NOT successfully cast the msg\n";
      }
    }
    return LeapFrog::Brio::kEventStatusOK;
  }

#ifdef EMULATION
  void
  VNVisionMPIPIMPL::OpenCVDebug(void) {
    //TODO: read from flag whether to show opencv debug
    if (outputImg_.cols > 0 && outputImg_.rows > 0) {
      cv::namedWindow("OpenCV Debug");
      cv::imshow("OpenCV Debug", outputImg_);
      // the 1 indicates to wait for 1 millisecond
      // if we do not wait for key events inside of the opencv window
      // the window does not get displayed.
      cv::waitKey(1);
    }
  }
#endif

  void
  VNVisionMPIPIMPL::AddHotSpot(const VNHotSpot* hotSpot,
			       std::vector<const VNHotSpot*> & hotSpots) {
    if (std::find(hotSpots.begin(),
		  hotSpots.end(),
		  hotSpot) == (hotSpots.end())) {
      hotSpots.push_back(hotSpot);
    }
  }

  void
  VNVisionMPIPIMPL::AddHotSpot(const VNHotSpot* hotSpot) {
    const VNRectHotSpot* rhs = dynamic_cast<const VNRectHotSpot*>(hotSpot);
    const VNCircleHotSpot *chs = dynamic_cast<const VNCircleHotSpot*>(rhs);
    const VNArbitraryShapeHotSpot *ahs = dynamic_cast<const VNArbitraryShapeHotSpot*>(rhs);
    // only add VNRectHotSpot not any children of VNRectHotSpot
    if (rhs && (chs == NULL) && (ahs == NULL)) {
      AddHotSpot(hotSpot, rectHotSpots_);
    } else {
      AddHotSpot(hotSpot, hotSpots_);
    }
  }

  bool
  VNVisionMPIPIMPL::RemoveHotSpot(const VNHotSpot *hotSpot,
				  std::vector<const VNHotSpot*> &hotSpots) {
    std::vector<const VNHotSpot*>::iterator it;
    it = std::find(hotSpots.begin(), hotSpots.end(), hotSpot);
    if (it != hotSpots.end()) {
      hotSpots.erase(it);
    }
  }

  void
  VNVisionMPIPIMPL::RemoveHotSpot(const VNHotSpot* hotSpot) {
    if (!RemoveHotSpot(hotSpot, hotSpots_)) {
      RemoveHotSpot(hotSpot, rectHotSpots_);
    }
  }

  void
  VNVisionMPIPIMPL::RemoveHotSpotByID(const LeapFrog::Brio::U32 tag,
				      std::vector<const VNHotSpot*> &hotSpots) {
    std::vector<const VNHotSpot*>::iterator it = hotSpots.begin();
    for ( ; it != hotSpots.end(); ++it) {
      if ((*it)->GetTag() == tag) {
	it = hotSpots.erase(it);
	// because erase returns an iterator to the "next" spot we need
	// to decrement the iterator since we icrement it at the end of each loop
	it--;
      }
    }
  }

  void
  VNVisionMPIPIMPL::RemoveHotSpotByID(const LeapFrog::Brio::U32 tag) {
    RemoveHotSpotByID(tag, hotSpots_);
    RemoveHotSpotByID(tag, rectHotSpots_);
  }

  void
  VNVisionMPIPIMPL::RemoveAllHotSpots(void) {
    rectHotSpots_.clear();
    hotSpots_.clear();
  }

} // namespace Vision
} // namespace LF
