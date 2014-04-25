#include <VNVisionMPIPIMPL.h>
#include <KernelMPI.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
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

namespace LF {
namespace Vision {

  static const double kVNMillisecondsInASecond = 1000.0;
  static const float kVNDefaultFrameProcessingRate = 0.03125; // 32 fps
  static const LeapFrog::Brio::U32 kVNDefaultStackSize = 134217728; // 128 MB in bytes
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
    dbg_(kGroupVision),
    visionAlgorithmRunning_(false),
    frameProcessingRate_(kVNDefaultFrameProcessingRate),
    taskHndl_(LeapFrog::Brio::kInvalidTaskHndl),
    algorithm_(NULL),
    frameTime_(time(0)),
    frameCount_(0),
    frameProcessingWidth_(kVNDefaultProcessingFrameWidth),
    frameProcessingHeight_(kVNDefaultProcessingFrameHeight),
    currentWand_(NULL) {

#if defined(EMULATION)
    showOCVDebugOutput_ = false;      
    if (FlagExists(kVNDebugOCVFlagPath.c_str()))
      showOCVDebugOutput_ = true;
#endif

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
    if (taskHndl_ != LeapFrog::Brio::kInvalidTaskHndl) {
      LeapFrog::Brio::CKernelMPI kernel;
      if (kernel.CancelTask(taskHndl_ != kNoErr))
	  result = static_cast<LeapFrog::Brio::Boolean>(false);	  
    }
    return result;
  }

  LeapFrog::Brio::tErrType
  VNVisionMPIPIMPL::SetCameraFormat(void) {
    LeapFrog::Brio::tCaptureModes modeList;
    LeapFrog::Brio::tErrType error = cameraMPI_.EnumFormats(modeList);
    
    if (error == kNoErr) {
      error = kVNCameraDoesNotSupportRequiredVisionFormat;
      std::cout << "Number of camera modes is " << modeList.size() << std::endl;
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

  LeapFrog::Brio::tErrType
  VNVisionMPIPIMPL::DispatchVisionThread(void) {
    LeapFrog::Brio::CKernelMPI kernel;
    LeapFrog::Brio::tTaskProperties props;
    props.TaskMainFcn = &VNVisionMPIPIMPL::CameraCaptureTask;
    props.taskMainArgCount = 1;
    props.pTaskMainArgValues = static_cast<LeapFrog::Brio::tPtr>(this);
    props.stackSize = kVNDefaultStackSize;
    return kernel.CreateTask(taskHndl_, props, NULL);
  }

  void
  VNVisionMPIPIMPL::UpdateHotSpotVisionCoordinates(void) {
    for (std::vector<const VNHotSpot*>::iterator it = hotSpots_.begin();
	 it != hotSpots_.end(); ++it) {
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
      std::cout << "Setting scaling based on dispayRect\n";
      displayFrame.left = displayRect->left;
      displayFrame.right = displayRect->right;
      displayFrame.top = displayRect->top;
      displayFrame.bottom = displayRect->bottom;
    } else if (videoSurf_) {
      std::cout << "Setting scaling based on videoSurf\n";
      displayFrame.right = videoSurf_->width;
      displayFrame.bottom = videoSurf_->height;
    } else {
      std::cout << "Setting scaling based on screen size\n";
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
			  bool dispatchSynchronously,
			  const LeapFrog::Brio::tRect *displayRect) {
    LeapFrog::Brio::tErrType error = kNoErr;

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
	  videoCapture_ = cameraMPI_.StartVideoCapture(videoSurf_);

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
	  
	    if (dispatchSynchronously) {
	      error = DispatchVisionThread();

	      if (error != kNoErr) {
		DeleteTask();
	      }
	    }
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
  
#define CLIP8BIT(v)    v > 255 ? 255 : v < 0 ? 0 : v
  
  void YUV2RGB(cv::Mat src, cv::Mat dst) {
    const int width = dst.cols;
    const int height = dst.rows;
    const int halfHeight = height / 2;
    const int pitch = src.step;
    const int halfPitch = pitch / 2;
    
    // Set all the plane starting points, UV lives out at the 2k boundry
    const unsigned char* yData = src.ptr();
    const unsigned char* uData = yData + halfPitch;
    const unsigned char* vData = uData + halfHeight * pitch;
    
    // Grab the output pointer
    unsigned char* rgbData = dst.ptr();
    
    // Each pair of 2 Y values share a single U & V
    for(int j = 0; j < height; ++j) {
      const unsigned char* y = yData + j * pitch;
      
      // U V rows advance half as fast (note integer divide by 2)
      const unsigned char* u = uData + (j / 2) * pitch;
      const unsigned char* v = vData + (j / 2) * pitch;
      
      // The output row
      unsigned char* c = rgbData + j * 3 * width;
      
      // Work on each pair of Ys (even & odd) at once
      for(int i = 0, k = 0; i < width; i += 2, ++k) {
	// U/V stays the same for each two pixels
	const short redUV = 1.13983f * (v[k] - 128);
	const short greenUV = - 0.39465 * (u[k] - 128) - 0.58060f * (v[k] - 128); 
	const short blueUV = 2.03211 * (u[k] - 128);
	
	for(int yi = 0; yi < 2; ++yi) {
	  *c++ = CLIP8BIT(*y + blueUV);
	  *c++ = CLIP8BIT(*y + greenUV);
	  *c++ = CLIP8BIT(*y + redUV);
	  ++y;
	}
      }
    }
  }
  
  static bool tableInited = false;
  static short rv[256];
  static short gu[256];
  static short gv[256];
  static short bu[256];
  
  void YUV2RGB_fast_table(cv::Mat src, cv::Mat dst) {
    if(!tableInited) {
      for(int i = 0; i < 256; ++i) {
	rv[i] = 1.13983f * (i - 128);
	gu[i] = - 0.39465 * (i - 128);
	gv[i] = - 0.58060f * (i - 128);
	bu[i] = 2.03211f * (i - 128);
      }
      tableInited = true;
    }
    const int width = dst.cols;
    const int tripleWidth = 3 * width;
    const int height = dst.rows;
    const int halfHeight = height / 2;
    const int pitch = src.step;
    const int halfPitch = pitch / 2;
    
    // Set all the plane starting points, UV lives out at the 2k boundry
    const unsigned char* yData = src.ptr();
    const unsigned char* uData = yData + halfPitch;
    const unsigned char* vData = yData + halfHeight * pitch + halfPitch;
    
    // Grab the output pointer
    unsigned char* rgbData = dst.ptr();
    
    // Each pair of 2 Y values share a single U & V
    for(int j = 0; j < height; ++j) {
      const unsigned char* y = yData + j * pitch;
      
      // U V rows advance half as fast (note integer divide by 2)
      const unsigned char* u = uData + (j / 2) * pitch;
      const unsigned char* v = vData + (j / 2) * pitch;
      
      // The output row
      unsigned char* c = rgbData + j * tripleWidth;
      
      const unsigned char* yEnd = y + width;
      
      // Work on the whole line of Y values
      while(y < yEnd) {
	// U/V stays the same for each two pixels
	const short redUV = rv[*v];
	const short greenUV = gu[*u] + gv[*v];
	const short blueUV = bu[*u];
	
	// Even Y
	*c++ = CLIP8BIT(*y + blueUV);
	*c++ = CLIP8BIT(*y + greenUV);
	*c++ = CLIP8BIT(*y + redUV);
	++y;
	
	// Odd Y
	*c++ = CLIP8BIT(*y + blueUV);
	*c++ = CLIP8BIT(*y + greenUV);
	*c++ = CLIP8BIT(*y + redUV);
	++y;
	
	++u;
	++v;
      }
    }
  }

  void
  VNVisionMPIPIMPL::CreateRGBImage(LeapFrog::Brio::tVideoSurf* surf,
				   cv::Mat& img) const {

    LeapFrog::Brio::U32 width = surf->width;
    LeapFrog::Brio::U32 height = surf->height;
    
    if (surf->format == LeapFrog::Brio::kPixelFormatYUV420) {
      cv::Mat tmp(cv::Size(width,
			   height),
		  CV_8UC2, 
		  surf->buffer,
		  surf->pitch); //4096); //FIXME: what is this magic number???
      YUV2RGB_fast_table(tmp, img);
      
    } else if (surf->format == LeapFrog::Brio::kPixelFormatRGB888) {
      img = cv::Mat(cv::Size(width, 
			     height), 
		    CV_8UC3, 
		    surf->buffer);
    } else if (surf->format == LeapFrog::Brio::kPixelFormatYUYV422) {
      cv::Mat tmp(cv::Size(width,
			   height),
		  CV_8UC2,
		  surf->buffer,
		  surf->pitch);
      cv::cvtColor(tmp, img, CV_YUV2RGB_YUYV);
    } else {
      //some other format
    }
  }

  void
  VNVisionMPIPIMPL::Update(void) {

    if (visionAlgorithmRunning_) {
      BeginFrameProcessing();

      if (!cameraMPI_.IsVideoCapturePaused(videoCapture_) ) {

	if (cameraMPI_.LockCaptureVideoSurface(videoCapture_)) {
	  LeapFrog::Brio::tVideoSurf *surf = cameraMPI_.GetCaptureVideoSurface(videoCapture_);
	  dbg_.Assert(surf != 0, "VNVisionMPIPIMPL::Update - failed getting video surface.\n");
	  if (!surf) return;
	  
	  unsigned char *buffer = surf->buffer;

	  dbg_.Assert(buffer && algorithm_, "VNVisionMPIPIMPL::Update - invalid buffer or algorithm.\n");
	  if (buffer && algorithm_) {
	    cv::Mat rgbMat(cv::Size(surf->width,
				    surf->height),
			   CV_8UC3);
	    CreateRGBImage(surf,
			   rgbMat);
#ifdef EMULATION
	    cv::namedWindow("input");
	    cv::imshow("input", rgbMat);
#endif
	    algorithm_->Execute(rgbMat, outputImg_);

#ifdef EMULATION
	    if (showOCVDebugOutput_)
	      OpenCVDebug();
#endif

	    TriggerHotSpots();
	    ++frameCount_;
	    if (frameCount_ % 30 == 0) {
	      std::cout << "FPS = " << frameCount_ / ((float)(time(0) - frameTime_)) << std::endl;
	    }
	  }
	}
	cameraMPI_.UnLockCaptureVideoSurface(videoCapture_);
      } else {
	dbg_.DebugOut(kDbgLvlCritical, "VNVisionMPIPIMPL - no frame.\n");
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

} // namespace Vision
} // namespace LF
