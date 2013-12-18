#include <Vision/VNTrigger.h>
#include <VNTriggerPIMPL.h>

namespace LF {
namespace Vision {

  void
  VNTrigger::SetInputData(VNPoint point,
			  cv::Rect& rect,
			  cv::Mat& img) {
    pimpl_->SetInputData(point, rect, img);
  }

}
}
