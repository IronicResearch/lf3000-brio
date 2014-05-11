#ifndef __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
#define __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <opencv2/opencv.hpp>
#include <VNTranslatorBase.h>
#include <CameraTypes.h>

namespace LF {
namespace Vision {
  class VNWand;

  class VNWandTrackerPIMPL {
  public:
    VNWandTrackerPIMPL(VNWand* wand,
		       VNInputParameters *params);
    virtual ~VNWandTrackerPIMPL(void);
    
    virtual void Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
			    LeapFrog::Brio::U16 frameProcessingHeight);
    void Execute(cv::Mat &input, cv::Mat &output);
    
    void SetAutomaticWandScaling(bool autoScale);
    bool GetAutomaticWandScaling(void) const;

    void SetWand(VNWand *wand);
    cv::Mat hsv_;

  private:
    VNWand* wand_;
    bool scaleInput_;
    cv::Rect subFrame_;
    VNTranslatorBase translator_;
    float wandAreaToStartScaling_;
    float minPercentToScale_;
    float minArea_;

    void SetParams(VNInputParameters *params);
    void ComputeLargestContour(cv::Mat& img, 
			       std::vector<std::vector<cv::Point> > &contours,
			       int &index);
    void FitCircleToContour(const std::vector<cv::Point> &contour,
			    cv::Point &center,
			    float &radius) const;
    void ScaleSubFrame(const cv::Mat &input,
		       const cv::Point &p,
		       const float radius);
    bool ScaleWandPoint(cv::Point &p) const;
    void SetProcessingFrameSize(LeapFrog::Brio::U16 width,
				LeapFrog::Brio::U16 height);
	void ConvertToRGB(const cv::Mat& in, cv::Mat& outrgb);
  };
}
}

#endif // __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
