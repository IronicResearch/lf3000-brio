#ifndef __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
#define __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <opencv2/opencv.hpp>
#include <VNWandPIMPL.h>

namespace LF {
namespace Vision {

  class VNWandTrackerPIMPL {
  public:
    VNWandTrackerPIMPL(VNWandPIMPL* wand);
    virtual ~VNWandTrackerPIMPL(void);
    
    void Execute(cv::Mat &input, cv::Mat &output);
    
    VNWandPIMPL* wand_;
    cv::Mat hsv_;
    float minArea_;

  private:
    void ComputeLargestContour(cv::Mat& img, 
			       std::vector<std::vector<cv::Point> > &contours,
			       int &index);
    void FitCircleToContour(const std::vector<cv::Point> &contour,
			    cv::Point &center,
			    float &radius) const;

  };
}
}

#endif // __VISION_INCLUDE_VNWANDTRACKERPIMPL_H__
