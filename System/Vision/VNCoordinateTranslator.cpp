#include <VNCoordinateTranslator.h>
#include <limits>
#include <cmath>
#include <assert.h>

namespace LF {
namespace Vision {

  VNCoordinateTranslator::VNCoordinateTranslator(void) :
    VNTranslatorBase() {
  }

  VNCoordinateTranslator::~VNCoordinateTranslator(void) {
  }

  VNCoordinateTranslator*
  VNCoordinateTranslator::Instance(void) {
    static VNCoordinateTranslator *sharedInstance = NULL;
    if (sharedInstance == NULL) {
      sharedInstance = new VNCoordinateTranslator();
    }
    return sharedInstance;
  }

  void
  VNCoordinateTranslator::SetVisionFrame(const LeapFrog::Brio::tRect &r) {
    VNTranslatorBase::SetDestFrame(r);
 }

  void
  VNCoordinateTranslator::SetDisplayFrame(const LeapFrog::Brio::tRect &r) {
    VNTranslatorBase::SetSourceFrame(r);
  }


  VNPoint
  VNCoordinateTranslator::FromDisplayToVision(const VNPoint &p) const {
    return VNTranslatorBase::FromSourceToDest(p);
  }

  VNPoint
  VNCoordinateTranslator::FromVisionToDisplay(const VNPoint &p) const {
    return VNTranslatorBase::FromDestToSource(p);
  }

  LeapFrog::Brio::tRect
  VNCoordinateTranslator::FromDisplayToVision(const LeapFrog::Brio::tRect &r) const {
    return VNTranslatorBase::FromSourceToDest(r);
  }

  LeapFrog::Brio::tRect
  VNCoordinateTranslator::FromVisionToDisplay(const LeapFrog::Brio::tRect &r) const {
    return VNTranslatorBase::FromDestToSource(r);
  }

  float
  VNCoordinateTranslator::FromDisplayToVisionAlongX(const float val) const {
    return VNTranslatorBase::FromSourceToDestAlongX(val);
  }

  float
  VNCoordinateTranslator::FromDisplayToVisionAlongY(const float val) const {
    return VNTranslatorBase::FromSourceToDestAlongY(val);
  }

  float
  VNCoordinateTranslator::FromVisionToDisplayAlongX(const float val) const {
    return VNTranslatorBase::FromDestToSourceAlongX(val);
  }

  float
  VNCoordinateTranslator::FromVisionToDisplayAlongY(const float val) const {
    return VNTranslatorBase::FromDestToSourceAlongY(val);
  }

  float
  VNCoordinateTranslator::GetVisionToDisplayXSF(void) const {
    return VNTranslatorBase::GetDestToSourceXSF();
  }

  float
  VNCoordinateTranslator::GetVisionToDisplayYSF(void) const {
    return VNTranslatorBase::GetDestToSourceYSF();
  }

  float
  VNCoordinateTranslator::GetDisplayToVisionXSF(void) const {
    return VNTranslatorBase::GetSourceToDestXSF();
  }

  float
  VNCoordinateTranslator::GetDisplayToVisionYSF(void) const {
    return VNTranslatorBase::GetSourceToDestYSF();
  }

}
}
