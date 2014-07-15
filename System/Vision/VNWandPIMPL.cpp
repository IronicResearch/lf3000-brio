#include <VNWandPIMPL.h>
#include <VNWandColors.h>
#include <algorithm>
#include <GroupEnumeration.h>
#include <zoJSON.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace LF {
namespace Vision {

  static const cv::Scalar kVNWandDefaultHSVMin = kVNWandGreenMin;
  static const cv::Scalar kVNWandDefaultHSVMax = kVNWandGreenMax;

  const LeapFrog::Brio::S16 kVNNoWandLocationX = -10000;
  const LeapFrog::Brio::S16 kVNNoWandLocationY = -10000;

  static const char kWandColorConfigurationFile[] = "/LF/Bulk/Data/Local/All/VNWandColorCalibration.json";
  static const char kWandDefaultColorConfiguration[] = "{\n"
                                                        "    \"controllers\" : {\n"
                                                        "        \"default\" : {\n"
                                                        "            \"green\" : {\n"
                                                        "                \"hmin\" : 71,\n"
                                                        "                \"hmax\" : 107,\n"
                                                        "                \"smin\" : 0,\n"
                                                        "                \"smax\" : 255,\n"
                                                        "                \"vmin\" : 77,\n"
                                                        "                \"vmax\" : 255\n"
                                                        "            },\n"
                                                        "            \"red\" : {\n"
                                                        "                \"hmin\" : 0,\n"
                                                        "                \"hmax\" : 36,\n"
                                                        "                \"smin\" : 0,\n"
                                                        "                \"smax\" : 255,\n"
                                                        "                \"vmin\" : 77,\n"
                                                        "                \"vmax\" : 255\n"
                                                        "            },\n"
                                                        "            \"blue\" : {\n"
                                                        "                \"hmin\" : 141,\n"
                                                        "                \"hmax\" : 177,\n"
                                                        "                \"smin\" : 0,\n"
                                                        "                \"smax\" : 255,\n"
                                                        "                \"vmin\" : 77,\n"
                                                        "                \"vmax\" : 255\n"
                                                        "            },\n"
                                                        "            \"yellow\" : {\n"
                                                        "                \"hmin\" : 0,\n"
                                                        "                \"hmax\" : 36,\n"
                                                        "                \"smin\" : 0,\n"
                                                        "                \"smax\" : 255,\n"
                                                        "                \"vmin\" : 77,\n"
                                                        "                \"vmax\" : 255\n"
                                                        "            },\n"
                                                        "            \"cyan\" : {\n"
                                                        "                \"hmin\" : 77,\n"
                                                        "                \"hmax\" : 113,\n"
                                                        "                \"smin\" : 0,\n"
                                                        "                \"smax\" : 255,\n"
                                                        "                \"vmin\" : 77,\n"
                                                        "                \"vmax\" : 255\n"
                                                        "            },\n"
                                                        "            \"magenta\" : {\n"
                                                        "                \"hmin\" : 0,\n"
                                                        "                \"hmax\" : 36,\n"
                                                        "                \"smin\" : 0,\n"
                                                        "                \"smax\" : 255,\n"
                                                        "                \"vmin\" : 77,\n"
                                                        "                \"vmax\" : 255\n"
                                                        "            },\n"
                                                        "            \"off\" : {\n"
                                                        "                \"hmin\" : 0,\n"
                                                        "                \"hmax\" : 0,\n"
                                                        "                \"smin\" : 0,\n"
                                                        "                \"smax\" : 0,\n"
                                                        "                \"vmin\" : 0,\n"
                                                        "                \"vmax\" : 0\n"
                                                        "            }\n"
                                                        "\n"
                                                        "\n"
                                                        "        }\n"
                                                        "    }\n"
                                                        "\n"
                                                        "}\n";


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

        // check if configuration file exists and if not create a default one
        struct stat buffer;
        if (stat (kWandColorConfigurationFile, &buffer) != 0) {
            std::ofstream defaultConfigurationFile( kWandColorConfigurationFile );
            defaultConfigurationFile << std::string( kWandDefaultColorConfiguration );
            defaultConfigurationFile.close();
        }

        zo::Value rootConfig;
        // load configuration
        std::ifstream configfile;
        configfile.open( kWandColorConfigurationFile );
        if( configfile.is_open() ) {
            debugMPI_.DebugOut(kDbgLvlValuable, "Opened color calibration file: %d\n", id_);
            debugMPI_.DebugOut(kDbgLvlValuable, "VNWandPIMPL::SetColor %08x\n", (unsigned int)color);
            zo::Value::parse( configfile, rootConfig );

            zo::Object controllersConfig = rootConfig["controllers"].get<zo::Object>();
            zo::Object defaultConfig;

            // get the controller bluetooth address
            debugMPI_.DebugOut(kDbgLvlValuable, "Controller BT Address: %s\n", btaddress_);

            if( controllersConfig.has( btaddress_ ) ) {
                defaultConfig = controllersConfig[btaddress_].get<zo::Object>();
            } else {    // has not been calibrated so get default configuration
                defaultConfig = controllersConfig["default"].get<zo::Object>();
            }
            zo::Object jsonColor;

            if (color == LF::Hardware::kHWControllerLEDGreen) {
                jsonColor = defaultConfig["green"].get<zo::Object>();
            } else if(color == LF::Hardware::kHWControllerLEDRed) {
                jsonColor = defaultConfig["red"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDBlue) {
                jsonColor = defaultConfig["blue"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDYellow) {
                jsonColor = defaultConfig["yellow"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDCyan) {
                jsonColor = defaultConfig["cyan"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDMagenta) {
                jsonColor = defaultConfig["magenta"].get<zo::Object>();
            } else {
                // this handles kHWControllerLEDOff case
                jsonColor = defaultConfig["green"].get<zo::Object>();
            }

            hsvMin_[0] = jsonColor["hmin"].get<int>();
            hsvMin_[1] = jsonColor["smin"].get<int>();
            hsvMin_[2] = jsonColor["vmin"].get<int>();

            hsvMax_[0] = jsonColor["hmax"].get<int>();
            hsvMax_[1] = jsonColor["smax"].get<int>();
            hsvMax_[2] = jsonColor["vmax"].get<int>();

            configfile.close();

        } else {
            debugMPI_.DebugOut(kDbgLvlValuable, "FAILED: Opening color calibration file");
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

    char*
    VNWandPIMPL::GetBluetoothAddress() {
        return btaddress_;
    }

    void VNWandPIMPL::SetBluetoothAddress( const char* btaddress) {
        strcpy( btaddress_, btaddress );
    }

} // namespace Vision
} // namespace LF
