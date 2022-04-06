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

    // get the temperature control
    LeapFrog::Brio::tControlInfo *temp = FindCameraControl(controls,
							   LeapFrog::Brio::kControlTypeTemperature);
    if (temp) {
      cameraMPI.SetCameraControl(temp, temp->preset); 
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for temperature\n");
    }

    // turn on autowhitebalance
    LeapFrog::Brio::tControlInfo *awb = FindCameraControl(controls,
							  LeapFrog::Brio::kControlTypeAutoWhiteBalance);
    if (awb) {
      cameraMPI.SetCameraControl(awb, 1);
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for auto white balance\n");
    }

    // set exposure to preset values
    LeapFrog::Brio::tControlInfo *e = FindCameraControl(controls,
							LeapFrog::Brio::kControlTypeExposure);
    if (e) {
      cameraMPI.SetCameraControl(e, e->preset);
    } else {
      dbg.DebugOut(LeapFrog::Brio::kDbgLvlCritical, "null camera control for exposure\n");
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

  bool CheckInputs(const cv::Mat& src,
		   cv::Mat& dst,
		   int type) {

    // initialize rgb image if not already initialized
    if (dst.empty()) {
      dst.create(cv::Size(src.cols, src.rows), type);
    }

    // check to see if different sizes, resize dst
    if (dst.cols != src.cols || dst.rows != src.rows) {
      dst.release();
      dst.create(cv::Size(src.cols, src.rows), type);
    }

    // explicitly exit if nothing to process
    if (((src.cols == 0) || (src.rows == 0)) || (dst.data == NULL) || (src.data == NULL)) {
      return false;
    }
    
    return true;
  }
}
}
