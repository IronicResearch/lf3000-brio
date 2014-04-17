#include <VNAlgorithmHelpers.h>

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
      if (!c)
	continue;

      if (c->type == type)
	break;
    }
    
    return c;
  }

}
}
