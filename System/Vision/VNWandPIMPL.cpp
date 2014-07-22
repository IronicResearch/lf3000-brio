#include <VNWandPIMPL.h>
#include <VNWandColors.h>
#include <algorithm>
#include <GroupEnumeration.h>
#include <zoJSON.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CameraMPI.h>
#include <VNAlgorithmHelpers.h>

namespace LF {
namespace Vision {

  static const cv::Scalar kVNWandDefaultYUVMin = kVNWandGreenMin;
  static const cv::Scalar kVNWandDefaultYUVMax = kVNWandGreenMax;

  const LeapFrog::Brio::S16 kVNNoWandLocationX = -10000;
  const LeapFrog::Brio::S16 kVNNoWandLocationY = -10000;

  static const char kWandColorConfigurationFile[] = "/LF/Bulk/Data/Local/All/VNWandColorCalibration.json";
  static const char kWandDefaultColorConfiguration[] = "{\n"
                                                        "    \"controllers\" : {\n"
                                                        "        \"default\" : {\n"
                                                        "            \"green\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 184,\n"
                                                        "                \"umax\" : 254,\n"
                                                        "                \"vmin\" : 144,\n"
                                                        "                \"vmax\" : 214\n"
                                                        "            },\n"
                                                        "            \"red\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 32,\n"
                                                        "                \"umax\" : 102,\n"
                                                        "                \"vmin\" : 56,\n"
                                                        "                \"vmax\" : 126\n"
                                                        "            },\n"
                                                        "            \"blue\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 36,\n"
                                                        "                \"umax\" : 106,\n"
                                                        "                \"vmin\" : 178,\n"
                                                        "                \"vmax\" : 248\n"
                                                        "            },\n"
                                                        "            \"yellow\" : {\n"
                                                        "                \"ymin\" : 40,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 8,\n"
                                                        "                \"umax\" : 78,\n"
                                                        "                \"vmin\" : 17,\n"
                                                        "                \"vmax\" : 87\n"
                                                        "            },\n"
                                                        "            \"cyan\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 0,\n"
                                                        "                \"umax\" : 70,\n"
                                                        "                \"vmin\" : 173,\n"
                                                        "                \"vmax\" : 243\n"
                                                        "            },\n"
                                                        "            \"magenta\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 37,\n"
                                                        "                \"umax\" : 107,\n"
                                                        "                \"vmin\" : 38,\n"
                                                        "                \"vmax\" : 108\n"
                                                        "            },\n"
                                                        "            \"off\" : {\n"
                                                        "                \"ymin\" : 0,\n"
                                                        "                \"ymax\" : 0,\n"
                                                        "                \"umin\" : 0,\n"
                                                        "                \"umax\" : 0,\n"
                                                        "                \"vmin\" : 0,\n"
                                                        "                \"vmax\" : 0\n"
                                                        "            }\n"
                                                        "\n"
                                                        "\n"
                                                        "        }\n"
                                                        "    }\n"
                                                        "\n"
                                                        "}\n";

  void
  SetCameraTemperature(bool warm) {
    LeapFrog::Brio::tCameraControls controls;
    LeapFrog::Brio::CCameraMPI cameraMPI;
    LeapFrog::Brio::CDebugMPI dbg(kGroupVision);

    LeapFrog::Brio::Boolean err = cameraMPI.GetCameraControls(controls);
    dbg.Assert(err, "VNWandPIMPL could get camera controls\n");

    // turn off autowhitebalance
    LeapFrog::Brio::tControlInfo *temp = FindCameraControl(controls,
							   LeapFrog::Brio::kControlTypeTemperature);
    if (temp) {
      cameraMPI.SetCameraControl(temp, ((warm) ? temp->max : temp->min)); // is a boolean, set to 0 for false
    } else {
      dbg.DebugOut(kDbgLvlCritical, "null camera control for auto white balance\n");
    }
  }

  VNWandPIMPL::VNWandPIMPL(void) :
    visible_(false),
    location_(VNPoint(kVNNoWandLocationX, kVNNoWandLocationY)),
    yuvMin_(kVNWandDefaultYUVMin),
    yuvMax_(kVNWandDefaultYUVMax),
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
	        SetCameraTemperature(true);
                jsonColor = defaultConfig["green"].get<zo::Object>();
            } else if(color == LF::Hardware::kHWControllerLEDRed) {
	        SetCameraTemperature(false);
                jsonColor = defaultConfig["red"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDBlue) {
	        SetCameraTemperature(true);
                jsonColor = defaultConfig["blue"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDYellow) {
	        SetCameraTemperature(false);
                jsonColor = defaultConfig["yellow"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDCyan) {
	        SetCameraTemperature(true);
                jsonColor = defaultConfig["cyan"].get<zo::Object>();
            } else if (color == LF::Hardware::kHWControllerLEDMagenta) {
	        SetCameraTemperature(false);
                jsonColor = defaultConfig["magenta"].get<zo::Object>();
            } else {
	        SetCameraTemperature(true);
                // this handles kHWControllerLEDOff case
                jsonColor = defaultConfig["green"].get<zo::Object>();
            }

            yuvMin_[0] = jsonColor["ymin"].get<int>();
            yuvMin_[1] = jsonColor["umin"].get<int>();
            yuvMin_[2] = jsonColor["vmin"].get<int>();

            yuvMax_[0] = jsonColor["ymax"].get<int>();
            yuvMax_[1] = jsonColor["umax"].get<int>();
            yuvMax_[2] = jsonColor["vmax"].get<int>();

            configfile.close();

        } else {
            debugMPI_.DebugOut(kDbgLvlValuable, "FAILED: Opening color calibration file");
            debugMPI_.DebugOut(kDbgLvlValuable, "VNWandPIMPL::SetColor %08x\n", (unsigned int)color);

            if (color == LF::Hardware::kHWControllerLEDGreen) {
	        SetCameraTemperature(true);
                yuvMin_ = kVNWandGreenMin;
                yuvMax_ = kVNWandGreenMax;

            } else if(color == LF::Hardware::kHWControllerLEDRed) {
	        SetCameraTemperature(false);
                yuvMin_ = kVNWandRedMin;
                yuvMax_ = kVNWandRedMax;

            } else if (color == LF::Hardware::kHWControllerLEDBlue) {
	        SetCameraTemperature(true);
                yuvMin_ = kVNWandBlueMin;
                yuvMax_ = kVNWandBlueMax;

            } else if (color == LF::Hardware::kHWControllerLEDYellow) {
	        SetCameraTemperature(false);
                yuvMin_ = kVNWandYellowMin;
                yuvMax_ = kVNWandYellowMax;

            } else if (color == LF::Hardware::kHWControllerLEDCyan) {
	        SetCameraTemperature(true);
                yuvMin_ = kVNWandCyanMin;
                yuvMax_ = kVNWandCyanMax;

            } else if (color == LF::Hardware::kHWControllerLEDMagenta) {
	        SetCameraTemperature(false);
                yuvMin_ = kVNWandMagentaMin;
                yuvMax_ = kVNWandMagentaMax;

            } else {
	        SetCameraTemperature(true);
                // this handles kHWControllerLEDOff case
                yuvMin_ = kVNWandDefaultYUVMin;
                yuvMax_ = kVNWandDefaultYUVMax;

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
