#include <VNVisionMPIPIMPL.h>
#include <KernelMPI.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNAlgorithm.h>
#include <Vision/VNHotSpot.h>

namespace LF {
namespace Vision {

	static const int kVNNumFramesBeforeTestingForHotSpotTriggering = 600;

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
		taskHndl_(LeapFrog::Brio::kInvalidTaskHndl),
		algorithm_(NULL){
	}

	VNVisionMPIPIMPL::~VNVisionMPIPIMPL(void) {
		DeleteTask();
	}

	void
	VNVisionMPIPIMPL::DeleteTask(void) {
		if (taskHndl_ != LeapFrog::Brio::kInvalidTaskHndl) {
			LeapFrog::Brio::CKernelMPI kernel;
			kernel.CancelTask(taskHndl_);
			visionAlgorithmRunning_ = false;
		}
	}
	void
	VNVisionMPIPIMPL::Start(LeapFrog::Brio::tVideoSurf& surf) {
		if (taskHndl_ != LeapFrog::Brio::kInvalidTaskHndl) {
			visionAlgorithmRunning_ = true;
		} else {
			videoSurf_ = surf;
			videoCapture_ = cameraMPI_.StartVideoCapture(&videoSurf_, NULL, "");

			LeapFrog::Brio::CKernelMPI kernel;
			LeapFrog::Brio::tTaskProperties props;
			props.TaskMainFcn = &VNVisionMPIPIMPL::CameraCaptureTask;
			props.taskMainArgCount = 1;
			props.pTaskMainArgValues = static_cast<LeapFrog::Brio::tPtr>(this);
			kernel.CreateTask(taskHndl_, props, NULL);
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

	void*
	VNVisionMPIPIMPL::CameraCaptureTask(void *arg) {
		static int numCalls = 0;
		VNVisionMPIPIMPL* me = static_cast<VNVisionMPIPIMPL*>(arg);

		me->visionAlgorithmRunning_ = true;

		bool ret = false;

		while (true) {

			if (me->visionAlgorithmRunning_) {
				if (!me->cameraMPI_.IsVideoCapturePaused(me->videoCapture_) ) {

					ret = me->cameraMPI_.LockCaptureVideoSurface(me->videoCapture_);

					if (ret) {

						// now do something with the buffer
						LeapFrog::Brio::tVideoSurf* captureSurface = me->cameraMPI_.GetCaptureVideoSurface( me->videoCapture_ );

						if (captureSurface) {
							unsigned char* buffer = captureSurface->buffer;

							if (me->algorithm_) {
								cv::Mat img(cv::Size(captureSurface->width,captureSurface->height), CV_8UC3, buffer);
								me->algorithm_->Execute(&img, &me->outputImg_);
								if (numCalls > kVNNumFramesBeforeTestingForHotSpotTriggering)
									me->TriggerHotSpots();
								else
									numCalls++;
							}
							ret = me->cameraMPI_.UnLockCaptureVideoSurface( me->videoCapture_);

						}
					}
				}
			}
		}
		//TODO: investigate what we should return from this method
		return NULL;
	}
} // namespace Vision
} // namespace LF
