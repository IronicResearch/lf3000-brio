#ifndef __INCLUDE_HARDWARE_HWCONTROLLERMPIPIMPL_H__
#define __INCLUDE_HARDWARE_HWCONTROLLERMPIPIMPL_H__

#include <Hardware/HWControllerTypes.h>
#include <EventListener.h>
#include <EventMPI.h>
#include <BluetopiaIO.h>
#include <vector>

namespace LF {
namespace Hardware {

  class HWController;

  /*!    
   * \class HWControllerMPIPIMPL
   */
  class HWControllerMPIPIMPL : public LeapFrog::Brio::IEventListener {
  public:
    static HWControllerMPIPIMPL* Instance(void);
    virtual ~HWControllerMPIPIMPL(void);
    
    HWController* GetControllerByID(LeapFrog::Brio::U32 id);
    void GetAllControllers(std::vector<HWController*> &controller);
    LeapFrog::Brio::U8 GetNumberOfConnectedControllers(void) const;
    void RegisterSelfAsListener(void);

    virtual LeapFrog::Brio::tEventStatus Notify(const LeapFrog::Brio::IEventMessage &msg);
    
  private:
    HWControllerMPIPIMPL(void);
    HWControllerMPIPIMPL(const HWControllerMPIPIMPL&);
    HWControllerMPIPIMPL& operator=(const HWControllerMPIPIMPL&);

    LeapFrog::Brio::CEventMPI eventMPI_;

  };
  
}	// namespace Hardware
}	// namespace LF

#endif // __INCLUDE_HARDWARE_HWCONTROLLERMPIPIMPL_H__
