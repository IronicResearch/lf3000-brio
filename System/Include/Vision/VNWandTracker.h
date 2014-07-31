#ifndef __INCLUDE_VISION_VNWANDTRACKER_H__
#define __INCLUDE_VISION_VNWANDTRACKER_H__

#include <Vision/VNAlgorithm.h>
#include <Vision/VNVisionTypes.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  /*!
   * kVNAreaToStartScalingKey
   * The string key used to override the default wand area (in pixels)
   * when scaling begins.  In this case scaling referes to the scaling
   * of the distance the LED moves to it's position on the screen.  Typical
   * usage of this is to reduce the physical space the LED must move in to
   * traverse the entire screen.  \sa SetAutomaticWandScaling
   */
  extern const LeapFrog::Brio::CString kVNAreaToStartScalingKey;

  /*!
   * kVNMinPercentToScaleKey
   * The string key used to override the default minimum percent to scale
   * the wand to.  This refers to the size of the scaled input box relative to 
   * the size of the screen dimensions. \sa SetAutomaticWandScaling
   */
  extern const LeapFrog::Brio::CString kVNMinPercentToScaleKey;

  /*!
   * kVNMinWandAreaKey
   * They key used to override the default minimum wand area (in pixels).  If the
   * VNWandTracker Algorithm finds an LED that is smaller than the sie specified with
   * this paramter it will ignore it as if there is no LED present on the screen.
   */
  extern const LeapFrog::Brio::CString kVNMinWandAreaKey;

  /*!
   * kVNUseWandSmoothingKey
   * This is the key for turning on wand smoothing.  A corresponding value that is
   * greater than or equal to 1.0 will turn on wand smoothing, any other value will not.
   */
  extern const LeapFrog::Brio::CString kVNUseWandSmoothingKey;

  /*!
   * kVNNumFramesToCacheLocationKey
   * The key used to set the number of frames to cache the last known location 
   * of the wand LED.  If wand smoothing is on, and the wand is no longer visible
   * by the VNWandTracker algorithm, either by occlusion, off screen or noise, the
   * location reported for the LED will be the last known location for n frames where
   * n is the value associated with this key in the input params.
   */
  extern const LeapFrog::Brio::CString kVNNumFramesToCacheLocationKey;

  /*!
   * kVNWandSmoothingAlphaKey
   * The key used to override the default wand smoothing alpha value.  If wand smoothing
   * is on, the location of the LED is reported as: 
   * alpha*currentLocation + (1.0-alpha)*previousLocation
   * The value associated with this key is used as alpha as long as it lies in (0,1)
   */
  extern const LeapFrog::Brio::CString kVNWandSmoothingAlphaKey;

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
  class VNVisionMPIPIMPL;
  class VNWandTrackerPIMPL;
  class VNWandTracker : public VNAlgorithm {
  public:
    /*!
     * Default constructor
     * \param params an optional list of input paramters to adjust various
     * aspects of the VNWandTracker algorithm.  Mostly intended for debugging.
     */
    VNWandTracker(VNInputParameters *params = NULL);

    /*!
     * Default destructor
     */
    virtual ~VNWandTracker(void);

    /*!
     * Virtual method to perform algorithm specific initialization
     * The wand tracking algorithm adjusts camera settings to better
     * optimize the ability to track the wand.
     * \param frameProcessingWidth the width of the frame size the vision mpi
     * uses for processing
     * \param frameProcessingHeight the height of the frame size the vision mpi
     * uses for processing
     */
    virtual void Initialize(LeapFrog::Brio::U16 frameProcessingWidth,
			    LeapFrog::Brio::U16 frameProcessingHeight);

    /*!
     * Virtual method to execute wand tracking algorithm
     * \param input a cv::Mat reference, the current frame from the camera
     * \param output a cv::Mat reference, the output image used for detecting
     *        change.  Must be of type CV_8U
     */
    virtual void Execute(cv::Mat &input, cv::Mat &output);

    /*!
     * \brief In this algorithm Shutdown resets the camera settings for
     * normal camera operations
     */
    virtual void Shutdown(void);

    /*!
     * \brief Wand Scaling refers to the mechanism that allows the WandTracker algorithm
     * to scale the physical space the wand moves in to the display space.  As a child moves
     * further away from the camera the physical space the camera sees is larger than
     * when closer to the camera.  If the wand input is not scaled the child would have
     * to physically move further side-to-side and up-and-down in order to reach the
     * edges of the display space.  Scaling the wand input is the mechanism that
     * scales smaller physical movement to larger display movement. 
     *
     * When automatic wand scaling is turned on, by calling this method with autoScale
     * set to true, the WandTracker algorithm will scale the physical input space based
     * on a the relative distance the wand is from the camera.  The benefit of turning 
     * on automatic wand scaling is the child can traverse the entire display using the
     * wand with much smaller physical movements.  A potential downside to scaling the
     * wand input is that the relative movement of the pointer is faster when the child
     * is further from the camera than when they are closer. 
     *
     * To further illustrate this functionality the following graphic depicts how this 
     * behavior works.  On the right is what we will refer to as Display Space, the 
     * rectangle that represent the display surface that is shown on screen.  Vision
     * Space refers to the rectangle that represents what the camera sees.  Sub-Vision
     * Space refers to the sub-rectangle of Vision Space that is used to scale the wand
     * input.
     *
     *                                 *---------------------------------*
     *                                 |                                 |
     *                                 |                                 |
     *    *--------------------*       |                                 |
     *    |   Vision Space     |       |        Display Space            |
     *    |  *--------------*  |       |                                 |
     *    |  | Sub-         |  |       |                                 |
     *    |  | Vision Space |  |       |                                 |
     *    |  *--------------*Ps|       |                                 |
     *    |                    |       |                                 |
     *    *--------------------*Pv     |                                 |
     *                                 |                                 |
     *                                 *---------------------------------*Pd
     *
     * In this small dipiction, when wand scaling is turned off, the vision 
     * pooint Pv will map to the display point Pd.  When wand scaling is turned on the
     * sub vision point Ps will map to the display point Pd even thought he camera still
     * see the entire vision space.
     *
     * \param autoScale when set to true automatic wand scaling is turned on
     */
    void SetAutomaticWandScaling(bool autoScale = false);

    /*!
     * \return true if automatic wand scaling is turned on
     */
    bool GetAutomaticWandScaling(void) const;

  private:
    boost::shared_ptr<VNWandTrackerPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     */
    VNWandTracker(const VNWandTracker& wt);
    VNWandTracker& operator=(const VNWandTracker& wt);

    friend class VNVisionMPIPIMPL;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNWANDTRACKER_H__
