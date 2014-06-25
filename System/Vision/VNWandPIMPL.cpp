#include <VNWandPIMPL.h>
#include <VNWandColors.h>
#include <algorithm>
#include <GroupEnumeration.h>

namespace LF {
namespace Vision {

  static const cv::Scalar kVNWandDefaultHSVMin = kVNWandGreenMin;
  static const cv::Scalar kVNWandDefaultHSVMax = kVNWandGreenMax;

  const LeapFrog::Brio::S16 kVNNoWandLocationX = -10000;
  const LeapFrog::Brio::S16 kVNNoWandLocationY = -10000;


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

    } else if (color == LF::Hardware::kHWControllerLEDYellow) {
      hsvMin_ = kVNWandYellowMin;
      hsvMax_ = kVNWandYellowMax;

    } else if (color == LF::Hardware::kHWControllerLEDCyan) {
      hsvMin_ = kVNWandCyanMin;
      hsvMax_ = kVNWandCyanMax;

    } else if (color == LF::Hardware::kHWControllerLEDMagenta) {
      hsvMin_ = kVNWandMagentaMin;
      hsvMax_ = kVNWandMagentaMax;

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

  
  LeapFrog::Brio::U8
  VNWandPIMPL::GetID(void) const {
    return id_;
  }

  void
  VNWandPIMPL::SetID(LeapFrog::Brio::U8 id) {
    id_ = id;
  }
} // namespace Vision
} // namespace LF
