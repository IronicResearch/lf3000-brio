#ifndef __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
#define __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <opencv2/opencv.hpp>
#include <VNTranslatorBase.h>
#include <CameraTypes.h>
#include <Vision/VNVisionMPI.h>
#include <list>

namespace LF {
namespace Vision {
  class VNWand;

  class VNWandTrackerPIMPL {
  public:
    VNWandTrackerPIMPL(VNWand* wand,
		       VNInputParameters *params);
    virtual ~VNWandTrackerPIMPL(void);

    void Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
		    LeapFrog::Brio::U16 frameProcessingHeight);
    void Execute(cv::Mat &input, cv::Mat &output);
    void Shutdown(void);

    void SetAutomaticWandScaling(bool autoScale);
    bool GetAutomaticWandScaling(void) const;

  private:
    VNVisionMPI visionMPI_;
    VNWand* wand_;
    bool scaleInput_;
    cv::Rect subFrame_;
    VNTranslatorBase translator_;
    float wandAreaToStartScaling_;
    float minPercentToScale_;
    float minArea_;
    cv::Mat integral_;
    cv::Mat rgb_;
    cv::Mat hsv_;
    cv::Mat yuv_;
    cv::Point prevLoc_;
    int numTimesUsedCachedLoc_;
    int numTimesUsedCachedLocBeforeReset_;
    LeapFrog::Brio::U8 wandSmoothingAlpha_;
    bool useWandSmoothing_;
    std::list<cv::Point> wandHistory_;

    void SetParams(VNInputParameters *params);
    void ComputeLargestContour(cv::Mat& img,
			       std::vector<std::vector<cv::Point> > &contours,
			       int &index);
    void FitCircleToContour(const std::vector<cv::Point> &contour,
			    cv::Point &center,
			    float &radius) const;
    void ScaleSubFrame(const cv::Mat &input,
		       const cv::Point &p,
		       const float area);
    bool ScaleWandPoint(cv::Point &p) const;
    void SetProcessingFrameSize(LeapFrog::Brio::U16 width,
				LeapFrog::Brio::U16 height);
    void ConvertToRGB(const cv::Mat& in, cv::Mat& outrgb);
    void ConvertToYUV(const cv::Mat& in, cv::Mat& outrgb);
    float FindLight(const cv::Mat &integral, cv::Point &c);
    int integralSum(const cv::Mat &integral, cv::Rect &roi);
    cv::Point ComputeWandLocation(cv::Point& p);
  };
}
}

#endif // __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
