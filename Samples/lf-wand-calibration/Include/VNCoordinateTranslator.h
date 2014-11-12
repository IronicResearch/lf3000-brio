#ifndef __VISION_INCLUDE_VNCOORDINATETRANSLATOR_H__
#define __VISION_INCLUDE_VNCOORDINATETRANSLATOR_H__


#include <VNTranslatorBase.h>
#include <DisplayTypes.h>

namespace LF {
namespace Vision {

  class VNCoordinateTranslator : public VNTranslatorBase {
  public:
    static VNCoordinateTranslator* Instance(void);
    virtual ~VNCoordinateTranslator(void);
    
    VNPoint FromDisplayToVision(const VNPoint &p) const;
    VNPoint FromVisionToDisplay(const VNPoint &p) const;

    LeapFrog::Brio::tRect FromDisplayToVision(const LeapFrog::Brio::tRect &r) const;
    LeapFrog::Brio::tRect FromVisionToDisplay(const LeapFrog::Brio::tRect &r) const;

    float FromVisionToDisplayAlongX(const float val) const;
    float FromVisionToDisplayAlongY(const float val) const;

    float FromDisplayToVisionAlongX(const float val) const;
    float FromDisplayToVisionAlongY(const float val) const;

    void SetVisionFrame(const LeapFrog::Brio::tRect &r);
    void SetDisplayFrame(const LeapFrog::Brio::tRect &r);

    float GetVisionToDisplayXSF(void) const;
    float GetVisionToDisplayYSF(void) const;
    
    float GetDisplayToVisionXSF(void) const;
    float GetDisplayToVisionYSF(void) const;

  private:
    VNCoordinateTranslator(void);
    VNCoordinateTranslator(const VNCoordinateTranslator&);
    VNCoordinateTranslator& operator =(const VNCoordinateTranslator&);
  };

} // namespace Vision
} // namespace LF

#endif // __VISION_INCLUDE_VNCOORDINATETRANSLATOR_H__
