#include <VNTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNTriggerPIMPL::VNTriggerPIMPL(void) {
    
  }
  
  VNTriggerPIMPL::~VNTriggerPIMPL(void) {
    
  }

  void
  VNTriggerPIMPL::SetInputData(VNPoint point,
			       cv::Rect& rect,
			       cv::Mat& img) {
    triggerPoint_ = point;
    triggerRect_ = rect;
    triggerImg_ = img;
  }
} // namespace Vision
} // namespace LF
