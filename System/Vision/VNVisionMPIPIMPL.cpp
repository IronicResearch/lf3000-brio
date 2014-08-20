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
  // LOCKING AND UNLOCKING 
#define HS_UPDATE_LOCK dbg_.Assert((kNoErr == kernelMPI_.LockMutex(hsUpdateLock_)), \
				    "Couldn't lock hot spot update mutex.\n");
#define HS_UPDATE_UNLOCK dbg_.Assert((kNoErr == kernelMPI_.UnlockMutex(hsUpdateLock_)),\
				     "Couldn't unlock hot spot update mutex.\n");

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
    videoCapture_(kInvalidVidCapHndl),
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

    // Setup mutexes
    const LeapFrog::Brio::tMutexAttr attr = {0};
    LeapFrog::Brio::tErrType err = kNoErr;
    err = kernelMPI_.InitMutex(hsUpdateLock_, attr);
    dbg_.Assert((err == kNoErr), "VNVisionMPIPIMPL::ctor: Could not init hot spot removal mutex.\n");

    dbg_.SetDebugLevel(kDbgLvlVerbose);
    dbg_.DebugOut(kDbgLvlImportant, "VNVisionMPI [ctor]\n");
  }

  VNVisionMPIPIMPL::~VNVisionMPIPIMPL(void) {
    DeleteTask();

    // delete mutexes
    kernelMPI_.DeInitMutex(hsUpdateLock_);

    dbg_.DebugOut(kDbgLvlImportant, "VNVisionMPI [dtor]\n");
  }

  LeapFrog::Brio::Boolean
  VNVisionMPIPIMPL::DeleteTask(void) {
    LeapFrog::Brio::Boolean result = static_cast<LeapFrog::Brio::Boolean>(true);
    visionAlgorithmRunning_ = false;

#if VN_USE_IMAGE_PROCESS_THREAD
    // Signal vision thread to exit
    if (hndlVisionThread_ != kInvalidTaskHndl) {
       isThreadRunning_ = false;
       void* retval = NULL;
       kernelMPI_.JoinTask(hndlVisionThread_, retval);
       hndlVisionThread_ = kInvalidTaskHndl;
    }
#endif

    // stop the video capture if it's still going
    if (videoCapture_ != kInvalidVidCapHndl) {
      result = cameraMPI_.StopVideoCapture(videoCapture_);
    }

    // if the algorithm is still present, shut it down
    if (algorithm_) {
      algorithm_->Shutdown();
    }

    // delete the memory for the surface buffer
    if (surface_.buffer) {
      delete surface_.buffer;
    }
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
	if (mode &&
	    mode->width  == frameProcessingWidth_ &&
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
      if (*it) {
	(*it)->pimpl_->UpdateVisionCoordinates();
      }
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
    if (translator) {
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
    if (videoCapture_ != kInvalidVidCapHndl) {
      result = cameraMPI_.PauseVideoCapture(videoCapture_);
      if (result) {
        visionAlgorithmRunning_ = false;
      }
    }
    return result;
  }

  LeapFrog::Brio::Boolean
  VNVisionMPIPIMPL::Resume(void) {
    LeapFrog::Brio::Boolean result = static_cast<LeapFrog::Brio::Boolean>(true);
    if (videoCapture_ != kInvalidVidCapHndl) {
      result = cameraMPI_.ResumeVideoCapture(videoCapture_);
      if (result) {
        visionAlgorithmRunning_ = true;
      }
    }
    return result;
  }

  void
  VNVisionMPIPIMPL::TriggerHotSpots(void) {
    std::vector<const VNHotSpot*> triggeredHotSpots;
    std::vector<const VNHotSpot*> changedHotSpots;

    //-----
    // compute integral image
    static cv::Mat integralImg;
#if VN_USE_FAST_INTEGRAL_IMAGE
    LF::Vision::IntegralImage(outputImg_, integralImg);
#else
    cv::integral(outputImg_, integralImg);
#endif
    VNHotSpotPIMPL::SetIntegralImage(&integralImg);
    //------

    //------
    // trigger hot spots
    bool wasTriggered = false;
    PROF_BLOCK_START("triggerHotSpots");
    HS_UPDATE_LOCK
    for (std::vector<const VNHotSpot*>::iterator hs = hotSpots_.begin();
	 hs != hotSpots_.end();
	 ++hs) {
      const VNHotSpot* hotSpot = *hs;
      if (hotSpot) {
	wasTriggered = hotSpot->IsTriggered();

	hotSpot->Trigger(outputImg_);

	// add hot spot to appropriate group of hot spots
	if (hotSpot->IsTriggered())
	  triggeredHotSpots.push_back(hotSpot);
	if (wasTriggered != hotSpot->IsTriggered())
	  changedHotSpots.push_back(hotSpot);
      }
    }
    //-----

    //-------
    // Send out group triggering notifications
    if (triggeredHotSpots.size() > 0) {
      VNHotSpotEventMessage msg(LF::Vision::kVNHotSpotGroupTriggeredEvent, triggeredHotSpots);
      eventMPI_.PostEvent(msg, 0);
    }

    if (changedHotSpots.size() > 0) {
      VNHotSpotEventMessage msg(LF::Vision::kVNHotSpotGroupTriggerChangeEvent, changedHotSpots);
      eventMPI_.PostEvent(msg, 0);
    }
    //------
    HS_UPDATE_UNLOCK
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
  void* VisionTask(void* pctx) {
      VNVisionMPIPIMPL* pthis = (VNVisionMPIPIMPL*)pctx;
      
      if (pthis) {
	while (pthis->isThreadRunning_) {
	  
	  if (pthis->visionAlgorithmRunning_ && pthis->algorithm_ && pthis->isFramePending_) {
	    PROF_BLOCK_START("Vision::Task");
	    
	    PROF_BLOCK_START("algorithm");
	    pthis->algorithm_->Execute(pthis->cameraSurfaceMat_, pthis->outputImg_);
	    PROF_BLOCK_END();

	    pthis->isFramePending_ = false;
	    
#if defined(EMULATION)
	    pthis->OpenCVDebug();
#endif
	    PROF_BLOCK_START("triggering");
	    pthis->TriggerHotSpots();
	    PROF_BLOCK_END();
	    
	    PROF_BLOCK_END();
	    
	    ++pthis->frameCount_;
	    if (pthis->frameCount_ % 30 == 0) {
	      std::cout << "FPS = " << pthis->frameCount_ / ((float)(time(0) - pthis->frameTime_)) << std::endl;
	    }
	  } else {
	    pthis->kernelMPI_.TaskSleep(1); // yield
	  }
	}
      }
      pthis->visionAlgorithmRunning_ = false;
      return pctx; // optional
  }
#endif

  LeapFrog::Brio::tEventStatus
  VNVisionMPIPIMPL::Notify(const LeapFrog::Brio::IEventMessage &msg) {

#if VN_USE_IMAGE_PROCESS_THREAD
    // If the vision thread does not process the previous frame fast enough
    // this will drop the next frame from the camera
    if (isFramePending_) {
      return LeapFrog::Brio::kEventStatusOK;
    } 
#endif

    if (visionAlgorithmRunning_ && algorithm_) {
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
	      
	      PROF_BLOCK_START("triggering");
	      TriggerHotSpots();
	      PROF_BLOCK_END();

	      ++frameCount_;
	      if (frameCount_ % 30 == 0) {
		std::cout << "FPS = " << frameCount_ / ((float)(time(0) - frameTime_)) << std::endl;
	      }
#endif
	      //EndFrameProcessing();
	    }
	    return LeapFrog::Brio::kEventStatusOKConsumed;
	  }
	  PROF_BLOCK_END();
	  return LeapFrog::Brio::kEventStatusOKConsumed;
	}
      } else {
	std::cout << "DID NOT successfully cast the msg" << std::endl;
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
  VNVisionMPIPIMPL::AddHotSpot(const VNHotSpot* hotSpot) {
    // avoid adding NULL
    if (hotSpot) {
      if (std::find(hotSpots_.begin(),
		    hotSpots_.end(),
		    hotSpot) == (hotSpots_.end())) {

	HS_UPDATE_LOCK
	hotSpots_.push_back(hotSpot);
	HS_UPDATE_UNLOCK
      }
    }
  }

  void
  VNVisionMPIPIMPL::RemoveHotSpot(const VNHotSpot* hotSpot) {
    // skip NULL
    if (hotSpot) {
      HS_UPDATE_LOCK
      // find where the hot spot is int he array and erase it
      std::vector<const VNHotSpot*>::iterator it;
      it = std::find(hotSpots_.begin(), hotSpots_.end(), hotSpot);
      if (it != hotSpots_.end()) {
	hotSpots_.erase(it);
      }
      HS_UPDATE_UNLOCK
    }
  }

  void
  VNVisionMPIPIMPL::RemoveHotSpotByID(const LeapFrog::Brio::U32 tag) {
    std::vector<const VNHotSpot*>::iterator it = hotSpots_.begin();
    HS_UPDATE_LOCK
    for ( ; it != hotSpots_.end(); ++it) {
      if ((*it) && (*it)->GetTag() == tag) {
	hotSpots_.erase(it);
	//decrement since it now points to the position past the one erased and 
	// the update branch of the for loop will increment the iterator
	--it; 
      }
    }
    HS_UPDATE_UNLOCK
  }

  void
  VNVisionMPIPIMPL::RemoveAllHotSpots(void) {
    HS_UPDATE_LOCK
    hotSpots_.clear();
    HS_UPDATE_UNLOCK
  }

} // namespace Vision
} // namespace LF
