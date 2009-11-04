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

/* match defines in CartridgeTypes.h */
static const char *cartridgeStateName[] = {
	"CARTRIDGE_STATE_NONE",
	"CARTRIDGE_STATE_INSERTED",
	"CARTRIDGE_STATE_DRIVER_READY",	/* Internal state: driver is fully initialized */
	"CARTRIDGE_STATE_READY",
	"CARTRIDGE_STATE_REMOVED",
	"CARTRIDGE_STATE_FS_CLEAN",		/* Internal state: File system is fully unmounted */
	"CARTRIDGE_STATE_CLEAN",
	"CARTRIDGE_STATE_REINSERT",
	"CARTRIDGE_STATE_RESTART_APPMANAGER",
	"CARTRIDGE_STATE_REBOOT",
	"CARTRIDGE_STATE_UNKNOWN"
};

void SetCartridgeState(tCartridgeData cartridge_data)
{
	FILE *fp;
	gCartridgeData = cartridge_data;
	CDebugMPI debug(kGroupEvent);
	

	fp = fopen(CART_BRIO_STATE, "w");
	if(fp == NULL) {
		debug.DebugOut(kDbgLvlImportant, "can't access %s !\n", CART_BRIO_STATE);
		return;
	}
	if(fprintf(fp, "%d, %s\n", cartridge_data.cartridgeState, cartridgeStateName[cartridge_data.cartridgeState]) < 0) {
		debug.DebugOut(kDbgLvlImportant, "can't write to %s !\n", CART_BRIO_STATE);
	}
	fclose(fp);
	return;
}

LF_END_BRIO_NAMESPACE()
