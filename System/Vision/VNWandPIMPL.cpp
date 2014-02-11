#include <VNWandPIMPL.h>

namespace LF {
namespace Vision {

  static const VNPixelType kVNWandHueDefault = 79;
  static const VNPixelType kVNWandSaturationDefault = 36;
  static const VNPixelType kVNWandBrightnessDefault = 96;
  static const VNPixelType kVNWandHueWidthDefault = 35;

  static const VNPixelType kVNWandHueMin = kVNMinPixelValue;
  static const VNPixelType kVNWandHueMax = kVNMaxPixelValue;
  static const VNPixelType kVNWandHueWidthMin = kVNMinPixelValue;
  static const VNPixelType kVNWandHueWidthMax = kVNMaxPixelValue;
  static const VNPixelType kVNWandSaturationMin = kVNMinPixelValue;
  static const VNPixelType kVNWandSaturationMax = kVNMaxPixelValue;
  static const VNPixelType kVNWandBrightnessMin = kVNMinPixelValue;
  static const VNPixelType kVNWandBrightnessMax = kVNMaxPixelValue;

  static const float kVNNoWandLocationX = -10000;
  static const float kVNNoWandLocationY = -10000;


  VNWandPIMPL::VNWandPIMPL(void) :
    visible_(false),
    location_(VNPoint()),
    hsvMin_(kVNWandHueDefault-0.5*kVNWandHueWidthDefault,
	    kVNWandSaturationDefault,
	    kVNWandBrightnessDefault),
    hsvMax_(kVNWandHueDefault+0.5*kVNWandHueWidthDefault,
	    kVNMaxPixelValue,
	    kVNMaxPixelValue) {
  }

  VNWandPIMPL::~VNWandPIMPL(void) {
  }

  void
  VNWandPIMPL::NotVisibleOnScreen(void) {
    location_.x = kVNNoWandLocationX;
    location_.y = kVNNoWandLocationY;
    visible_ = false;
  }

  void
  VNWandPIMPL::VisibleOnScreen(const cv::Point &p) {
    location_.x = p.x;
    location_.y = p.y;
    visible_ = true;
  }

  bool
  VNWandPIMPL::IsVisible(void) const {
    return visible_;
  }

  VNPoint
  VNWandPIMPL::GetLocation(void) const {
    return location_;
  }

} // namespace Vision
} // namespace LF
