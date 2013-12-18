#ifndef __INCLUDE_VISION_VNWANDTRACKER_H__
#define __INCLUDE_VISION_VNWANDTRACKER_H__

#include <Vision/VNAlgorithm.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  /*!
   * \class VNWandTracker
   *
   * \brief The VNWandTracker class is a discrete algorithm used to track the 
   * end of the LF wand, a colored light source.  The basic algorithm filters
   * the input video frames based on a specific hue, based on the light color,
   * then thresholds the resulting image based on saturation and intensity or
   * brightness.
   *
   * The binary image produced from this filtering/thresholding process results
   * in the detection of the light source in the current video frame.  This is then
   * used to determine the center of this light source, and therefore update the
   * current location of the wand "pointer" on the screen.
   *
   * Currently the hue, saturation and intensity are fixed for the specific 
   * LF wand light color.
   */
  class VNWandTrackerPIMPL;
  class VNWandTracker : public VNAlgorithm {
  public:
    /*!
     * Default constructor
     */
    VNWandTracker(void);

    /*!
     * Default destructor
     */
    virtual ~VNWandTracker(void);

    /*!
     * Virtual method to execute wand tracking algorithm
     * \param input a void* to the input data
     * \param output a void* to the output data
     */
    void Execute(void *input, void *output);
  private:
    boost::shared_ptr<VNWandTrackerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     */
    VNWandTracker(const VNWandTracker& wt);
    VNWandTracker& operator=(const VNWandTracker& wt);
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNWANDTRACKER_H__
