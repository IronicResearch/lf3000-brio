#ifndef EGPICKERSTATE_H_
#define EGPICKERSTATE_H_


#include <StringTypes.h>
#include <vector>
#include <GameStateHandler.h>
#include <KeyboardManager.h>
#include <CGraphics2D.h>
#include <ButtonMPI.h>
#include <EventMPI.h>
#include <GameStateBase.h>
#include <CBillboard.h>
#include <CText.h>
#include <LTM.h>
#include "boost/shared_ptr.hpp"

#define NUMBER_OF_ICONS		2

class EgIcon
{
public:
	EgIcon(LeapFrog::Brio::CPath path="", LeapFrog::Brio::CString text="", CGameStateBase* const state=NULL) 
		: imagePath(path), textString(text), pGameState(state) {};
	
	LeapFrog::Brio::CPath		imagePath;
	LeapFrog::Brio::CString		textString;
	CGameStateBase*				pGameState;
};

class EgPickerState : public CGameStateBase
{
public:
	static EgPickerState* Instance() { if (!mState) mState = new EgPickerState(); return mState; }
	virtual void Suspend() {};
	virtual void Resume() {};
	virtual void Enter(CStateData* userData);
	virtual void Update(CGameStateHandler* sh);
	virtual void Exit();
	
protected:
	EgPickerState();
	~EgPickerState();
	
	static EgPickerState *				mState;
	
	LeapFrog::Brio::U16 				currentSelection_;
	EgIcon						iconDataArray[NUMBER_OF_ICONS];
	CBillboard					iconBillboardArray[NUMBER_OF_ICONS];
	CText						iconTextArray[NUMBER_OF_ICONS];

	tPlayerID					old_id;
	LTM::CPlayerProfile 				*cp;
	LTM::CSystem 					cs;

	const char						*name_string;
	int						bitz;

	boost::shared_ptr<CText>			bitz_display;	
};



#endif // EGPICKERSTATE_H_

