// BrioAudio.cpp
//
// Test playing multiple audio files triggered by key presses

/*
Button Mappings:
	Up				= Sine500Hz
	Down			= Sine1K
	Left			= Sine2K
	Right			= Sine5K
	A				= WhiteNoise6db
	B				= SFX
	Hint			= Music / Neutr_3_noDrums
	Right Shoulder	= Voice1
	Left Shoulder	= Voice2 
	Pause			= Quit App
*/

#include <SystemErrors.h>
#include <StringTypes.h>
#include <AudioMPI.h>
#include <ButtonMPI.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EventMessage.h>

#include <BrioOpenGLConfig.h>
#include <EmulationConfig.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif


//----------------------------------------------------------------------------
inline CPath GetAppRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir;
#else	// EMULATION
	return "/LF/Bulk/ProgramFiles/BrioAudio/rsrc/";
#endif	// EMULATION
}

//============================================================================
// Audio and Button Listener
//============================================================================
const tEventType kMyControllerTypes[] = { kAllButtonEvents, kAllAudioEvents };
const U32		 kDelayTime = 5000;

//----------------------------------------------------------------------------
class TransitionController : public IEventListener
{
public:
	TransitionController()
		: IEventListener(kMyControllerTypes, ArrayCount(kMyControllerTypes)),
		dbgMPI_(kMyApp)
	{
		isDone_ = false;
		
		isAudioRunning_ = true;
		isPlaying_Sine500Hz_ = false;
		isPlaying_Sine1K_ = false;
		isPlaying_Sine2K_ = false;
		isPlaying_Sine5K_ = false;
		isPlaying_WhiteNoise6db_ = false;
		isPlaying_Music_=  false;
		isPlaying_Voice1_ = false;		
		isPlaying_Voice2_ = false;
		isPlaying_Sfx_ = false;
		isPlaying_Noise05_16k_ = false;

		startTime_ = kernel_.GetElapsedTimeAsMSecs();
		debugLevel_ = dbgMPI_.GetDebugLevel();
		dbgMPI_.SetDebugLevel( kDbgLvlVerbose );
		audioMPI_.SetAudioResourcePath(GetAppRsrcFolder());
	}
	
	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{

		if( msgIn.GetEventType() == kAudioCompletedEvent )
		{	
			const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
			const tAudioMsgAudioCompleted& data = msg.audioMsgData.audioCompleted;
			dbgMPI_.DebugOut(kDbgLvlVerbose, "Audio done: id=%d, payload=%d, count=%d\n", 
					static_cast<int>(data.audioID), static_cast<int>(data.payload), static_cast<int>(data.count));
	
			// Change the flag for completing audio resources.
			if (data.audioID == idSine500Hz_) {
				isPlaying_Sine500Hz_ = false;
			} else if (data.audioID == idSine1K_) {
				isPlaying_Sine1K_ = false;
			} else if (data.audioID == idSine2K_) {
				isPlaying_Sine2K_ = false;
			} else if (data.audioID == idSine5K_) {
				isPlaying_Sine5K_ = false;
			} else if (data.audioID == idWhiteNoise6db_) {
				isPlaying_WhiteNoise6db_ = false;
			} else if (data.audioID == idMusic_) {
				isPlaying_Music_ = false;
			} else if (data.audioID == idSfx_) {
				isPlaying_Sfx_ = false;
			} else if (data.audioID == idVoice1_) {
				isPlaying_Voice1_ = false;
			} else if (data.audioID == idVoice2_) {
				isPlaying_Voice2_ = false;

			} else if (data.audioID == idNoise05_16k_) {
				isPlaying_Noise05_16k_ = false;

			} else {
				dbgMPI_.DebugOut(kDbgLvlCritical, "BrioAudio -- Mystery audioID completed, going unhandled!\n");
			}
		} 
		else if( msgIn.GetEventType() == kButtonStateChanged )
		{	
			U8		volume   = 100;
			S8		pan      = 0;
			tAudioPriority	priority = 1;
			tAudioOptionsFlags	flags = kAudioOptionsDoneMsgAfterComplete; // | kAudioOptionsLooped;

			const CButtonMessage& msg = dynamic_cast<const CButtonMessage&>(msgIn);
			tButtonData data = msg.GetButtonState();
			char ch = (data.buttonState & data.buttonTransition) ? '+' : '-';
			switch( data.buttonTransition )
			{
				// "Page Up" button
				case kButtonPause:
					if (ch == '+')
						isDone_ = true;
				break;
				
				// Button Up
				case kButtonUp:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button Up\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_Sine500Hz_) {
							idSine500Hz_ = audioMPI_.StartAudio( "TestTone_500Hz.ogg",  volume, priority, pan,  this, 0, flags );
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Key Up, StartAudio Sine500Hz, audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(idSine500Hz_) );
							isPlaying_Sine500Hz_ = true;
						} else {
							audioMPI_.StopAudio( idSine500Hz_, false );
							isPlaying_Sine500Hz_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio Sine500Hz, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idSine500Hz_) );
						}
					}	
				break;

				// "Down Arrow" button
				case kButtonDown:
				{
				U8		volume   = 100;
				S8		pan      = 0;
				tAudioPriority	priority = 1;
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button Down\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_Sine1K_) {
							idSine1K_ = audioMPI_.StartAudio( "TestTone_1000Hz.ogg", volume, priority, pan, this, 0, flags );
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Key Down, StartAudio Sine1K, audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(idSine1K_) );
							isPlaying_Sine1K_ = true;
						} else {
							audioMPI_.StopAudio( idSine1K_, false );
							isPlaying_Sine1K_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio Sine1K, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idSine1K_) );
						}
					}	
				}
				break;

				// "Left Arrow" button
				case kButtonLeft:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button Left\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_Sine2K_) {
							idSine2K_ = audioMPI_.StartAudio( "TestTone_2000Hz.ogg",  volume, priority, pan,  this, 0, flags );
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Key Down, StartAudio Sine2K, audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(idSine2K_) );
							isPlaying_Sine2K_ = true;
						} else {
							audioMPI_.StopAudio( idSine2K_, false );
							isPlaying_Sine2K_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio Sine2K, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idSine2K_) );
						}
					}	
				break;

				// "Right Arrow" button
				case kButtonRight:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button Right\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_Sine5K_) {
							idSine5K_ = audioMPI_.StartAudio( "TestTone_5000Hz.ogg", volume, priority, pan, this, 0, flags );
							isPlaying_Sine5K_ = true;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio Sine5K, audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(idSine5K_) );
						} else {
							audioMPI_.StopAudio( idSine5K_, false );
							isPlaying_Sine5K_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio Sine5K, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idSine5K_) );
						}
					}	
				break;
				
				// 'a' button
				case kButtonA:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button A\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_WhiteNoise6db_) {
							idWhiteNoise6db_ = audioMPI_.StartAudio( "WhiteNoise_60s_6dB_44k.ogg", volume, priority, pan,  this, 0, flags );
							isPlaying_WhiteNoise6db_ = true;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio WhiteNoise6db, audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(idWhiteNoise6db_) );
						} else {
							audioMPI_.StopAudio( idWhiteNoise6db_, false );
							isPlaying_WhiteNoise6db_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio WhiteNoise6db, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idWhiteNoise6db_) );
						}
					}	
				break;
				
				// 'b' button
				case kButtonB:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button B\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_Sfx_) {
							idSfx_ = audioMPI_.StartAudio( "LightningTest_SFXA_preproc_44.ogg", volume, priority, pan,  this, 0, flags );
							isPlaying_Sfx_ = true;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio SFX, audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(idSfx_) );
						} else {
							audioMPI_.StopAudio( idSfx_, false );
							isPlaying_Sfx_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio SFX, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idSfx_) );
						}
					}
				break;
					
				// "Home" button
				case kButtonHint:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c ButtonHint\n", ch);
						if( ch == '+' ) {
							if (!isPlaying_Music_) {
								idMusic_ = audioMPI_.StartAudio( "LightningTest_MusicA_preproc_44.ogg", volume, priority, pan,  this, 0, flags );
								isPlaying_Music_ = true;
								dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio Music, audioID = 0x%x\n", 
													ch, static_cast<unsigned int>(idMusic_) );
							} else {
								audioMPI_.StopAudio( idMusic_, false );
								isPlaying_Music_ = false;
								dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio Music, audioID = 0x%x\n", ch, 
										static_cast<unsigned int>(idMusic_) );
	
							}
						}	
				break;
					
				// "Delete" button
				case kButtonLeftShoulder:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button Left Shoulder\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_Voice1_) {
							idVoice1_ = audioMPI_.StartAudio( "LightningTest_VoiceA_preproc_44.ogg", volume, priority, pan, this, 0, flags );
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio Voice1, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idVoice1_) );
							isPlaying_Voice1_ = true;
						} else {
							audioMPI_.StopAudio( idVoice1_, false );
							isPlaying_Voice1_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio Voice1, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idVoice1_) );
						}
					}	
				break;
					
				// "Page Down" button
				case kButtonRightShoulder:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%c Button Right Shoulder\n", ch);
					if( ch == '+' ) {
						if (!isPlaying_Voice2_) {
							idVoice2_ = audioMPI_.StartAudio( "LightningTest_Voice_preproc_44.ogg", volume, priority, pan, this, 0, flags );
							isPlaying_Voice2_ = true;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio Voice2 audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(idVoice2_) );
						} else {
							audioMPI_.StopAudio( idVoice2_, false );
							isPlaying_Voice2_ = false;
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StopAudio Voice2, audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(idVoice2_) );
						}
					}	
				break;
			}
		}

		return kEventStatusOKConsumed;
	}
	
	bool IsDone()
	{
		return isDone_;
	}

	bool ShouldTransition()
	{
		U32 now = kernel_.GetElapsedTimeAsMSecs();
		if( (now - startTime_) > kDelayTime )
		{
			startTime_ = now;
			return true;
		}
		return false;
	}

	void ForceTransition()
	{
		startTime_ = 0;		//reset to force transition
	}
	
private:
	Boolean			isDone_;
	
	CDebugMPI		dbgMPI_;
	CAudioMPI		audioMPI_;

	tAudioID		idSine500Hz_;
	tAudioID		idSine1K_;
	tAudioID		idSine2K_;
	tAudioID		idSine5K_;
	tAudioID		idWhiteNoise6db_;
	tAudioID		idMusic_;
	tAudioID		idSfx_;
	tAudioID		idVoice1_;
	tAudioID		idVoice2_;
	tAudioID		idNoise05_16k_;

	Boolean		isAudioRunning_;

	Boolean		isPlaying_Sine500Hz_;
	Boolean		isPlaying_Sine1K_;
	Boolean		isPlaying_Sine2K_;
	Boolean		isPlaying_Sine5K_;
	Boolean		isPlaying_WhiteNoise6db_;
	Boolean		isPlaying_Music_;
	Boolean		isPlaying_Sfx_;
	Boolean		isPlaying_Voice1_;
	Boolean		isPlaying_Voice2_;
	Boolean		isPlaying_Noise05_16k_;


	mutable CKernelMPI	kernel_;	// FIXME/tp: Fix const correctness of KernelMPI

	U32			startTime_;
	tDebugLevel	debugLevel_;
};


//============================================================================
// main()
//============================================================================
int main() 
{
#ifdef EMULATION	
	// Initialize OpenGL state (lets us get button msgs in Emulation)
	BrioOpenGLConfig	config;
#endif
	
	// Setup button handler
	// NOTE: Order of events here is important on embedded target
	bool 					done = false;
	CDebugMPI				dbg(kMyApp);
	CDisplayMPI				dispmgr;	// FIXME: workaround for BRIGHT button dependency
	CButtonMPI				button;
	CKernelMPI				kernel;
	TransitionController	controller;

	if (button.RegisterEventListener(&controller) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return -1;
	}
	
	//Message Loop
	while(!done)
	{
		// Yield to other task threads, like audio
		kernel.TaskSleep( 100 ); 
	
		done = controller.IsDone();
	}
	
	button.UnregisterEventListener(&controller);
	return 0;

}


