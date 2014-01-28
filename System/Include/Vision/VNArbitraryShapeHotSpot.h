#ifndef __INCLUDE_VISION_VNARBITRARYSHAPEHOTSPOT_H__
#define __INCLUDE_VISION_VNARBITRARYSHAPEHOTSPOT_H__

#include <boost/shared_ptr.hpp>
#include <Vision/VNRectHotSpot.h>
#include <FontTypes.h>

namespace LF {
namespace Vision {

  // forward declarations
  class VNVisionMPI;
  class VNArbitraryShapeHotSpotPIMPL;
  class VNTrigger;

  /*!
   * \class VNArbitraryShapeHotSpot
   *
   * The VNArbitraryShapeHotSpot class allows for the creation of a hot spot that
   * has a completely arbitrary "shape".  The region of interest in the framebuffer is
   * specified via a rectangle and the shape of the hot spot is created by a filter
   * image, a binary cv::Mat image.  
   *
   * The type of image needs to be CV_8U, a single unsigned char per pixel.  
   * The mask will take any pixel values set to kVNMaxPixelValue (255) as part of the hot spot.  
   * All other values are treated as no part of the hot spot.
   */

  class VNArbitraryShapeHotSpot : public VNRectHotSpot {
  public:

    /*!
     * \brief Default Constructor
     * This constructor does not specify a rectangle to cover.  The developer must specify
     * the rectangle associated with this hot spot via \sa SetArbitraryShape
     */
    VNArbitraryShapeHotSpot(void);

    /*!
     * \biref Constructor
     * \param rect the rectangle specifying the region of the framebuffer to monitor
     *        for trigger events
     * \param filterImage the cv::Mat binary image used to mask the image inside of
     *        the rect to create an arbitrary shape.  Only pixel values of kVNMaxPixelValue (255)
     *        will be used in the filter and the incoming cv::Mat must be of type CV_8U
     */
    VNArbitraryShapeHotSpot(const LeapFrog::Brio::tRect& rect,
			    const cv::Mat &filterImage);

    /*!
     * \biref Constructor
     * \param rect the rectangle specifying the region of the framebuffer to monitor
     *        for trigger events
     * \param filterImage the LeapFrog::Brio::tFontSurf representing the filter mask.  The assumption
     *        is the tFontSurf will come from a CBlitBuffer object create by the developer.  By construction
     *        CBlitBuffers are of pixel format LeapFrog::Brio::kPixelFormatARGB8888.  This constructor
     *        will take the ARGB8888 image, ignore the alpha channel and only look at the RGB channels to 
     *        produce a binary image suitable for use in the filtering process of
     *        this hot spot.  All incoming RGB triplets of 255,255,255 will be included in the filtering mask
     *        while all other pixels will be excluded from the filter mask.
     */
    VNArbitraryShapeHotSpot(const LeapFrog::Brio::tRect& rect,
			    const LeapFrog::Brio::tFontSurf &filterImage);

    /*!
     * \brief Default destructor
     */
    virtual ~VNArbitraryShapeHotSpot(void);
    
    /*!
     * \brief SetFilterImage sets the binary image used as a filter mask to create the
     *        arbitrary hotspot shape.  The image must be of type CV_8U and the portion
     *        of the image to remain should have each pixel set to kVNMaxPixelValue
     *        (white/255)
     * \param filterImage the cv::Mat binary image used as the filter to create the 
     *        hotspot shape.
     */
    void SetFilterImage(const cv::Mat &filterImage);

    /*!
     * \brief SetFilterImage sets the binary image used as a filter mask to create the
     *        arbitrary hotspot shape.  The image must be of type CV_8U and the portion
     *        of the image to remain should have each pixel set to kVNMaxPixelValue
     *        (white/255)
     * \param filterImage the LeapFrog::Brio::tFontSurf representing the filter mask.  The assumption
     *        is the tFontSurf will come from a CBlitBuffer object create by the developer.  By construction
     *        CBlitBuffers are of pixel format LeapFrog::Brio::kPixelFormatARGB8888.  This constructor
     *        will take the ARGB8888 image, ignore the alpha channel and only look at the RGB channels to 
     *        produce a binary image suitable for use in the filtering process of
     *        this hot spot.  All incoming RGB triplets of 255,255,255 will be included in the filtering mask
     *        while all other pixels will be excluded from the filter mask.
     */
    void SetFilterImage(const LeapFrog::Brio::tFontSurf &filterImage);

    /*!
     * \brief GetFilterImage returns the cv::Mat image used as a filter mask to create
     *        the shape of this hot spot
     * \return the cv::Mat image used as a filter mask
     */
    cv::Mat GetFilterImage(void) const;

  private:
    boost::shared_ptr<VNArbitraryShapeHotSpotPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantic
     */
    VNArbitraryShapeHotSpot(const VNArbitraryShapeHotSpot& hotSpot);
    VNArbitraryShapeHotSpot& operator=(const VNArbitraryShapeHotSpot& hotSpot);

    /*!
     * friend classes
     */
    friend class VNVisionMPI;
    friend class VNTrigger;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNARBITRARYSHAPEHOTSPOT_H__
