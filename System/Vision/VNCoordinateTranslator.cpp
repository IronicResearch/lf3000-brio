#include <VNCoordinateTranslator.h>
#include <limits>
#include <cmath>
#include <assert.h>

#define VN_COORDINATETRANSLATOR_DEBUG 0
#if VN_COORDINATETRANSLATOR_DEBUG
#include <iostream>
#endif

namespace LF {
namespace Vision {

  LeapFrog::Brio::S16 RectWidth(LeapFrog::Brio::tRect &r) {
    return r.right - r.left;
  }

  LeapFrog::Brio::S16 RectHeight(LeapFrog::Brio::tRect &r) {
    return r.bottom - r.top;
  }

  void
  VNCoordinateTranslator::InitRects(void) {
    // create equally scalled rectangles
    visionFrame_.left = 0;
    visionFrame_.right = 1;
    visionFrame_.top = 0;
    visionFrame_.bottom = 1;

    displayFrame_.left = 0;
    displayFrame_.right = 1;
    displayFrame_.top = 0;
    displayFrame_.bottom = 1;
  }

  void
  VNCoordinateTranslator::UpdateScaleFactors(void) {
    if (RectWidth(visionFrame_) > 0 && RectWidth(displayFrame_) > 0) {
      visionToDisplayWidthSF_ = static_cast<float>(RectWidth(displayFrame_))/static_cast<float>(RectWidth(visionFrame_));
    }
    if (RectHeight(visionFrame_) > 0 && RectHeight(displayFrame_) > 0) {
      visionToDisplayHeightSF_ = static_cast<float>(RectHeight(displayFrame_))/static_cast<float>(RectHeight(visionFrame_));
    }
#if VN_COORDINATETRANSLATOR_DEBUG
    std::cout << "CT UpdateSF: visionToDisplayWidthSF_ = " << visionToDisplayWidthSF_ << ", visionToDisplayHeightSF_ = " << visionToDisplayHeightSF_ << std::endl;
#endif

  }

  VNCoordinateTranslator::VNCoordinateTranslator(void) :
    visionToDisplayWidthSF_(1.0),
    visionToDisplayHeightSF_(1.0) {
    InitRects();
    UpdateScaleFactors();
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
    visionFrame_.left = r.left;
    visionFrame_.right = r.right;
    visionFrame_.top = r.top;
    visionFrame_.bottom = r.bottom;
    UpdateScaleFactors();

#if VN_COORDINATETRANSLATOR_DEBUG
    std::cout << "CT SetVisionFrame: " << visionFrame_.left << ", " 
	      << visionFrame_.top << ", " 
	      << visionFrame_.right << ", " 
	      << visionFrame_.bottom << std::endl;
#endif
 }

  void
  VNCoordinateTranslator::SetDisplayFrame(const LeapFrog::Brio::tRect &r) {
    displayFrame_.left = r.left;
    displayFrame_.right = r.right;
    displayFrame_.top = r.top;
    displayFrame_.bottom = r.bottom;
    UpdateScaleFactors();

#if VN_COORDINATETRANSLATOR_DEBUG
    std::cout << "CT SetDisplayFrame: " << displayFrame_.left << ", " 
	      << displayFrame_.top << ", " 
	      << displayFrame_.right << ", " 
	      << displayFrame_.bottom << std::endl;
#endif
  }


  VNPoint
  VNCoordinateTranslator::FromDisplayToVision(const VNPoint &p) const {
    static VNPoint result;
    
    result.x = (1.0f/visionToDisplayWidthSF_)*(p.x-displayFrame_.left) + visionFrame_.left;
    result.y = (1.0f/visionToDisplayHeightSF_)*(p.y-displayFrame_.top) + visionFrame_.top;    

#if VN_COORDINATETRANSLATOR_DEBUG
    std::cout << "CT Point DtoV->p = " << p.x << ", " << p.y << ", result = " << result.x << ", " << result.y << std::endl;
#endif

    return result;
  }

  VNPoint
  VNCoordinateTranslator::FromVisionToDisplay(const VNPoint &p) const {
    static VNPoint result;

    result.x = visionToDisplayWidthSF_*(p.x-visionFrame_.left) + displayFrame_.left;
    result.y = visionToDisplayHeightSF_*(p.y-visionFrame_.top) + displayFrame_.top;

#if VN_COORDINATETRANSLATOR_DEBUG
    std::cout << "CT Point VtoD->p = " << p.x << ", " << p.y << ", result = " << result.x << ", " << result.y << std::endl;
#endif

    return result;
  }

  LeapFrog::Brio::tRect
  VNCoordinateTranslator::FromDisplayToVision(const LeapFrog::Brio::tRect &r) const {
    static LeapFrog::Brio::tRect result;
    static VNPoint p;

    // first get top left corner
    p.x = r.left; p.y = r.top;
    p = FromDisplayToVision(p);
    result.left = p.x; result.top = p.y;

    // no bottom right corner
    p.x = r.right; p.y = r.bottom;
    p = FromDisplayToVision(p);
    result.right = p.x; result.bottom = p.y;

#if VN_COORDINATETRANSLATOR_DEBUG
    std::cout << "CT Rect DtoV: r =" << r.left << ", " 
	      << r.top << ", " 
	      << r.right << ", " 
	      << r.bottom << std::endl;
    std::cout << "         result =" << result.left << ", " 
	      << result.top << ", " 
	      << result.right << ", " 
	      << result.bottom << std::endl;
#endif
    return result;
  }

  LeapFrog::Brio::tRect
  VNCoordinateTranslator::FromVisionToDisplay(const LeapFrog::Brio::tRect &r) const {
    static LeapFrog::Brio::tRect result;
    static VNPoint p;

    // first get top left corner
    p.x = r.left; p.y = r.top;
    p = FromVisionToDisplay(p);
    result.left = p.x; result.top = p.y;

    // no bottom right corner
    p.x = r.right; p.y = r.bottom;
    p = FromVisionToDisplay(p);
    result.right = p.x; result.bottom = p.y;

#if VN_COORDINATETRANSLATOR_DEBUG
    std::cout << "CT Rect VtoD: r =" << r.left << ", " 
	      << r.top << ", " 
	      << r.right << ", " 
	      << r.bottom << std::endl;
    std::cout << "         result =" << result.left << ", " 
	      << result.top << ", " 
	      << result.right << ", " 
	      << result.bottom << std::endl;
#endif
    return result;
  }

  float
  VNCoordinateTranslator::FromDisplayToVisionAlongX(const float val) const {
    return (1.0f/visionToDisplayWidthSF_)*val;
  }

  float
  VNCoordinateTranslator::FromDisplayToVisionAlongY(const float val) const {
    return (1.0f/visionToDisplayHeightSF_)*val;
  }

  float
  VNCoordinateTranslator::FromVisionToDisplayAlongX(const float val) const {
    return visionToDisplayWidthSF_*val;
  }

  float
  VNCoordinateTranslator::FromVisionToDisplayAlongY(const float val) const {
    return visionToDisplayHeightSF_*val;
  }

  float
  VNCoordinateTranslator::GetVisionToDisplayXSF(void) const {
    return visionToDisplayWidthSF_;
  }

  float
  VNCoordinateTranslator::GetVisionToDisplayYSF(void) const {
    return visionToDisplayHeightSF_;
  }

  float
  VNCoordinateTranslator::GetDisplayToVisionXSF(void) const {
    assert(fabs(visionToDisplayWidthSF_) > std::numeric_limits<float>::epsilon());
    return (1.0f/visionToDisplayWidthSF_);
  }

  float
  VNCoordinateTranslator::GetDisplayToVisionYSF(void) const {
    assert(fabs(visionToDisplayHeightSF_) > std::numeric_limits<float>::epsilon());
    return (1.0f/visionToDisplayHeightSF_);
  }

}
}
