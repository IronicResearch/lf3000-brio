#ifndef __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
#define __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <opencv2/opencv.hpp>
#include <VNWandPIMPL.h>
#include <VNTranslatorBase.h>

namespace LF {
namespace Vision {

  class VNWandTrackerPIMPL {
  public:
    VNWandTrackerPIMPL(VNWandPIMPL* wand,
		       VNInputParameters *params);
    virtual ~VNWandTrackerPIMPL(void);
    
    void Execute(cv::Mat &input, cv::Mat &output);
    
    void SetAutomaticWandScaling(bool autoScale);
    bool GetAutomaticWandScaling(void) const;

    VNWandPIMPL* wand_;
    cv::Mat hsv_;

  private:
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
  };
}
}

#endif // __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
