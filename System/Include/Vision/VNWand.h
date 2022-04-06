#ifndef __INCLUDE_VISION_VNWAND_H__
#define __INCLUDE_VISION_VNWAND_H__
/////////////////////
 /** DEPRECATED **/
/////////////////////

#include <Vision/VNVisionTypes.h>
#include <boost/shared_ptr.hpp>

class TestWand;


namespace LF {

namespace Hardware {
  class HWControllerPIMPL;
}

namespace Vision {

  // forward declaration
  class VNWandPIMPL;
  class VNWandTrackerPIMPL;
  class VNVisionMPI;


  /*!
   * \class VNWand
   *
   * ** DEPRECATED **
   * This class is deprecated as there is no means in the API to obtain or create an instance of 
   * VNWand.  In a future SDK release this file will be removed from the API/SDK.
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
     * with respect to the display surface coordinate system
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
    friend class VNWandTrackerPIMPL;
    friend class VNVisionMPI;
    friend class LF::Hardware::HWControllerPIMPL;
	  friend class ::TestWand;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNWAND_H__
