#ifndef __INCLUDE_HARDWARE_HWCONTROLLERMPIPIMPL_H__
#define __INCLUDE_HARDWARE_HWCONTROLLERMPIPIMPL_H__

#include <Hardware/HWControllerTypes.h>
#include <EventListener.h>
#include <EventMPI.h>
#include <BluetopiaIO.h>
#include <vector>
#include <map>

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

    void ScanForDevices(void);
    void AddController(char* link);

    int numControllers_;
    std::vector<HWController*> listControllers_;
    std::map<std::string, HWController*> mapControllers_;
    bool isScanning_;

    LeapFrog::Brio::CEventMPI eventMPI_;

    void* dll_;
    int handle_;
    pFnInit	    		pBTIO_Init_;
    pFnExit 			pBTIO_Exit_;
    pFnSendCommand		pBTIO_SendCommand_;
    pFnQueryStatus		pBTIO_QueryStatus_;
    pFnScanForDevices	pBTIO_ScanDevices_;

    static void DeviceCallback(void*, void*, int);
    static void InputCallback(void*, void*, int, char*);
    static void ScanCallback(void*, void*, int);

    friend class HWControllerPIMPL;
    friend class HWControllerBluetoothPIMPL;
  };
  
}	// namespace Hardware
}	// namespace LF

#endif // __INCLUDE_HARDWARE_HWCONTROLLERMPIPIMPL_H__
