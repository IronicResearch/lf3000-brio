#ifndef __INCLUDE_VISION_VNALGORITHM_H__
#define __INCLUDE_VISION_VNALGORITHM_H__

namespace LF {
namespace Vision {

  /*!
   * \class VNAlgorithm
   *
   * VNAlgorithm is the virtual base class for all vision library algorithms.
   * Each algorithm implements it's own Execute method that is responsible for
   * performing the algorithm, taking a (void*) as input and manipulating the
   * (void*) output.
   */
  class VNVirtualTouch;
  class VNAlgorithm {
  public:
    /*!
     * \brief Execute the virtual method called to execute the algorithm
     * \param input a void* to the input data
     * \param output a void* to the output data
     */
    virtual void Execute(void *input, void *output) = 0;
  };

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNALGORITHM_H__
