#ifndef __VISION_INCLUDE_VNTRANSLATORBASE_H__
#define __VISION_INCLUDE_VNTRANSLATORBASE_H__

#include <Vision/VNVisionTypes.h>
#include <DisplayTypes.h>
#include <opencv2/core/core.hpp>

namespace LF {
namespace Vision {

  class VNTranslatorBase {
  public:
    VNTranslatorBase(void);
    virtual ~VNTranslatorBase(void);
    VNTranslatorBase(const VNTranslatorBase&);
    VNTranslatorBase& operator =(const VNTranslatorBase&);
    
    VNPoint FromSourceToDest(const VNPoint &p) const;
    VNPoint FromDestToSource(const VNPoint &p) const;

    cv::Point FromSourceToDest(const cv::Point &p) const;
    cv::Point FromDestToSource(const cv::Point &p) const;

    LeapFrog::Brio::tRect FromSourceToDest(const LeapFrog::Brio::tRect &r) const;
    LeapFrog::Brio::tRect FromDestToSource(const LeapFrog::Brio::tRect &r) const;

    float FromDestToSourceAlongX(const float val) const;
    float FromDestToSourceAlongY(const float val) const;

    float FromSourceToDestAlongX(const float val) const;
    float FromSourceToDestAlongY(const float val) const;

    void SetDestFrame(const LeapFrog::Brio::tRect &r);
    void SetDestFrame(const cv::Rect &r);
    const LeapFrog::Brio::tRect GetDestFrame(void) const;

    void SetSourceFrame(const LeapFrog::Brio::tRect &r);
    void SetSourceFrame(const cv::Rect &r);
    const LeapFrog::Brio::tRect GetSourceFrame(void) const;

    float GetDestToSourceXSF(void) const;
    float GetDestToSourceYSF(void) const;
    
    float GetSourceToDestXSF(void) const;
    float GetSourceToDestYSF(void) const;

  private:
    void InitRects(void);
    void UpdateScaleFactors(void);
    void Copy(const VNTranslatorBase &b);
    static LeapFrog::Brio::S16 RectWidth(LeapFrog::Brio::tRect &r);
    static LeapFrog::Brio::S16 RectHeight(LeapFrog::Brio::tRect &r);

    LeapFrog::Brio::tRect sourceFrame_;
    LeapFrog::Brio::tRect destFrame_;
    float destToSourceWidthSF_;
    float destToSourceHeightSF_;
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNTRANSLATORBASE_H__
