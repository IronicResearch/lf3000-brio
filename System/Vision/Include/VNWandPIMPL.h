#ifndef __VISION_INCLUDE_VNWANDPIMPL_H__
#define __VISION_INCLUDE_VNWANDPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <VNCoordinateTranslator.h>
#include <opencv2/opencv.hpp>
#include <Hardware/HWControllerTypes.h>
#include <DebugMPI.h>

namespace LF {
namespace Vision {

  class VNWandPIMPL {
  public:
    VNWandPIMPL(void);
    virtual ~VNWandPIMPL(void);
    
    void NotVisibleOnScreen(void);
    void VisibleOnScreen(const cv::Point &p);

    void SetColor(const LF::Hardware::HWControllerLEDColor color);

    bool IsVisible(void) const;
    VNPoint GetLocation(void) const;

    cv::Scalar hsvMin_;
    cv::Scalar hsvMax_;

  protected:
    bool visible_;
    VNPoint location_;
    LeapFrog::Brio::CDebugMPI debugMPI_;

    VNCoordinateTranslator *translator_;
  };
}
}

#endif // __VISION_INCLUDE_VNWANDPIMPL_H__
