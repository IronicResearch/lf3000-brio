#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  //TODO: Need to perform further adjustments on these colors and it may
  // depend on environmental factors

  //NOTE: if using opencv RGB-->HSV the Hue values are in the range
  // of 0-180, however, the NEON and C-based implementation in
  // VNRGB2HSV.cpp are in the range 0-255.  This is only imporatant
  // if debuggin with OpenCV as both the emulator and device conversions
  // use the methods in VNRGB2HSV.cpp

  // green LED
  static const cv::Scalar kVNWandGreenMin(71, 0, 77);
  static const cv::Scalar kVNWandGreenMax(107, 255, 255);

  // red LED
  static const cv::Scalar kVNWandRedMin(0, 0, 77);
  static const cv::Scalar kVNWandRedMax(36, 255, 255);

  // blue LED
  static const cv::Scalar kVNWandBlueMin(141, 0, 77);
  static const cv::Scalar kVNWandBlueMax(177, 255, 255);

  // yellow LED
  static const cv::Scalar kVNWandYellowMin(0, 0, 77);
  static const cv::Scalar kVNWandYellowMax(36, 255, 255);

  // cyan LED
  static const cv::Scalar kVNWandCyanMin(77, 0, 77);
  static const cv::Scalar kVNWandCyanMax(113, 255, 255);

  // magenta LED
  static const cv::Scalar kVNWandMagentaMin(0, 0, 77);
  static const cv::Scalar kVNWandMagentaMax(36, 255, 255);

} // namespace Vision
} // namespace LF
