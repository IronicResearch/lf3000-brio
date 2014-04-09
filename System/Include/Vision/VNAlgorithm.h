#ifndef __INCLUDE_VISION_VNALGORITHM_H__
#define __INCLUDE_VISION_VNALGORITHM_H__

#include <CoreTypes.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  /*!
   * \class VNAlgorithm
   *
   * VNAlgorithm is the virtual base class for all vision library algorithms.
   * Each algorithm implements it's own Execute method that is responsible for
   * performing the algorithm, taking a cv::Mat as input and manipulating the
   * cv::Mat output.
   *
   * Developers can create their own algorithms derived off of VNAlgorithm as
   * long as they implement the Execute method.  If the custom algorithm will
   * interact with hotspots, the output cv::Mat must be of form CV_8UC and 
   * indicate where "change" has occured in the viewframe as this is used as
   * the basis for detecting if a hot spot has been triggered.
   */
  class VNVirtualTouch;
  class VNAlgorithm {
  public:
    /*!
     * \brief an optional initialization method that gets called during
     * VNVisionMPI::Start after the camera is setup and the capture session
     * has been started
     * \param frameProcessingWidth the width of the frame size the vision mpi
     * uses for processing
     * \param frameProcessingHeight the height of the frame size the vision mpi
     * uses for processing
     */
    virtual void Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
			    LeapFrog::Brio::U16 frameProcessingHeight);

    /*!
     * \brief Execute the virtual method called to execute the algorithm
     * \param input a reference to the input image in cv::Mat format
     * \param output a reference to the output binary image, used to determine
     *        hot spot triggering.  Must be of CV_8U format
     */
    virtual void Execute(cv::Mat &input, cv::Mat &output) = 0;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNALGORITHM_H__
