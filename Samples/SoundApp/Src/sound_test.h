#ifndef         SOUNDTEST_H_
    #define     SOUNDTEST_H_


    #include    <GameStateBase.h>
    #include    <CBillboard.h>
    #include    <CText.h>



class SOUND_TEST : public CGameStateBase
{
public:
	static SOUND_TEST* Instance() { if (!mState) mState = new SOUND_TEST(); return mState; }
	virtual void Suspend() {};
	virtual void Resume() {};
	virtual void Enter(CStateData* userData);
	virtual void Update(CGameStateHandler* sh);
	virtual void Exit();
	
private: 
	void prepare_gfx(void);
	void delete_gfx(void);
	void prepare_sound_paths(void);
    void load_another_sprite(void);

protected:
	SOUND_TEST();
	~SOUND_TEST();
	
	CBillboard						* sprite;
	CText							   * direction;


	static SOUND_TEST		* mState;
	

	
	
};


#endif /* EGLONGEVITYSTATE_H_ */

