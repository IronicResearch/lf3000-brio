#include <VNAlgorithmHelpers.h>
#include <DebugMPI.h>

namespace LF {
namespace Vision {

  LeapFrog::Brio::tControlInfo*
  FindCameraControl(const LeapFrog::Brio::tCameraControls &controls,
		    const LeapFrog::Brio::tControlType type) {
    LeapFrog::Brio::tControlInfo *c = NULL;
    for(LeapFrog::Brio::tCameraControls::const_iterator i = controls.begin();
	i != controls.end();
	++i) {

      c = *i;
      if (!c) {
	continue;
      }

      if (c->type == type) {
	return c;
      }
    }
    
    return NULL;
  }

  void
  ResetCamera(void) {
    LeapFrog::Brio::tCameraControls controls;
    LeapFrog::Brio::CCameraMPI cameraMPI;
    LeapFrog::Brio::CDebugMPI dbg(LeapFrog::Brio::kGroupVision);

    LeapFrog::Brio::Boolean err = cameraMPI.GetCameraControls(controls);
    dbg.Assert(err, "VNAlgorithmHelpers could get camera controls\n");

    // turn on autowhitebalance
    LeapFrog::Brio::tControlInfo *awb = FindCameraControl(controls,
							  LeapFrog::Brio::kControlTypeAutoWhiteBalance);
    if (awb) {
      cameraMPI.SetCameraControl(awb, 1);
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for auto white balance\n");
    }

    // turn on auto exposure
    LeapFrog::Brio::tControlInfo *ae = FindCameraControl(controls,
							 LeapFrog::Brio::kControlTypeAutoExposure);
    if (ae) {
      cameraMPI.SetCameraControl(ae, 3);
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for auto exposure\n");
    }

  }
}
}
