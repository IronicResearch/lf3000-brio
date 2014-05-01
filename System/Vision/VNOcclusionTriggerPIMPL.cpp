#include <VNOcclusionTriggerPIMPL.h>

namespace LF {
namespace Vision {

  VNOcclusionTriggerPIMPL::VNOcclusionTriggerPIMPL(float percentOccludedToTrigger) :
    percentOccludedToTrigger_(percentOccludedToTrigger),
    percentOccluded_(0) {
    
  }
  
  VNOcclusionTriggerPIMPL::~VNOcclusionTriggerPIMPL(void) {
    
  }

  void
  VNOcclusionTriggerPIMPL::TriggerWithIntegralImage(VNHotSpotPIMPL &hs,
						    cv::Mat img,
						    int &numPixels,
						    int &numDiffPixels) {
    // NOTE: The assumption here is that the integral image will
    // only be used with rectangular hot spots as we can only 
    // compute the sum of a rectangular region using the integral image.
    // this means we can get the total number of pixels in the hot spot
    // by calculating the area of the bounding box.
    cv::Rect bb = hs.GetBoundingBox();
    numPixels = bb.width*bb.height;
    if (numPixels > 0) {
      // use -1 because the integral image is of size cols+1, rows+1
      cv::Rect imgRect(0,0,img.cols-1, img.rows-1);
      bb = bb & imgRect;
      // TODO: Insure using at is as good as other accessor methods with
      // respect to performance
      int tl = img.at<unsigned int>(bb.y, bb.x);
      int tr = img.at<unsigned int>(bb.y, bb.x+bb.width);
      int bl = img.at<unsigned int>(bb.y+bb.height, bb.x);
      int br = img.at<unsigned int>(bb.y+bb.height, bb.x+bb.width);
      
      numDiffPixels = (br-bl-tr+tl)/static_cast<int>(kVNMaxPixelValue);
    }
  }

  void
  VNOcclusionTriggerPIMPL::TriggerWithDiffImage(VNHotSpotPIMPL &hs,
						int &numPixels,
						int &numDiffPixels) {
    cv::Mat img;
    numPixels = hs.GetTriggerImage(img);
    assert(img.type() == CV_8U);
    
    if (numPixels > 0) {
      for (int i = 0; i < img.rows; ++i) {
	const VNPixelType* rowData = img.ptr<VNPixelType>(i);
	for (int j = 0; j < img.cols; ++j) {
	  if (rowData[j] == kVNMaxPixelValue) {
	    numDiffPixels++;
	  }
	}
      }    
    }  
  }

  bool
  VNOcclusionTriggerPIMPL::Triggered(VNHotSpotPIMPL &hs) {
    int numDiffPixels = 0;
    int numPixels = 0;
    cv::Mat img;
    bool useIntegral = hs.GetIntegralImage(img);
    if (useIntegral) {
      TriggerWithIntegralImage(hs, 
			       img, 
			       numPixels, 
			       numDiffPixels);
    } else {
      TriggerWithDiffImage(hs, 
			   numPixels, 
			   numDiffPixels);

    }

    if (numPixels <= 0) {
      percentOccluded_ = 0.f;
      return false;
    }

    // if the percentage of occluded pixels is greater than the threshold to trigger, return true.
    percentOccluded_ = (1.0f - (static_cast<float>(numPixels - numDiffPixels)/static_cast<float>(numPixels)));
    return (percentOccluded_ > percentOccludedToTrigger_);    

  }

} // namespace Vision
} // namespace LF
