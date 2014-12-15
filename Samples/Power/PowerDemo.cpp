/******************************************************************************

 @Description  A very simple app that monitors the Power Interface

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <PowerMPI.h>
#include <EmulationConfig.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <boost/scoped_ptr.hpp>
LF_USING_BRIO_NAMESPACE()


//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// Power MPI event handler
//============================================================================
const tEventType kMyHandledTypes[] = { kAllPowerEvents };

#define NUM_POWER 8
const char *power_names[NUM_POWER] = {
	"kPowerNull",
	"kPowerExternal",
	"kPowerBattery",
	"kPowerLowBattery",
	"kPowerShutdown",
	"kPowerCritical",
	"kPowerRechargeable",
	"kPowerCharging"
};

//----------------------------------------------------------------------------
class MyPwrEventListener : public IEventListener
{
 public:
	MyPwrEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)),dbg_(kMyApp) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
		{
			dbg_.SetDebugLevel(kDbgLvlVerbose);
			type_ = msg.GetEventType();
			const CPowerMessage& pwrmsg = reinterpret_cast<const CPowerMessage&>(msg);
			tPowerState state = pwrmsg.GetPowerState();

			if (NUM_POWER <= state)		// state out of expected range
				dbg_.DebugOut(kDbgLvlVerbose, "unrecognised state=%d\n",
					state);
			else				// state known, show its name 
				dbg_.DebugOut(kDbgLvlVerbose, "%s \n",
			    	   power_names[state]);

			dbg_.DebugOut(kDbgLvlVerbose, "\n");

			return kEventStatusOKConsumed;
		}
	tEventType	type_;
	tPowerData	data_;
private:
 	CDebugMPI	dbg_;
};

int main()
{
	CDebugMPI dbg(kMyApp);

#ifdef EMULATION
	dbg.Assert(false, "This sample only runs on the target platform!");
#endif
	
	MyPwrEventListener handler;

	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "PowerDemo setup...timeout after 30 sec\n");

	CPowerMPI	pwrmgr;

	tPowerState state = pwrmgr.GetPowerState();
	if (NUM_POWER <= state)		// state out of expected range
		dbg.DebugOut(kDbgLvlVerbose, "unrecognised startup state=%d\n",
			state);
	else				// state known, show its name 
		dbg.DebugOut(kDbgLvlVerbose, "startup state is: %s \n",
	    	   power_names[state]);

	if (pwrmgr.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "Failed to register event handler.\n");
		return -1;
	}

	dbg.DebugOut(kDbgLvlVerbose, "PowerDemo loop: Change input power level\n");
	for (int i = 0; i < 30; i++) {
		/* Just hang around waiting for handler to be called. */
		sleep(1);
	}

	dbg.DebugOut(kDbgLvlVerbose, "PowerDemo done.\n");
	pwrmgr.UnregisterEventListener(&handler);
	return 0;
}
