#ifndef __INCLUDE_VISION_VNWAND_H__
#define __INCLUDE_VISION_VNWAND_H__

#include <Vision/VNVisionTypes.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  // forward declaration
  class VNWandPIMPL;
  class VNWandTracker;
  class VNVisionMPI;

  /*!
   * \class VNWand
   *
   * \brief The VNWand class is an object that represents a physical light wand
   * used for interaction with the console via the VNWandTracking algorithm.  
   */
  class VNWand {
  public:
    virtual ~VNWand(void);

    /*!
     * \return true if the wand is currently visible in the camera viewport
     */
    bool IsVisible(void) const;

    /*!
     * GetLocation
     * \return a VNPoint representing the x,y position in the camera viewport
     */
    VNPoint GetLocation(void) const;

  private:
    boost::shared_ptr<VNWandPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     */
    VNWand(const VNWand& wt);
    VNWand& operator=(const VNWand& wt);

    /*!
     * VNWand objects are handed out by the VNVisionMPI
     */
    VNWand(void);

    /*!
     * Friend classes
     */
    friend class VNWandTracker;
    friend class VNVisionMPI;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNWAND_H__
