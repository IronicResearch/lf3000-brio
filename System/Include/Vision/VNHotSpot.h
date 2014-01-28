#ifndef __INCLUDE_VISION_VNHOTSPOT_H__
#define __INCLUDE_VISION_VNHOTSPOT_H__

#include <boost/shared_ptr.hpp>
#include <DisplayTypes.h>
#include <Vision/VNTrigger.h>
#include <opencv2/opencv.hpp>

namespace LF {
namespace Vision {

  // forward declarations
  class VNOcclusionTrigger;
  class VNPointTrigger;
  class VNRectHotSpot;
  class VNCircleHotSpot;
  class VNArbitraryShapeHotSpot;
  class VNHotSpotPIMPL;

  /*!
   * \class VNHotSpot
   *
   * VNHotSpots are regions associated with the framebuffers from a live video feed from 
   * the camera.  VNHotSpots are used in conjunction with a VNAlgorithm to determine when
   * triggering events occur for a given hot spot.  
   *
   * Each VNHotSpot contains a pointer to a VNTrigger object which houses the logic for
   * determining when and how a hot spot is triggered.  Developers are able to register
   * and listen for hot spot triggering events via an IEventListener and the passing of
   * VNHotSpotEventMessages.
   *
   * When a VNHotSpot is situated such that part of the hot spot is off screen, only the
   * portion of the hot spot that is visible will be considered for triggering.  This
   * does not impact the VNPointTrigger in that if the point is inside of the portion
   * of the hot spot that is visible it will still trigger, however, if you are using a
   * VNOcclusionTrigger you should read the note in the VNOcclusionTrigger.h file as 
   * it will be harder to trigger a hot spot with an occlusion trigger when part of the
   * hot spot is not visible.
   *
   * This class can be subclassed by developers to create custom hot spots.  Just be sure
   * to immplement the Trigger method.
   */
  class VNHotSpot {
  public:
    
    /*!
     * Default constructor
     */
    VNHotSpot(void);

    /*!
     * Ddestructor
     */
    virtual ~VNHotSpot(void);



    /*!
     * \brief Trigger 
     * \param input a cv::Mat reference containing the CV_8U binary image
     * representing the change that the hot spot should trigger against.
     */
    virtual void Trigger(cv::Mat &input) const = 0;
    
    /*!
     * \brief IsTriggered
     * \return true if the hot spot is triggered, false otherwise
     */
    bool IsTriggered(void) const;
    
    /*!
     * \brief SetTrigger sets the logic for determining when the hot spot
     * gets triggered
     * \param trigger A VNTrigger that contains the logic to determine when
     * and how the hot spot is triggered
     */
    void SetTrigger(VNTrigger *trigger);

    /*!
     * \brief GetTrigger returns the current VNTrigger object used to determine 
     * when and how this hot spot gets triggered.
     * \return The VNTrigger* associated witht his hot spot
     */
    VNTrigger* GetTrigger(void) const;
    
    /*!
     * \brief SetTag allows developers to set custom identification tags.
     * By default, the VNHotSpot class uses a monotonically increasing U32 
     * value as the default tag for each new hot spot.  This allows each hot
     * spot to have a unique tag regardless of developer setting.  Keep in mind
     * it is posible to have multiple hot spots with the same tag, it is the 
     * responsibility of the developer to manage tags. 
     * \param tag a U32 identification tag
     */
    void SetTag(LeapFrog::Brio::U32 tag);

    /*!
     * \brief GetTag returns the current identification tag for this hot spot
     * \return The U32 identification tag used to identify this hot spot
     */
    LeapFrog::Brio::U32 GetTag(void) const;
    
  private:
    boost::shared_ptr<VNHotSpotPIMPL> pimpl_;
    
    /*!
     * Explicitly disable copy semantic
     */
    VNHotSpot(const VNHotSpot& hotSpot);
    VNHotSpot& operator=(const VNHotSpot& hotSpot);
    
    /*!
     * Friend classes
     */
    friend class VNOcclusionTrigger;
    friend class VNPointTrigger;
    friend class VNRectHotSpot;
    friend class VNCircleHotSpot;
    friend class VNArbitraryShapeHotSpot;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNHOTSPOT_H__
