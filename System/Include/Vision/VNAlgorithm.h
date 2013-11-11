#ifndef __INCLUDE_VISION_VNALGORITHM_H__
#define __INCLUDE_VISION_VNALGORITHM_H__

namespace LF {
namespace Vision {

class VNVirtualTouch;
  class VNAlgorithm {
  public:
    virtual void Execute(void *input, void *output) = 0;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNALGORITHM_H__
