#include <VNOcclusionTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNOcclusionTriggerPIMPL::VNOcclusionTriggerPIMPL(float percentOccludedToTrigger) :
    percentOccludedToTrigger_(percentOccludedToTrigger) {
    
  }
  
  VNOcclusionTriggerPIMPL::~VNOcclusionTriggerPIMPL(void) {
    
  }

  bool
  VNOcclusionTriggerPIMPL::Triggered(void) const {
    assert(triggerImg_.type() == CV_8U);
    
    int numDiffPixels = 0;
    for (int i = 0; i < triggerImg_.rows; ++i) {
      const uchar* rowData = triggerImg_.ptr<uchar>(i);
      for (int j = 0; j < triggerImg_.cols; ++j) {
	if (rowData[j] == static_cast<uchar>(0)) {
	  numDiffPixels++;
	}
      }
    }
    
    int numPixels = triggerImg_.rows*triggerImg_.cols;
    // if the percentage of occluded pixels is greater than the threshold to trigger, return true.
    return ((1.0f - (static_cast<float>(numPixels - numDiffPixels)/static_cast<float>(numPixels))) > percentOccludedToTrigger_);
  }

} // namespace Vision
} // namespace LF
