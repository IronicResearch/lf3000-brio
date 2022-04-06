#include <VNTranslatorBase.h>
#include <limits>
#include <cmath>
#include <assert.h>

#define VN_TRANSLATOR_DEBUG 0
#if VN_TRANSLATOR_DEBUG
#include <iostream>
#endif

namespace LF {
namespace Vision {

  LeapFrog::Brio::S16 
  VNTranslatorBase::RectWidth(LeapFrog::Brio::tRect &r) {
    return r.right - r.left;
  }

  LeapFrog::Brio::S16 
  VNTranslatorBase::RectHeight(LeapFrog::Brio::tRect &r) {
    return r.bottom - r.top;
  }

  void
  VNTranslatorBase::InitRects(void) {
    // create equally scalled rectangles
    destFrame_.left = 0;
    destFrame_.right = 1;
    destFrame_.top = 0;
    destFrame_.bottom = 1;

    sourceFrame_.left = 0;
    sourceFrame_.right = 1;
    sourceFrame_.top = 0;
    sourceFrame_.bottom = 1;
  }

  void
  VNTranslatorBase::UpdateScaleFactors(void) {
    if (RectWidth(destFrame_) > 0 && RectWidth(sourceFrame_) > 0) {
      destToSourceWidthSF_ = static_cast<float>(RectWidth(sourceFrame_))/static_cast<float>(RectWidth(destFrame_));
    }
    if (RectHeight(destFrame_) > 0 && RectHeight(sourceFrame_) > 0) {
      destToSourceHeightSF_ = static_cast<float>(RectHeight(sourceFrame_))/static_cast<float>(RectHeight(destFrame_));
    }
#if VN_TRANSLATOR_DEBUG
    std::cout << "CT UpdateSF: destToSourceWidthSF_ = " << destToSourceWidthSF_ << ", destToSourceHeightSF_ = " << destToSourceHeightSF_ << std::endl;
#endif

  }

  VNTranslatorBase::VNTranslatorBase(void) :
    destToSourceWidthSF_(1.0),
    destToSourceHeightSF_(1.0) {
    InitRects();
    UpdateScaleFactors();
  }

  VNTranslatorBase::~VNTranslatorBase(void) {

  }

  void
  VNTranslatorBase::Copy(const VNTranslatorBase &b) {
    SetDestFrame(b.destFrame_);
    SetSourceFrame(b.sourceFrame_);    
  }

  VNTranslatorBase::VNTranslatorBase(const VNTranslatorBase &b) {
    Copy(b);
  }

  VNTranslatorBase&
  VNTranslatorBase::operator =(const VNTranslatorBase &b) {
    Copy(b);
    return *this;
  }

  const LeapFrog::Brio::tRect
  VNTranslatorBase::GetDestFrame(void) const {
    return destFrame_;
  }

  void
  VNTranslatorBase::SetDestFrame(const LeapFrog::Brio::tRect &r) {
    destFrame_.left = r.left;
    destFrame_.right = r.right;
    destFrame_.top = r.top;
    destFrame_.bottom = r.bottom;
    UpdateScaleFactors();

#if VN_TRANSLATOR_DEBUG
    std::cout << "CT SetDestFrame: " << destFrame_.left << ", " 
	      << destFrame_.top << ", " 
	      << destFrame_.right << ", " 
	      << destFrame_.bottom << std::endl;
#endif
 }

  void
  VNTranslatorBase::SetDestFrame(const cv::Rect &r) {
    LeapFrog::Brio::tRect rect;
    rect.left = r.x; rect.top = r.y;
    rect.right = r.x + r.width;
    rect.bottom = r.y + r.height;
    SetDestFrame(rect);
  }

  void
  VNTranslatorBase::SetSourceFrame(const LeapFrog::Brio::tRect &r) {
    sourceFrame_.left = r.left;
    sourceFrame_.right = r.right;
    sourceFrame_.top = r.top;
    sourceFrame_.bottom = r.bottom;
    UpdateScaleFactors();

#if VN_TRANSLATOR_DEBUG
    std::cout << "CT SetSourceFrame: " << sourceFrame_.left << ", " 
	      << sourceFrame_.top << ", " 
	      << sourceFrame_.right << ", " 
	      << sourceFrame_.bottom << std::endl;
#endif
  }

  void
  VNTranslatorBase::SetSourceFrame(const cv::Rect &r) {
    LeapFrog::Brio::tRect rect;
    rect.left = r.x; rect.top = r.y;
    rect.right = r.x + r.width;
    rect.bottom = r.y + r.height;
    SetSourceFrame(rect);
  }

  const LeapFrog::Brio::tRect
  VNTranslatorBase::GetSourceFrame(void) const {
    return sourceFrame_;
  }


  VNPoint
  VNTranslatorBase::FromSourceToDest(const VNPoint &p) const {
    static VNPoint result;
    
    result.x = (1.0f/destToSourceWidthSF_)*(p.x-sourceFrame_.left) + destFrame_.left;
    result.y = (1.0f/destToSourceHeightSF_)*(p.y-sourceFrame_.top) + destFrame_.top;    

#if VN_TRANSLATOR_DEBUG
    std::cout << "CT Point StoD->p = " << p.x << ", " << p.y << ", result = " << result.x << ", " << result.y << std::endl;
#endif

    return result;
  }

  cv::Point 
  VNTranslatorBase::FromSourceToDest(const cv::Point &p) const {
    static VNPoint lfp;
    lfp.x = p.x; lfp.y = p.y;
    lfp = FromSourceToDest(lfp);

    return cv::Point(lfp.x, lfp.y);
  }

  VNPoint
  VNTranslatorBase::FromDestToSource(const VNPoint &p) const {
    static VNPoint result;

    result.x = destToSourceWidthSF_*(p.x-destFrame_.left) + sourceFrame_.left;
    result.y = destToSourceHeightSF_*(p.y-destFrame_.top) + sourceFrame_.top;

#if VN_TRANSLATOR_DEBUG
    std::cout << "CT Point DtoS->p = " << p.x << ", " << p.y << ", result = " << result.x << ", " << result.y << std::endl;
#endif

    return result;
  }

  cv::Point 
  VNTranslatorBase::FromDestToSource(const cv::Point &p) const {
    static VNPoint lfp;
    lfp.x = p.x; lfp.y = p.y;
    lfp = FromDestToSource(lfp);

    return cv::Point(lfp.x, lfp.y);
  }

  LeapFrog::Brio::tRect
  VNTranslatorBase::FromSourceToDest(const LeapFrog::Brio::tRect &r) const {
    static LeapFrog::Brio::tRect result;
    static VNPoint p;

    // first get top left corner
    p.x = r.left; p.y = r.top;
    p = FromSourceToDest(p);
    result.left = p.x; result.top = p.y;

    // no bottom right corner
    p.x = r.right; p.y = r.bottom;
    p = FromSourceToDest(p);
    result.right = p.x; result.bottom = p.y;

#if VN_TRANSLATOR_DEBUG
    std::cout << "CT Rect StoD: r =" << r.left << ", " 
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
  VNTranslatorBase::FromDestToSource(const LeapFrog::Brio::tRect &r) const {
    static LeapFrog::Brio::tRect result;
    static VNPoint p;

    // first get top left corner
    p.x = r.left; p.y = r.top;
    p = FromDestToSource(p);
    result.left = p.x; result.top = p.y;

    // no bottom right corner
    p.x = r.right; p.y = r.bottom;
    p = FromDestToSource(p);
    result.right = p.x; result.bottom = p.y;

#if VN_TRANSLATOR_DEBUG
    std::cout << "CT Rect DtoS: r =" << r.left << ", " 
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
  VNTranslatorBase::FromSourceToDestAlongX(const float val) const {
    return (1.0f/destToSourceWidthSF_)*val;
  }

  float
  VNTranslatorBase::FromSourceToDestAlongY(const float val) const {
    return (1.0f/destToSourceHeightSF_)*val;
  }

  float
  VNTranslatorBase::FromDestToSourceAlongX(const float val) const {
    return destToSourceWidthSF_*val;
  }

  float
  VNTranslatorBase::FromDestToSourceAlongY(const float val) const {
    return destToSourceHeightSF_*val;
  }

  float
  VNTranslatorBase::GetDestToSourceXSF(void) const {
    return destToSourceWidthSF_;
  }

  float
  VNTranslatorBase::GetDestToSourceYSF(void) const {
    return destToSourceHeightSF_;
  }

  float
  VNTranslatorBase::GetSourceToDestXSF(void) const {
    assert(fabs(destToSourceWidthSF_) > std::numeric_limits<float>::epsilon());
    return (1.0f/destToSourceWidthSF_);
  }

  float
  VNTranslatorBase::GetSourceToDestYSF(void) const {
    assert(fabs(destToSourceHeightSF_) > std::numeric_limits<float>::epsilon());
    return (1.0f/destToSourceHeightSF_);
  }

}
}
