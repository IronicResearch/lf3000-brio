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

} // namespace Vision
} // namespace LF
