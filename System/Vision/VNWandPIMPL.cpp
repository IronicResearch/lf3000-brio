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

using namespace LF::Hardware;

namespace LF {
namespace Vision {
  std::map<LF::Hardware::HWControllerLEDColor, LeapFrog::Brio::U8> createMap(void) {
    std::map<LF::Hardware::HWControllerLEDColor, LeapFrog::Brio::U8> colorMap;
    colorMap[LF::Hardware::kHWControllerLEDGreen]   = 0;
    colorMap[LF::Hardware::kHWControllerLEDRed]     = 1;
    colorMap[LF::Hardware::kHWControllerLEDBlue]    = 2;
    colorMap[LF::Hardware::kHWControllerLEDYellow]  = 3;
    colorMap[LF::Hardware::kHWControllerLEDCyan]    = 4;
    colorMap[LF::Hardware::kHWControllerLEDMagenta] = 5;
    return colorMap;
  }

  static const cv::Scalar kVNWandDefaultYUVMin = kVNWandGreenMin;
  static const cv::Scalar kVNWandDefaultYUVMax = kVNWandGreenMax;

  const LeapFrog::Brio::S16 kVNNoWandLocationX = -10000;
  const LeapFrog::Brio::S16 kVNNoWandLocationY = -10000;

  static std::map<LF::Hardware::HWControllerLEDColor, LeapFrog::Brio::U8> colorToIndex = createMap();

  static const std::string kVNDefaultBTAddress = "default";
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

  
  VNWandPIMPL::VNWandPIMPL(void) :
    visible_(false),
    location_(VNPoint(kVNNoWandLocationX, kVNNoWandLocationY)),
    yuvMin_(kVNWandDefaultYUVMin),
    yuvMax_(kVNWandDefaultYUVMax),
    debugMPI_(kGroupVision),
    translator_(VNCoordinateTranslator::Instance()),
    btaddress_(kVNDefaultBTAddress),
    loadColors_(true) {

    LoadColorFilterValues();
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
  VNWandPIMPL::SetCameraTemperature(LF::Hardware::HWControllerLEDColor color) {
    LeapFrog::Brio::tCameraControls controls;
    LeapFrog::Brio::Boolean err = cameraMPI_.GetCameraControls(controls);
    debugMPI_.Assert(err, "VNWandPIMPL could not get camera controls\n");

    // get the temperature control
    LeapFrog::Brio::tControlInfo *temp = FindCameraControl(controls,
							   LeapFrog::Brio::kControlTypeTemperature);

    if (temp) {
      if (color == kHWControllerLEDGreen ||
	  color == kHWControllerLEDBlue ||
	  color == kHWControllerLEDCyan) {
	// set it warm for cool colors
	cameraMPI_.SetCameraControl(temp, temp->max); 

      } else if(color == kHWControllerLEDRed ||
		color == kHWControllerLEDYellow || 
		color == kHWControllerLEDMagenta) {
	// set it cool for warm colors
	cameraMPI_.SetCameraControl(temp, temp->min); 
      } else {
	// led off, do nothing
      }
    } else {
      debugMPI_.DebugOut(kDbgLvlCritical, "null camera control for temperature\n");
    }
  }

  void
  VNWandPIMPL::LoadColorFilterValuesFromFile(std::ifstream &configfile) {
      zo::Value rootConfig;

      debugMPI_.DebugOut(kDbgLvlValuable, "Opened color calibration file: %d\n", id_);
      zo::Value::parse(configfile, rootConfig);
      
      zo::Object controllersConfig = rootConfig["controllers"].get<zo::Object>();
      zo::Object defaultConfig;
      
      // get the controller bluetooth address
      debugMPI_.DebugOut(kDbgLvlValuable, "Controller BT Address: %s\n", btaddress_.c_str()); 

      if( controllersConfig.has( btaddress_.c_str() ) ) {
	debugMPI_.DebugOut(kDbgLvlValuable, "Controller has been calibrated\n");
	defaultConfig = controllersConfig[btaddress_.c_str()].get<zo::Object>();
      } else {    // has not been calibrated so get default configuration
	debugMPI_.DebugOut(kDbgLvlValuable, "Controller has NOT been calibrated\n");
	defaultConfig = controllersConfig["default"].get<zo::Object>();
      }
      zo::Object jsonColor;
      cv::Scalar color;
      // green
      jsonColor = defaultConfig["green"].get<zo::Object>();
      color[0] = jsonColor["ymin"].get<int>();
      color[1] = jsonColor["umin"].get<int>();
      color[2] = jsonColor["vmin"].get<int>();
      yuvColors_[colorToIndex[LF::Hardware::kHWControllerLEDGreen]][kVNYUVMinIndex] = color;

      color[0] = jsonColor["ymax"].get<int>();
      color[1] = jsonColor["umax"].get<int>();
      color[2] = jsonColor["vmax"].get<int>();
      yuvColors_[colorToIndex[LF::Hardware::kHWControllerLEDGreen]][kVNYUVMaxIndex] = color;

      // red
      jsonColor = defaultConfig["red"].get<zo::Object>();
      color[0] = jsonColor["ymin"].get<int>();
      color[1] = jsonColor["umin"].get<int>();
      color[2] = jsonColor["vmin"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDRed]][kVNYUVMinIndex] = color;

      color[0] = jsonColor["ymax"].get<int>();
      color[1] = jsonColor["umax"].get<int>();
      color[2] = jsonColor["vmax"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDRed]][kVNYUVMaxIndex] = color;

      // blue
      jsonColor = defaultConfig["blue"].get<zo::Object>();
      color[0] = jsonColor["ymin"].get<int>();
      color[1] = jsonColor["umin"].get<int>();
      color[2] = jsonColor["vmin"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDBlue]][kVNYUVMinIndex] = color;

      color[0] = jsonColor["ymax"].get<int>();
      color[1] = jsonColor["umax"].get<int>();
      color[2] = jsonColor["vmax"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDBlue]][kVNYUVMaxIndex] = color;

      // yellow
      jsonColor = defaultConfig["yellow"].get<zo::Object>();
      color[0] = jsonColor["ymin"].get<int>();
      color[1] = jsonColor["umin"].get<int>();
      color[2] = jsonColor["vmin"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDYellow]][kVNYUVMinIndex] = color;

      color[0] = jsonColor["ymax"].get<int>();
      color[1] = jsonColor["umax"].get<int>();
      color[2] = jsonColor["vmax"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDYellow]][kVNYUVMaxIndex] = color;

      // cyan
      jsonColor = defaultConfig["cyan"].get<zo::Object>();
      color[0] = jsonColor["ymin"].get<int>();
      color[1] = jsonColor["umin"].get<int>();
      color[2] = jsonColor["vmin"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDCyan]][kVNYUVMinIndex] = color;

      color[0] = jsonColor["ymax"].get<int>();
      color[1] = jsonColor["umax"].get<int>();
      color[2] = jsonColor["vmax"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDCyan]][kVNYUVMaxIndex] = color;

      // magenta
      jsonColor = defaultConfig["magenta"].get<zo::Object>();
      color[0] = jsonColor["ymin"].get<int>();
      color[1] = jsonColor["umin"].get<int>();
      color[2] = jsonColor["vmin"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDMagenta]][kVNYUVMinIndex] = color;

      color[0] = jsonColor["ymax"].get<int>();
      color[1] = jsonColor["umax"].get<int>();
      color[2] = jsonColor["vmax"].get<int>();
      yuvColors_[colorToIndex[kHWControllerLEDMagenta]][kVNYUVMaxIndex] = color;

      configfile.close();

      loadColors_ = false;
  }

  void
  VNWandPIMPL::LoadColorFilterValuesFromDefaults(void) {
      debugMPI_.DebugOut(kDbgLvlValuable, "FAILED: Opening color calibration file\n");

      // green
      yuvColors_[colorToIndex[kHWControllerLEDGreen]][kVNYUVMinIndex] = kVNWandGreenMin;
      yuvColors_[colorToIndex[kHWControllerLEDGreen]][kVNYUVMaxIndex] = kVNWandGreenMax;
      // red
      yuvColors_[colorToIndex[kHWControllerLEDRed]][kVNYUVMinIndex] = kVNWandRedMin;
      yuvColors_[colorToIndex[kHWControllerLEDRed]][kVNYUVMaxIndex] = kVNWandRedMax;
      // blue
      yuvColors_[colorToIndex[kHWControllerLEDBlue]][kVNYUVMinIndex] = kVNWandBlueMin;
      yuvColors_[colorToIndex[kHWControllerLEDBlue]][kVNYUVMaxIndex] = kVNWandBlueMax;
      // yellow
      yuvColors_[colorToIndex[kHWControllerLEDYellow]][kVNYUVMinIndex] = kVNWandYellowMin;
      yuvColors_[colorToIndex[kHWControllerLEDYellow]][kVNYUVMaxIndex] = kVNWandYellowMax;
      // cyan
      yuvColors_[colorToIndex[kHWControllerLEDCyan]][kVNYUVMinIndex] = kVNWandCyanMin;
      yuvColors_[colorToIndex[kHWControllerLEDCyan]][kVNYUVMaxIndex] = kVNWandCyanMax;
      // magenta
      yuvColors_[colorToIndex[kHWControllerLEDMagenta]][kVNYUVMinIndex] = kVNWandMagentaMin;
      yuvColors_[colorToIndex[kHWControllerLEDMagenta]][kVNYUVMaxIndex] = kVNWandMagentaMax;

      loadColors_ = false;
  }

  void
  VNWandPIMPL::CreateConfigFileIfNonExistent(void) {
    struct stat buffer;
    if (stat (kWandColorConfigurationFile, &buffer) != 0) {
      std::ofstream defaultConfigurationFile( kWandColorConfigurationFile );
      defaultConfigurationFile << std::string( kWandDefaultColorConfiguration );
      defaultConfigurationFile.close();
    }
  }

  void
  VNWandPIMPL::LoadColorFilterValues(void) {
    // check if configuration file exists and if not create a default one
    CreateConfigFileIfNonExistent();

    // load configuration
    std::ifstream configfile;
    configfile.open( kWandColorConfigurationFile );
    if( configfile.is_open() ) {
      LoadColorFilterValuesFromFile(configfile);

    } else {
      LoadColorFilterValuesFromDefaults();
    }

  }

  void
  VNWandPIMPL::SetColor(const LF::Hardware::HWControllerLEDColor color) {
    
    debugMPI_.DebugOut(kDbgLvlValuable, "VNWandPIMPL::SetColor %08x\n", (unsigned int)color);
    if (loadColors_) {
      LoadColorFilterValues();
    }
    
    LeapFrog::Brio::U8 index = color >> 1;
    yuvMin_ = yuvColors_[colorToIndex[color]][kVNYUVMinIndex];
    yuvMax_ = yuvColors_[colorToIndex[color]][kVNYUVMaxIndex];
    
    SetCameraTemperature(color);
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

    const char*
    VNWandPIMPL::GetBluetoothAddress() {
      return btaddress_.c_str();
    }

    void VNWandPIMPL::SetBluetoothAddress( const char* btaddress) {
        if (btaddress) {
	  btaddress_ = std::string(btaddress);
        }
    }

} // namespace Vision
} // namespace LF
