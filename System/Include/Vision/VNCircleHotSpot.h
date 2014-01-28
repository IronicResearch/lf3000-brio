#ifndef __INCLUDE_VISION_VNCIRCLEHOTSPOT_H__
#define __INCLUDE_VISION_VNCIRCLEEHOTSPOT_H__

#include <boost/shared_ptr.hpp>
#include <Vision/VNHotSpot.h>
#include <Vision/VNVisionTypes.h>

namespace LF {
namespace Vision {

  // forward declarations
  class VNVisionMPI;
  class VNCircleHotSpotPIMPL;
  class VNTrigger;

  /*!
   * \class VNCircleHotSpot
   *
   * \brief A VNCircleHotSpot is a hot spot that covers a circular region of the
   * framebuffer.  The circle is defined by a center,and a raidus.
   */

  class VNCircleHotSpot : public VNHotSpot {
  public:

    /*!
     * \brief Default Constructor
     * This constructor does not specify the equation for the circle, a developer
     * must set the circle before using the hot spot
     */
    VNCircleHotSpot(void);

    /*!
     * \biref Constructor
     * \param center The center of the circle in framebuffer coordinates
     * \param radius the radius of the circle
     */
    VNCircleHotSpot(const VNPoint &center,
		    float radius);

    /*!
     * \brief Default destructor
     */
    virtual ~VNCircleHotSpot(void);
    
    /*!
     * \brief Trigger the virtual method used to determine if this hot spot 
     * should be triggered in the current algorithmic cycle
     * \param input a cv::Mat reference containing the CV_8U binary image
     * representing the change that the hot spot should trigger against.
     */
    virtual void Trigger(cv::Mat &input) const;

    /*!
     * \brief SetCenter sets the center of the circle
     * \param center the center of the circle
     */
    void SetCenter(const VNPoint &center);

    /*!
     * \brief GetCenter returns the current circle center
     * \return the center of this hot spot
     */
    VNPoint GetCenter(void) const;

    /*!
     * \brief SetRadius set the radius of the circle
     * \param radius the radius of the circle
     */
    void SetRadius(float radius);

    /*!
     * \brief GetRadius returns the current value of the radius
     * \return the radius
     */
    float GetRadius(void) const;
    
  private:
    boost::shared_ptr<VNCircleHotSpotPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantic
     */
    VNCircleHotSpot(const VNCircleHotSpot& hotSpot);
    VNCircleHotSpot& operator=(const VNCircleHotSpot& hotSpot);

    /*!
     * friend classes
     */
    friend class VNVisionMPI;
    friend class VNTrigger;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNCIRCLEHOTSPOT_H__
