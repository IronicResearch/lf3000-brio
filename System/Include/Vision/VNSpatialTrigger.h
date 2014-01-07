#ifndef __INCLUDE_VISION_VNSPATIALTRIGGER_H__
#define __INCLUDE_VISION_VNSPATIALTRIGGER_H__

#include <Vision/VNTrigger.h>

namespace LF {
namespace Vision {
  
  /*!
   * \class VNSpatialTrigger
   *
   * VNSpatialTrigger is the base lass for a set of trigger classes that
   * have spatial awareness and cause hot spot triggering based on location
   * information. 
   *
   * At this time, a VNSpatialTrigger adds no new functionality on top of
   * the VNTrigger.  However, the distinction is necessary to create a clean
   * separation of logic for compound triggers.  
   */
  class VNSpatialTrigger : public VNTrigger {
  public:
    
  };
}
}

#endif // __INCLUDE_VISION_VNSPATIALTRIGGER_H__
