#include <VNOcclusionTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNOcclusionTriggerPIMPL::VNOcclusionTriggerPIMPL(float percentOccludedToTrigger) :
    percentOccludedToTrigger_(percentOccludedToTrigger),
    percentOccluded_(0) {
    
  }
  
  VNOcclusionTriggerPIMPL::~VNOcclusionTriggerPIMPL(void) {
    
  }

  bool
  VNOcclusionTriggerPIMPL::Triggered(VNHotSpotPIMPL &hs) {
    cv::Mat img;
    int numPixels;
    numPixels = hs.GetTriggerImage(img);
    assert(img.type() == CV_8U);
    if (numPixels <= 0) return false;

    int numDiffPixels = 0;
    for (int i = 0; i < img.rows; ++i) {
      const VNPixelType* rowData = img.ptr<VNPixelType>(i);
      for (int j = 0; j < img.cols; ++j) {
	if (rowData[j] == kVNMaxPixelValue) {
	  numDiffPixels++;
	}
      }
    }
    
    // if the percentage of occluded pixels is greater than the threshold to trigger, return true.
    percentOccluded_ = (1.0f - (static_cast<float>(numPixels - numDiffPixels)/static_cast<float>(numPixels)));
    return (percentOccluded_ > percentOccludedToTrigger_);    
  }

} // namespace Vision
} // namespace LF
