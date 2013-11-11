#ifndef __INCLUDE_VISION_VNWANDTRACKER_H__
#define __INCLUDE_VISION_VNWANDTRACKER_H__

#include <Vision/VNAlgorithm.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  class VNWandTrackerPIMPL;
  class VNWandTracker : public VNAlgorithm {
  pubic:
    VNWandTracker(void);
    virtual ~VNWandTracker(void);

    VNWandTracker(const VNWandTracker& wt);
    VNWandTracker& operator=(const VNWandTracker& wt);

    // add getters and setters for necessary properties

    void Execute(void *input, void *output);
  private:
    boost::shared_ptr<VNWandTrackerPIMPL*> pimpl_;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNWANDTRACKER_H__
