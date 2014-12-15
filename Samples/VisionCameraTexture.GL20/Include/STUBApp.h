#ifndef STUBAPP_H_
#define STUBAPP_H_

#include <AppInterface.h>
#include <DebugMPI.h>
#include <GameStateHandler.h>
#include <LTM.h>

#define DEBUG_LEVEL kDbgLvlNoteable
//#define DEBUG_LEVEL kDbgLvlVerbose
/*
 * Debug levels:
 * kDbgLvlSilent
 * kDbgLvlCritical
 * kDbgLvlImportant
 * kDbgLvlValuable
 * kDbgLvlNoteable
 * kDbgLvlVerbose
 * kMaxDebugLevel
 */

class STUBApp : public CAppInterface
{
public:
	STUBApp(void *userData);
	~STUBApp();

	void Enter();
	void Update();
	void Exit();

	void Suspend();
	void Resume();

	static void RequestNewState(CGameStateBase* state);

	LeapFrog::Brio::U32	getProductID()		{ return productID_; }
	char *				getPartNumber()		{ return const_cast<char *>(partNumber_.c_str()); }
	char *				getProductVersion()	{ return const_cast<char *>(pkgVersion_.c_str()); }
	LeapFrog::Brio::U8	getLogDataVersion()	{ return logDataVersion_; }
	tPlayerID			getPlayerID()		{ return playerID_; }
	LTM::CPlayerProfile*	getPlayerProfile()	{ return playerProfile_;}


protected:
	LeapFrog::Brio::CDebugMPI			debugMPI_;
	CGameStateHandler *					stateHandler_;

	static bool							newState_;
	static CGameStateBase*				newStateRequested_;
	static CGameStateBase*				currentState_;

	LeapFrog::Brio::U32			productID_;
	LeapFrog::Brio::CString		partNumber_;
	LeapFrog::Brio::CString		pkgVersion_;
	LeapFrog::Brio::U8			logDataVersion_;
	tPlayerID					playerID_;
	LTM::CPlayerProfile*		playerProfile_;

};


#endif // STUBAPP_H_

