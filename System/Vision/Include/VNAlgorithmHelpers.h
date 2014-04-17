#include <CameraTypes.h>
#include <CameraMPI.h>

namespace LF {
namespace Vision {

  LeapFrog::Brio::tControlInfo*
  FindCameraControl(const LeapFrog::Brio::tCameraControls &controls,
		    const LeapFrog::Brio::tControlType type);

} // namespace Vision
} // namespace LF
