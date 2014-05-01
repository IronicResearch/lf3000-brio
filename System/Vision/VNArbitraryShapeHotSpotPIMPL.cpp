#include <VNArbitraryShapeHotSpotPIMPL.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNWand.h>
#include <Vision/VNTrigger.h>
#include <opencv2/opencv.hpp>
#include <cmath>

namespace LF {
namespace Vision {

  VNArbitraryShapeHotSpotPIMPL::VNArbitraryShapeHotSpotPIMPL(void) :
    VNRectHotSpotPIMPL() {
  }
  
  VNArbitraryShapeHotSpotPIMPL::VNArbitraryShapeHotSpotPIMPL(const LeapFrog::Brio::tRect& rect,
							     const cv::Mat &filterImage) :
    VNRectHotSpotPIMPL(rect) {
    SetFilterImage(filterImage);
  }
  
  VNArbitraryShapeHotSpotPIMPL::VNArbitraryShapeHotSpotPIMPL(const LeapFrog::Brio::tRect& rect,
							     const LeapFrog::Brio::tFontSurf &filterImage) :
    VNRectHotSpotPIMPL(rect) {
    SetFilterImage(filterImage);
  }
  
  VNArbitraryShapeHotSpotPIMPL::~VNArbitraryShapeHotSpotPIMPL(void) {
  }

  void
  VNArbitraryShapeHotSpotPIMPL::UpdateVisionCoordinates(void) {
    // there is some redundant behavior in calling UpdateVisionCoordinates on
    // the parent class and then setting the rect (in the parent class) however
    // we would miss another update if one gets added to the parent class 
    //inside of UpdateVisionCoordinates
    VNRectHotSpotPIMPL::UpdateVisionCoordinates();
   
    filterImage_.create(rect_.height, // number of rows - height
			rect_.width,  // number of cols - width
			CV_8U);
    
    float sfx = translator_->GetDisplayToVisionXSF();
    float sfy = translator_->GetDisplayToVisionYSF();
    float relativeScale = sqrtf(sfx*sfx + sfy*sfy);

    // set interpolation based on if the image is (generally speaking)
    // enlarging or shrinking when going from display to vision coordinates
    int interpolation = (relativeScale > 1.0f) ? cv::INTER_CUBIC : cv::INTER_AREA;

    cv::resize(origFilterImage_, 
	       filterImage_, 
	       filterImage_.size(), 
	       0, // scales to the size already set
	       0, // scales to the size already set
	       interpolation);
    UpdateNumberOfActivePixels(filterImage_);
  }
  
  void
  VNArbitraryShapeHotSpotPIMPL::SetFilterImage(const cv::Mat &filterImage) {
    assert(filterImage.type() == CV_8U);
    filterImage.copyTo(origFilterImage_);
    UpdateVisionCoordinates();
  }

  void
  VNArbitraryShapeHotSpotPIMPL::SetFilterImage(const LeapFrog::Brio::tFontSurf &filterImage) {
    assert(filterImage.format == LeapFrog::Brio::kPixelFormatARGB8888);
    LeapFrog::Brio::U8 *buffer = filterImage.buffer;
    assert(buffer);
    int bpp = 4; // we are assuming 4 bytes per pixel, ARGB format
    cv::Mat binaryMask(filterImage.width, filterImage.height, CV_8U);

    // only accept white pixels
    for (int i = 0; i < filterImage.height; ++i) {
      
      int offset = i*filterImage.pitch;
      unsigned char *data = binaryMask.ptr(i);
      for (int j = 0; j < filterImage.width; ++j) {
	if (buffer[offset+j*bpp+1] == kVNMaxPixelValue &&
	    buffer[offset+j*bpp+2] == kVNMaxPixelValue &&
	    buffer[offset+j*bpp+2] == kVNMaxPixelValue) {
	  data[j] = kVNMaxPixelValue;
	} else {
	  data[j] = kVNMinPixelValue;
	}
      }
    }
    SetFilterImage(binaryMask);
  }

  cv::Mat
  VNArbitraryShapeHotSpotPIMPL::GetFilterImage(void) const {
    return origFilterImage_;
  }

  void
  VNArbitraryShapeHotSpotPIMPL::Trigger(cv::Mat &input, const VNHotSpot *hs) {
    if (trigger_ != NULL) {
      if (input.rows > 0 && input.cols > 0) {
	// obtain the rectangle that is the intersection of the current rect_ and
	// the rectangle produced by the bounds of the input image (0,0,width,height)
	cv::Rect clippedRect = ClipRectToImage(input);

	// we need a clipping rectangle for the mask with the appropriate origin and
	// size based on the clippedRect.  To achieve this we will shift the clipped
	// rect by the origin of the original rect
	cv::Rect adjustedClippedRect = clippedRect - rect_.tl();

	// clip the filtering image such that when we apply the filter
	// to the input image we are applying it in the correct location
	cv::Mat clippedFilter = filterImage_(adjustedClippedRect);

	// NOTE: If we want to calculate the percentage of occlusion based only on
	// the visible portion of the hot spot we should uncomment this line of
	// code and use only the active pixels that are visible
	// because the filter is clipped we need to update the number of active
	// pixels in the final trigger image
	//UpdateNumberOfActivePixels(clippedFilter);

	tmpMat_ = (input)(clippedRect);
	tmpMat_.copyTo(triggerImage_, clippedFilter);
      }
      isTriggered_ = trigger_->Triggered(hs);      
    }
  }

  bool
  VNArbitraryShapeHotSpotPIMPL::ContainsPoint(const VNPoint &p) const {
    // need to check and see if the point lies atop one of the
    // active filter image pixels

    // first check to see if the point is witin our rectangle of interest
    if (rect_.contains(cv::Point(p.x, p.y))) {

      // the point is potentially contained within our filter image
      //
      // get a local reference to the point p such that if the point p were
      // at the top left corner of the rectangle our local version would
      // be at (0,0)
      VNPoint localP(p.x - rect_.x, p.y - rect_.y);
      if (localP.x <= filterImage_.cols && 
	  localP.y <= filterImage_.rows) {
	const VNPixelType* rowData = filterImage_.ptr<VNPixelType>(static_cast<int>(localP.y));
	if (rowData[static_cast<int>(localP.x)] == kVNMaxPixelValue)
	  return true;
      }
    }
    return false;
  }

  int
  VNArbitraryShapeHotSpotPIMPL::GetTriggerImage(cv::Mat &img) {
    img = triggerImage_;
    return numPixels_;
  }

  bool
  VNArbitraryShapeHotSpotPIMPL::GetIntegralImage(cv::Mat &img) {
    // do not use integral image
    return false;
  }

} // namespace Vision
} // namespace LF
