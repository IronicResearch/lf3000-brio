#ifndef __INCLUDE_VISION_VNTRIGGER_H__
#define __INCLUDE_VISION_VNTRIGGER_H__

namespace LF {
namespace Vision {

  // forward decleration
  class VNHotSpot;

  /*!
   * \class VNTrigger
   *
   * NOTE: For use with LeapTV applications ONLY.
   *
   * \brief Virtual base class for all trigger logic.  VNTrigger objects are used in conjunction
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
    virtual bool Triggered(const VNHotSpot *hotSpot) = 0;
  };
}
}

#endif // __INCLUDE_VISION_VNTRIGGER_H__
