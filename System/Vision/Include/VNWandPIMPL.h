#ifndef __VISION_INCLUDE_VNWANDPIMPL_H__
#define __VISION_INCLUDE_VNWANDPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <VNCoordinateTranslator.h>
#include <opencv2/opencv.hpp>
#include <Hardware/HWControllerTypes.h>
#include <DebugMPI.h>

namespace LF {
namespace Vision {
  extern const LeapFrog::Brio::S16 kVNNoWandLocationX;
  extern const LeapFrog::Brio::S16 kVNNoWandLocationY;

  class VNWandPIMPL {
  public:
    VNWandPIMPL(void);
    virtual ~VNWandPIMPL(void);

    void NotVisibleOnScreen(void);
    void VisibleOnScreen(const cv::Point &p);

    void SetColor(const LF::Hardware::HWControllerLEDColor color);

    bool IsVisible(void) const;
    VNPoint GetLocation(void) const;

    LeapFrog::Brio::U8 GetID(void) const;
    void SetID(LeapFrog::Brio::U8 id);

    const char* GetBluetoothAddress();
    void SetBluetoothAddress( const char* btaddress );

    cv::Scalar yuvMin_;
    cv::Scalar yuvMax_;

  protected:
    LeapFrog::Brio::U8 id_;
    bool visible_;
    VNPoint location_;
    LeapFrog::Brio::CDebugMPI debugMPI_;

    VNCoordinateTranslator *translator_;

    std::string    btaddress_;
  };
}
}

#endif // __VISION_INCLUDE_VNWANDPIMPL_H__
