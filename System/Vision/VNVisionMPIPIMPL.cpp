#include <VNVisionMPIPIMPL.h>
#include <KernelMPI.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>
#include <DisplayTypes.h>
#include <DisplayMPI.h>
#include <Vision/VNWand.h>
#include <VNHotSpotPIMPL.h>
#include <VNCoordinateTranslator.h>
#ifdef EMULATION
#include <opencv2/highgui/highgui.hpp>
#endif

#include <DisplayTypes.h>
#include <GroupEnumeration.h>
#include <KernelMPI.h>
#include <Utility.h>

namespace LF {
namespace Vision {

  static const double kVNMillisecondsInASecond = 1000.0;
  static const float kVNDefaultFrameProcessingRate = 0.03125; // 32 fps
  static const LeapFrog::Brio::U32 kVNDefaultStackSize = 134217728; // 128 MB in bytes
  static const LeapFrog::Brio::U16 kVNVisionProcessingFrameWidth = 640;
  static const LeapFrog::Brio::U16 kVNVisionProcessingFrameHeight = 480;


  VNVisionMPIPIMPL*
  VNVisionMPIPIMPL::Instance(void) {
    static VNVisionMPIPIMPL* sharedInstance = NULL;
    if (sharedInstance == NULL) {
      sharedInstance = new VNVisionMPIPIMPL();
    }
    return sharedInstance;
  }
  
  VNVisionMPIPIMPL::VNVisionMPIPIMPL(void) :
    dbg_(kGroupVision),
    visionAlgorithmRunning_(false),
    frameProcessingRate_(kVNDefaultFrameProcessingRate),
    taskHndl_(LeapFrog::Brio::kInvalidTaskHndl),
    algorithm_(NULL),
    frameTime_(time(0)),
    frameCount_(0) {

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
      for (tCaptureModes::iterator it = modeList.begin(); it != modeList.end(); ++it) {
	LeapFrog::Brio::tCaptureMode* mode = *it;

	if (mode->width  == kVNVisionProcessingFrameWidth &&
	    mode->height == kVNVisionProcessingFrameHeight) {

	  LeapFrog::Brio::tCaptureFormat useFormat = LeapFrog::Brio::kCaptureFormatError;
	  if (GetPlatformName() == "CABO") {
	    useFormat = LeapFrog::Brio::kCaptureFormatYUV420;
	  } else if (GetPlatformName() == "GLASGOW") {
	    useFormat = LeapFrog::Brio::kCaptureFormatRAWYUYV;
	  }
	  
	  mode->pixelformat = useFormat;
	  // Glasgow requires framerate setting
	  mode->fps_numerator = 1;
	  mode->fps_denominator = 30;

	  cameraMPI_.SetCurrentFormat(mode);
	  error = kNoErr;
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

  void
  VNVisionMPIPIMPL::SetCameraControls(void) {
    if(0) {
      tCameraControls controls;
      
      dbg_.Assert(cameraMPI_.GetCameraControls(controls), "VNVision could get camera controls\n");
      
      bool setExposure = false;
      bool setWhiteBlaance = false;
      
      for(tCameraControls::iterator i = controls.begin();
	  i != controls.end();
	  ++i) {
	tControlInfo* c = *i;
	
	if(!c) {
	  dbg_.DebugOut(kDbgLvlCritical, "null camera control (tControlInfo)\n");
	  continue;
	} else {
	  printf("Control type: %i\n", c->type);
	}
	
	switch(c->type) {
	case kControlTypeAutoWhiteBalance:
	  printf("AutoWhiteBalance: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	  cameraMPI_.SetCameraControl(c, 0);
	  printf("New AutoWhiteBalance: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	  break;
	case kControlTypeAutoExposure:
	  printf("New Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	  cameraMPI_.SetCameraControl(c, 0);
	  printf("New Auto Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	  break;
	case kControlTypeExposure:
	  printf("Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	  cameraMPI_.SetCameraControl(c, 9000);
	  printf("New Exposure: %li %li %li %li\n", c->min, c->max, c->preset, c->current);
	  break;
	}
      }
    }
  }

  LeapFrog::Brio::tErrType
  VNVisionMPIPIMPL::SetCurrentCamera(void) {
    LeapFrog::Brio::tCameraDevice useCamera = LeapFrog::Brio::kCameraDefault;
#ifdef EMULATION
    // do nothing
#else
    if(GetPlatformName() == "CABO") {
      useCamera = LeapFrog::Brio::kCameraFront;
    } 
#endif 
    if (useCamera == LeapFrog::Brio::kCameraDefault)
      dbg_.DebugOut(kDbgLvlValuable, "VNVisionMPIPIMPL: selected default camera\n");
    else if (useCamera == LeapFrog::Brio::kCameraFront)
      dbg_.DebugOut(kDbgLvlValuable, "VNVisionMPIPIMPL: selected front camera\n");
    
    return cameraMPI_.SetCurrentCamera(useCamera);
  }

  void
  VNVisionMPIPIMPL::SetCoordinateTranslatorFrames(void) {
    VNCoordinateTranslator *translator = VNCoordinateTranslator::Instance();
    LeapFrog::Brio::tRect visionFrame;
    visionFrame.left = visionFrame.top = 0;
    visionFrame.right = kVNVisionProcessingFrameWidth;
    visionFrame.bottom = kVNVisionProcessingFrameHeight;
    translator->SetVisionFrame(visionFrame);
    
    LeapFrog::Brio::tRect displayFrame;
    displayFrame.left = displayFrame.top = 0;
    if (videoSurf_) {
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
  VNVisionMPIPIMPL::Start(LeapFrog::Brio::tVideoSurf* surf,
			  bool dispatchSynchronously) {
    LeapFrog::Brio::tErrType error = kNoErr;
    if (!visionAlgorithmRunning_) {
      frameCount_ = 0;
      frameTime_ = time(0);
      
      SetCameraControls();
      
      error = SetCurrentCamera();
       
       if (error == kNoErr) {
	videoSurf_ = surf;

	SetCoordinateTranslatorFrames();
	error = SetCameraFormat();

	if (error == kNoErr) {
	  videoCapture_ = cameraMPI_.StartVideoCapture(videoSurf_);

	  if (videoCapture_ == LeapFrog::Brio::kInvalidVidCapHndl) {
	    error = kVNVideoCaptureFailed;
	    dbg_.DebugOut(kDbgLvlCritical, "Failed to start vision video capture\n");
	  } else {
	    dbg_.DebugOut(kDbgLvlImportant, "VNVision [started video capture.\n");

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
	  *c++ = CLIP8BIT(*y + redUV);
	  *c++ = CLIP8BIT(*y + greenUV);
	  *c++ = CLIP8BIT(*y + blueUV);
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
	*c++ = CLIP8BIT(*y + redUV);
	*c++ = CLIP8BIT(*y + greenUV);
	*c++ = CLIP8BIT(*y + blueUV);
	++y;
	
	// Odd Y
	*c++ = CLIP8BIT(*y + redUV);
	*c++ = CLIP8BIT(*y + greenUV);
	*c++ = CLIP8BIT(*y + blueUV);
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
		  CV_8U, 
		  surf->buffer,
		  surf->pitch); //4096); //FIXME: what is this magic number???
      YUV2RGB_fast_table(tmp, img);
      
    } else if (surf->format == LeapFrog::Brio::kPixelFormatRGB888) {
      img = cv::Mat(cv::Size(width, 
			     height), 
		    CV_8UC3, 
		    surf->buffer);
    } else if (surf->format == LeapFrog::Brio::kPixelFormatYUYV422) {
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
	    algorithm_->Execute(rgbMat, outputImg_);

#ifdef EMULATION
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
