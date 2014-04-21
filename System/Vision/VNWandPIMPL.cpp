#include <VNWandPIMPL.h>
#include <VNWandColors.h>
#include <algorithm>
#include <GroupEnumeration.h>

namespace LF {
namespace Vision {

  static const cv::Scalar kVNWandDefaultHSVMin = kVNWandGreenMin;
  static const cv::Scalar kVNWandDefaultHSVMax = kVNWandGreenMax;

  static const LeapFrog::Brio::S16 kVNNoWandLocationX = -10000;
  static const LeapFrog::Brio::S16 kVNNoWandLocationY = -10000;


  VNWandPIMPL::VNWandPIMPL(void) :
    visible_(false),
    location_(VNPoint(kVNNoWandLocationX, kVNNoWandLocationY)),
    hsvMin_(kVNWandDefaultHSVMin),
    hsvMax_(kVNWandDefaultHSVMax),
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

    if (color == LF::Hardware::kHWControllerLEDGreen) {
      hsvMin_ = kVNWandGreenMin;
      hsvMax_ = kVNWandGreenMax;

    } else if(color == LF::Hardware::kHWControllerLEDRed) {
      hsvMin_ = kVNWandRedMin;
      hsvMax_ = kVNWandRedMax;

    } else if (color == LF::Hardware::kHWControllerLEDBlue) {
      hsvMin_ = kVNWandBlueMin;
      hsvMax_ = kVNWandBlueMax;

    } else if (color == LF::Hardware::kHWControllerLEDOrange) {
      hsvMin_ = kVNWandOrangeMin;
      hsvMax_ = kVNWandOrangeMax;

    } else if (color == LF::Hardware::kHWControllerLEDTurqoise) {
      hsvMin_ = kVNWandTurqoiseMin;
      hsvMax_ = kVNWandTurqoiseMax;

    } else if (color == LF::Hardware::kHWControllerLEDPurple) {
      hsvMin_ = kVNWandPurpleMin;
      hsvMax_ = kVNWandPurpleMax;

    } else {
      // this handles kHWControllerLEDOff case
      hsvMin_ = kVNWandDefaultHSVMin;
      hsvMax_ = kVNWandDefaultHSVMax;

    }
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
