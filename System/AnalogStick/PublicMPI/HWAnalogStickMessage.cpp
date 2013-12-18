#include <Hardware/HWAnalogStickTypes.h>

namespace LF {
namespace Hardware {

  HWAnalogStickMessage::HWAnalogStickMessage(const tHWAnalogStickData& data)
    : IEventMessage(kHWAnalogStickDataChanged), 
      data_(data) {
  }  
  
  LeapFrog::Brio::U16
  HWAnalogStickMessage::GetSizeInBytes(void) const {
    return sizeof(HWAnalogStickMessage);
  }
  
  tHWAnalogStickData 	
  HWAnalogStickMessage::GetAnalogStickData(void) const {
    return data_;
  }
  
} // namesapce Hardware
} // namespace LF
