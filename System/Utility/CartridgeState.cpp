#include <CartridgeTypes.h>
#include <Utility.h>

LF_BEGIN_BRIO_NAMESPACE()

static tCartridgeData gCartridgeData;

tCartridgeData GetCartridgeState(void)
{
	tCartridgeData data = gCartridgeData;
	return data;
}

void SetCartridgeState(tCartridgeData cartridge_data)
{
	gCartridgeData = cartridge_data;
}

LF_END_BRIO_NAMESPACE()
