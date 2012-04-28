#include <CartridgeTypes.h>
#include <Utility.h>
#include <DebugMPI.h>

#define CART_BRIO_STATE  "/tmp/cart_brio_state"	

LF_BEGIN_BRIO_NAMESPACE()

static tCartridgeData gCartridgeData = {CARTRIDGE_STATE_CLEAN};

tCartridgeData GetCartridgeState(void)
{
	#if 0
	return gCartridgeData;
	#else
	int ret;
	FILE *fp;
	tCartridgeData data = {CARTRIDGE_STATE_CLEAN};
	CDebugMPI debug(kGroupCartridge);
	

	fp = fopen(CART_BRIO_STATE, "r");
	if(fp == NULL) {
		// debug.DebugOut(kDbgLvlImportant, "can't open %s for read !\n", CART_BRIO_STATE);
		return data;
	}
	ret = fscanf(fp, "%d", (int *)&data);
	if((ret == 0) || (ret == EOF)) {
		debug.DebugOut(kDbgLvlCritical, "data at %s: Format Wrong \n", CART_BRIO_STATE);
	} 
	fclose(fp);
	return data;
	#endif
}

void SetCartridgeState(tCartridgeData cartridge_data)
{
	gCartridgeData = cartridge_data;

	return;
}

LF_END_BRIO_NAMESPACE()
