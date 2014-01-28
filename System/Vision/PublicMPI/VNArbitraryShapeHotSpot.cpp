#include <Vision/VNArbitraryShapeHotSpot.h>
#include <VNArbitraryShapeHotSpotPIMPL.h>
#include <Vision/VNVisionTypes.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <EventMPI.h>

namespace LF {
namespace Vision {

  VNArbitraryShapeHotSpot::VNArbitraryShapeHotSpot(void) :
    VNRectHotSpot(),
    pimpl_(new VNArbitraryShapeHotSpotPIMPL() ){
    VNHotSpot::pimpl_ = pimpl_;
    VNRectHotSpot::pimpl_ = pimpl_;
  }
  
  VNArbitraryShapeHotSpot::VNArbitraryShapeHotSpot(const LeapFrog::Brio::tRect& rect,
						   const cv::Mat &filterImage) :
    VNRectHotSpot(rect),
    pimpl_(new VNArbitraryShapeHotSpotPIMPL(rect, filterImage) ){
    VNHotSpot::pimpl_ = pimpl_;
    VNRectHotSpot::pimpl_ = pimpl_;
  }

  VNArbitraryShapeHotSpot::VNArbitraryShapeHotSpot(const LeapFrog::Brio::tRect& rect,
						   const LeapFrog::Brio::tFontSurf &filterImage) :
    VNRectHotSpot(rect),
    pimpl_(new VNArbitraryShapeHotSpotPIMPL(rect, filterImage) ){
    VNHotSpot::pimpl_ = pimpl_;
    VNRectHotSpot::pimpl_ = pimpl_;
  }
  
  VNArbitraryShapeHotSpot::~VNArbitraryShapeHotSpot(void) {
    
  }
  
  void
  VNArbitraryShapeHotSpot::SetFilterImage(const cv::Mat &filterImage) {
    pimpl_->SetFilterImage(filterImage);
  }
  
  void
  VNArbitraryShapeHotSpot::SetFilterImage(const LeapFrog::Brio::tFontSurf &filterImage) {
    pimpl_->SetFilterImage(filterImage);
  }
  
  cv::Mat
  VNArbitraryShapeHotSpot::GetFilterImage(void) const {
    return pimpl_->GetFilterImage();
  }
    
} // namespace Vision
} // namespace LF
