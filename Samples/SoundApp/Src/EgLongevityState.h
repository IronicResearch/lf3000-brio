#ifndef EGLONGEVITYSTATE_H_
#define EGLONGEVITYSTATE_H_


#include <GameStateBase.h>
#include <CBillboard.h>
#include <CText.h>



class EgLongevityState : public CGameStateBase
{
public:
	static EgLongevityState* Instance() { if (!mState) mState = new EgLongevityState(); return mState; }
	virtual void Suspend() {};
	virtual void Resume() {};
	virtual void Enter(CStateData* userData);
	virtual void Update(CGameStateHandler* sh);
	virtual void Exit();
	
private: 
	void prepare_gfx(void);
	void delete_gfx(void);
   void load_another_sprite(void);

protected:
	EgLongevityState();
	~EgLongevityState();
	

	
	CBillboard						* sprite;
	CText							   * direction;


	static EgLongevityState		* mState;
	

	
	
};


#endif /* EGLONGEVITYSTATE_H_ */

