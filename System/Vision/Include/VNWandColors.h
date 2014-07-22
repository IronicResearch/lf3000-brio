#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  // NOTE: These values refer to the min YUV and max YUV are
  // are used in conjunction with the inRange method in VNInRagen3.cpp
  
  // green LED
  static const cv::Scalar kVNWandGreenMin(1, 184, 144);
  static const cv::Scalar kVNWandGreenMax(255, 254, 214);

  // red LED
  static const cv::Scalar kVNWandRedMin(1, 32, 56);
  static const cv::Scalar kVNWandRedMax(255, 102, 126);

  // blue LED
  static const cv::Scalar kVNWandBlueMin(1, 36, 178);
  static const cv::Scalar kVNWandBlueMax(255, 106, 248);

  // yellow LED
  static const cv::Scalar kVNWandYellowMin(40, 8, 17);
  static const cv::Scalar kVNWandYellowMax(255, 78, 87);

  // cyan LED
  static const cv::Scalar kVNWandCyanMin(1, 0, 173);
  static const cv::Scalar kVNWandCyanMax(255, 70, 243);

  // magenta LED
  static const cv::Scalar kVNWandMagentaMin(1, 37, 38);
  static const cv::Scalar kVNWandMagentaMax(255, 107, 108);

} // namespace Vision
} // namespace LF
