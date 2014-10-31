#ifndef __INCLUDE_VISION_VNRECTHOTSPOT_H__
#define __INCLUDE_VISION_VNRECTHOTSPOT_H__

#include <boost/shared_ptr.hpp>
#include <Vision/VNHotSpot.h>

namespace LF {
namespace Vision {

  /*!
     * \class VNRectHotSpot
     *
     * NOTE: For use with LeapTV applications only.
     *
   */

  class VNVisionMPI;
  class VNArbitraryShapeHotSpot;
  class VNRectHotSpotPIMPL;
  class VNTrigger;

  class VNRectHotSpot : public VNHotSpot {
  public:

    /*!
     * \brief Default Constructor
     * This constructor does not specify a rectangle to cover.  The developer must specify
     * the rectangle associated with this hot spot via \sa SetRect
     */
    VNRectHotSpot(void);

    /*!
     * \biref Constructor
     * \param rect the rectangle specifying the region of the framebuffer to monitor
     * for trigger events.  The coordinates of rect must be with respect to the display
     * surface.
     */
    VNRectHotSpot(const LeapFrog::Brio::tRect& rect);

    /*!
     * \brief Default destructor
     */
    virtual ~VNRectHotSpot(void);

    /*!
     * \brief Trigger the virtual method used to determine if this hot spot
     * should be triggered in the current algorithmic cycle
     * \param input a cv::Mat reference containing the CV_8U binary image
     * representing the change that the hot spot should trigger against.
     */
    virtual void Trigger(cv::Mat &input) const;

    /*!
     * \brief SetRect sets the rectangle for this hot spot to monitor
     * \param rect the rectangle specifying the region of the framebuffer to monitor.
     * The coordinates of rect must be with respect to the display
     * surface.
     */
    void SetRect(const LeapFrog::Brio::tRect& rect);

    /*!
     * \biref GetRect returns the current rectangle this hot spot is tracking
     * \return the rectangle this hot spot is tracking
     */
    LeapFrog::Brio::tRect GetRect(void) const;

  private:
    boost::shared_ptr<VNRectHotSpotPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantic
     */
    VNRectHotSpot(const VNRectHotSpot& hotSpot);
    VNRectHotSpot& operator=(const VNRectHotSpot& hotSpot);

    /*!
     * friend classes
     */
    friend class VNVisionMPI;
    friend class VNTrigger;
    friend class VNArbitraryShapeHotSpot;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNRECTHOTSPOT_H__
