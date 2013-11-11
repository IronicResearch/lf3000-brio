#ifndef __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
#define __VISION_INCLUDE_VNVISIONMPIPIMPL_H__

#include <VideoTypes.h>
#include <KernelTypes.h>
#include <CameraMPI.h>
#include <vector>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

	class VNHotSpot;
	class VNAlgorithm;
	class VNVisionMPIPIMPL {
	public:
		static VNVisionMPIPIMPL* Instance(void);
		virtual ~VNVisionMPIPIMPL(void);

		void DeleteTask(void);

		void Start(LeapFrog::Brio::tVideoSurf& surf);
		void Stop(void);
		void Pause(void);

		void TriggerHotSpots(void);

		static void* CameraCaptureTask(void* arg);

		bool visionAlgorithmRunning_;
		LeapFrog::Brio::tTaskHndl taskHndl_;
		std::vector<const VNHotSpot*> hotSpots_;
		VNAlgorithm* algorithm_;
		LeapFrog::Brio::tVideoSurf videoSurf_;
		LeapFrog::Brio::tVidCapHndl videoCapture_;
		LeapFrog::Brio::CCameraMPI cameraMPI_;
		cv::Mat outputImg_;

	private:
		VNVisionMPIPIMPL(void);
		VNVisionMPIPIMPL(const VNVisionMPIPIMPL&);
		VNVisionMPIPIMPL& operator=(const VNVisionMPIPIMPL&);
	};

}
}

#endif // __VISION_INCLUDE_VNVISIONMPIPIMPL_H__
