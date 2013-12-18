#ifndef __INCLUDE_VISION_VNTRIGGER_H__
#define __INCLUDE_VISION_VNTRIGGER_H__

#include <boost/shared_ptr.hpp>
#include <Vision/VNVisionTypes.h>
#include <opencv2/core/core.hpp>

namespace LF {
namespace Vision {
  
  // forward decleration
  class VNTriggerPIMPL;
  class VNOcclusionTrigger;
  class VNPointTrigger;

  /*!
   * \class VNTrigger
   *
   * Virtual base class for all trigger logic.  VNTrigger objects are used in conjunction
   * with VNHotSpots to determine if the hot spot was triggered.  Triggering events are 
   * based on the type of trigger and the current agorithm the VNVisionMPI is using.
   */
  class VNTrigger {
  public:

    /*!
     * \brief Triggered a virtual method used to determine if the hot spot associated with
     * this trigger logic has been triggered
     * \return true if triggered, false otherwise
     */
    virtual bool Triggered(void) = 0;

    virtual void SetInputData(VNPoint point,
			      cv::Rect& rect,
			      cv::Mat& img);
  private:
    boost::shared_ptr<VNTriggerPIMPL> pimpl_;

    friend class VNOcclusionTrigger;
    friend class VNPointTrigger;
  };
}
}

#endif // __INCLUDE_VISION_VNTRIGGER_H__
