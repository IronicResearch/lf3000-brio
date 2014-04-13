#include <VNWandPIMPL.h>
#include <VNWandColors.h>
#include <algorithm>
#include <GroupEnumeration.h>

namespace LF {
namespace Vision {

  static const VNPixelType kVNWandHueDefault = kVNWandGreenHue;
  static const VNPixelType kVNWandSaturationDefault = kVNWandGreenSaturation;
  static const VNPixelType kVNWandBrightnessDefault = kVNWandGreenBrightness;
  static const VNPixelType kVNWandHueWidthDefault = 36; //50;

  static const VNPixelType kVNWandHueMin = kVNMinPixelValue;
  static const VNPixelType kVNWandHueMax = kVNMaxPixelValue;
  static const VNPixelType kVNWandHueWidthMin = kVNMinPixelValue;
  static const VNPixelType kVNWandHueWidthMax = kVNMaxPixelValue;
  static const VNPixelType kVNWandSaturationMin = kVNMinPixelValue;
  static const VNPixelType kVNWandSaturationMax = kVNMaxPixelValue;
  static const VNPixelType kVNWandBrightnessMin = kVNMinPixelValue;
  static const VNPixelType kVNWandBrightnessMax = kVNMaxPixelValue;

  static const LeapFrog::Brio::S16 kVNNoWandLocationX = -10000;
  static const LeapFrog::Brio::S16 kVNNoWandLocationY = -10000;


  VNWandPIMPL::VNWandPIMPL(void) :
    visible_(false),
    location_(VNPoint(kVNNoWandLocationX, kVNNoWandLocationY)),
    hsvMin_(kVNWandHueDefault-0.5*kVNWandHueWidthDefault,
	    kVNWandSaturationDefault,
	    kVNWandBrightnessDefault),
    hsvMax_(kVNWandHueDefault+0.5*kVNWandHueWidthDefault,
	    kVNMaxPixelValue,
	    kVNMaxPixelValue),
    debugMPI_(kGroupVision),
    translator_(VNCoordinateTranslator::Instance()) {
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
    location_.x = static_cast<LeapFrog::Brio::S16>(p.x);
    location_.y = static_cast<LeapFrog::Brio::S16>(p.y);
    visible_ = true;
  }

  void
  VNWandPIMPL::SetColor(const LF::Hardware::HWControllerLEDColor color) {
    debugMPI_.DebugOut(kDbgLvlValuable, "VNWandPIMPL::SetColor %08x\n", (unsigned int)color);
    VNPixelType hue = 0;
    VNPixelType saturation = 0;
    VNPixelType brightness = 0;
    if (color == LF::Hardware::kHWControllerLEDGreen) {
      hue = kVNWandGreenHue;
      saturation = kVNWandGreenSaturation;
      brightness = kVNWandGreenBrightness;	
    } else if(color == LF::Hardware::kHWControllerLEDRed) {
      hue = kVNWandRedHue;
      saturation = kVNWandRedSaturation;
      brightness = kVNWandRedBrightness;	
    } else if (color == LF::Hardware::kHWControllerLEDBlue) {
      hue = kVNWandBlueHue;
      saturation = kVNWandBlueSaturation;
      brightness = kVNWandBlueBrightness;	
    } else if (color == LF::Hardware::kHWControllerLEDOrange) {
      hue = kVNWandOrangeHue;
      saturation = kVNWandOrangeSaturation;
      brightness = kVNWandOrangeBrightness;	
    } else if (color == LF::Hardware::kHWControllerLEDTurqoise) {
      hue = kVNWandTurqoiseHue;
      saturation = kVNWandTurqoiseSaturation;
      brightness = kVNWandTurqoiseBrightness;	
    } else if (color == LF::Hardware::kHWControllerLEDPurple) {
      hue = kVNWandPurpleHue;
      saturation = kVNWandPurpleSaturation;
      brightness = kVNWandPurpleBrightness;	
    } else {
      // this handles kHWControllerLEDOff case
      hue = kVNWandHueDefault;
      saturation = kVNWandSaturationDefault;
      brightness = kVNWandBrightnessDefault;
    }

    //NOTE: if using opencv RGB-->HSV the Hue values are in the range
    // of 0-180, however, the NEON and C-based implementation in
    // VNRGB2HSV.cpp are in the range 0-255.  This is only imporatant
    // if debuggin with OpenCV as both the emulator and device conversions
    // use the methods in VNRGB2HSV.cpp
    hsvMin_[0] = hue-0.5*kVNWandHueWidthDefault;
    hsvMin_[1] = saturation;
    hsvMin_[2] = brightness;
    if (hsvMin_[0] < 0) hsvMin_[0] = 0;

    hsvMax_[0] = hue+0.5*kVNWandHueWidthDefault;
    hsvMax_[1] = kVNMaxPixelValue;
    hsvMax_[2] = kVNMaxPixelValue;
    if (hsvMax_[0] > kVNMaxPixelValue) hsvMax_[0] = kVNMaxPixelValue;
  }

  bool
  VNWandPIMPL::IsVisible(void) const {
    return visible_;
  }

  VNPoint
  VNWandPIMPL::GetLocation(void) const {
    if (location_.x == kVNNoWandLocationX &&
	location_.y == kVNNoWandLocationY)
      return location_;
    return translator_->FromVisionToDisplay(location_);
  }

} // namespace Vision
} // namespace LF
