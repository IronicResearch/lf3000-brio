#ifndef EGAPP_H_
#define EGAPP_H_

#include <AppInterface.h>
#include <CGraphics2D.h>
#include <CBillboard.h>
#include <DebugMPI.h>
#include <KeyboardManager.h>
#include <GameStateHandler.h>


class EgApp : public CAppInterface 
{
public:
	EgApp(void *userData);
	~EgApp();
	
	void Enter();
	void Update();
	void Exit();

	void Suspend() {}
	void Resume() {}
	
	static void RequestNewState(CGameStateBase* state);
	
	// Note:  These are just made up values.  An actual title would have these defined somewhere in the process.
	static LeapFrog::Brio::U32	getProductID()		{ return 0xBEEF; }
	static char *				getPartNumber()		{ return "LEG"; }
	static char *				getProductVersion()	{ return "0.1"; }
	static LeapFrog::Brio::U8	getLogDataVersion()	{ return 0; }
	
protected:
	LF_ADD_BRIO_NAMESPACE(CDebugMPI)	debugMPI_;
//	CGraphics2D *						pGraphicsLib_;
	KeyboardManager *					pKeyManager_;
	CGameStateHandler					stateHandler_;
	
	static bool							newState_;
	static CGameStateBase*				newStateRequested_;

};


#endif // EGAPP_H_

