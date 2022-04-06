#ifndef __VISION_INCLUDE_VNWANDPIMPL_H__
#define __VISION_INCLUDE_VNWANDPIMPL_H__

#include <Vision/VNVisionTypes.h>
#include <VNCoordinateTranslator.h>
#include <opencv2/opencv.hpp>
#include <Hardware/HWControllerTypes.h>
#include <DebugMPI.h>
#include <CameraMPI.h>
#include <map>

namespace LF {
namespace Vision {
  extern const LeapFrog::Brio::S16 kVNNoWandLocationX;
  extern const LeapFrog::Brio::S16 kVNNoWandLocationY;
  
  static const LeapFrog::Brio::U8 kVNNumWandColors = 6;
  static const LeapFrog::Brio::U8 kVNYUVMinIndex = 0;
  static const LeapFrog::Brio::U8 kVNYUVMaxIndex = 1;

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
    void LoadColorFilterValuesFromFile(std::ifstream &configfile);
    void LoadColorFilterValuesFromDefaults(void);
    void CreateConfigFileIfNonExistent(void);
    void LoadColorFilterValues(void);
    void SetCameraTemperature(LF::Hardware::HWControllerLEDColor color);

    LeapFrog::Brio::U8 id_;
    bool visible_;
    VNPoint location_;
    LeapFrog::Brio::CDebugMPI debugMPI_;
    LeapFrog::Brio::CCameraMPI cameraMPI_;

    VNCoordinateTranslator *translator_;

    std::string    btaddress_;

    bool loadColors_;
    cv::Scalar yuvColors_[kVNNumWandColors][2];
    
  };
}
}

#endif // __VISION_INCLUDE_VNWANDPIMPL_H__
