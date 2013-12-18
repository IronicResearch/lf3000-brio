#ifndef __INCLUDE_VISION_VNVIRTUALTOUCH_H__
#define __INCLUDE_VISION_VNVIRTUALTOUCH_H__

#include <Vision/VNAlgorithm.h>
#include <boost/shared_ptr.hpp>

namespace LF {
namespace Vision {

  /*!
   * kVNDefaultVirtualTouchLearningRate
   * The default learning rate used when no learning rate is specified
   */
  extern const float kVNDefaultVirtualTouchLearningRate;

  // foprward declaration
  class VNVirtualTouchPIMPL;

  /*!
   * \class VNVirtualTouch
   *
   * VNVirtualTouch is a foreground/background segmentation algorithm that gives the
   * VNVisionMPI the ability to determine the foreground object.  The VNVirtualTouch
   * algorithm uses a learning rate that affects how fast the background is updated.
   * Moving objects are part of the foreground.  An object that was moving, and part 
   * of the foregroud, will become part of the background at rate proportional to
   * the learning rate.  This larger the value the fast an object moves to the background.
   * The smaller the learning rate the slower an object moved to the background.
   */
  class VNVirtualTouch : public VNAlgorithm {
  public:
    
    /*!
     * \brief Constuctor
     * \param learningRate the value of the learning rate for this algorithm
     */
    VNVirtualTouch(float learningRate = kVNDefaultVirtualTouchLearningRate);

    /*!
     * \brief Default destructor
     */
    virtual ~VNVirtualTouch(void);

    /*!
     * \brief SetLearningRate Valid ranges are 0.001 to 1.0 
     * the large the value of the learning rate the faster an object, that was moving,
     * will move in to the background.
     * \param rate is the learning rate that is used to update the evolving background
     * \return the actual value of the learning rate after attempting to set the value to rate
     */
    float SetLearningRate(float rate);

    /*!
     * \brief GetLearningRate returns the current value of the learning rate
     * \return the learning rate as a float
     */
    float GetLearningRate(void) const;

    /*!
     * \brief Execute the virtual method called to execute the algorithm
     * \param input a void* to the input data
     * \param output a void* to the output data
     */
    void Execute(void *input, void *output);
  private:
    boost::shared_ptr<VNVirtualTouchPIMPL> pimpl_;

    /*!
     * Explicitly disable copy semantics
     */
    VNVirtualTouch(const VNVirtualTouch&);
    VNVirtualTouch& operator =(const VNVirtualTouch&);
  };
  

} // namespace Vision
} // namespace LF

#endif // __INCLUDE_VISION_VNFGBGSEGMENTATIONALGORITHM_H__
