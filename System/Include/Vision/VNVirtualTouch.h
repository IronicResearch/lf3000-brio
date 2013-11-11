#ifndef __INCLUDE_VISION_VNVIRTUALTOUCH_H__
#define __INCLUDE_VISION_VNVIRTUALTOUCH_H__

#include <Vision/VNAlgorithm.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  extern const float kVNDefaultVirtualTouchLearningRate;

  class VNVirtualTouchPIMPL;
  class VNVirtualTouch : public VNAlgorithm {
  public:
    VNVirtualTouch(float learningRate = kVNDefaultVirtualTouchLearningRate);
    virtual ~VNVirtualTouch(void);

    void SetLearningRate(float rate);
    float GetLearningRate(void) const;

    void Execute(void *input, void *output);
  private:
    boost::shared_ptr<VNVirtualTouchPIMPL> pimpl_;
  };
  

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNFGBGSEGMENTATIONALGORITHM_H__
