#include <CameraTypes.h>
#include <CameraMPI.h>

namespace LF {
namespace Vision {

  // returns a pointer to the camera control of the type requested.
  // if that control is not available on the current camera it returns NULL
  LeapFrog::Brio::tControlInfo*
  FindCameraControl(const LeapFrog::Brio::tCameraControls &controls,
		    const LeapFrog::Brio::tControlType type);

  // resets the camera settings to the preset settings
  void
  ResetCamera(void);

} // namespace Vision
} // namespace LF
