// TestAudio.h 

#include <cxxtest/TestSuite.h>
#include <boost/shared_array.hpp>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <AudioMPI.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EventListener.h>
#include <EventMessage.h>
#include <EventMPI.h> 
#include <UnitTestUtils.h> 
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
 


//  For lots of text output, enable this:
#define	LF_BRIO_VERBOSE_TEST_OUTPUT

#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT
#define PRINT_TEST_NAME() printf("Running %s\n", __FUNCTION__)
#else
#define PRINT_TEST_NAME() {}
#endif

LF_USING_BRIO_NAMESPACE()

#define kPan        0
#define kVolume   100
#define kPriority   1

#define kPayload 0
#define kFlags   0 // kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped

//----------------------------------------------------------------------------
inline CPath GetAudioRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir + "Audio/";
#else	// EMULATION
	return "/Didj/Base/Brio/rsrc/Audio/";
#endif	// EMULATION
}

//============================================================================
// Audio Listener
//============================================================================
const tEventType kMyAudioTypes[] = { kAllAudioEvents };

//============================================================================
// TestAudioMgr functions
//============================================================================
class TestAudio : public CxxTest::TestSuite, TestSuiteBase, private IEventListener
{
private:
	CAudioMPI*			pAudioMPI_;
	CKernelMPI*			pKernelMPI_;
	Boolean				gotAudioCallback;
	Boolean				gotMidiCallback;
	U32					numLoopEndEvents;

public:
	TestAudio():IEventListener(kMyAudioTypes, ArrayCount(kMyAudioTypes))
	{
	}

	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{
		tEventStatus status = kEventStatusOK;
		CAudioMPI	audioMPI;

		const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
		if( msg.GetEventType() == kAudioCompletedEvent )
		{
			status = kEventStatusOKConsumed;
			gotAudioCallback = true;
		}
		else if( msg.GetEventType() == kMidiCompletedEvent )
		{
			gotMidiCallback = true;
			status = kEventStatusOKConsumed;
		}
		else if( msg.GetEventType() == kAudioLoopEndEvent )
		{
			numLoopEndEvents++;
			status = kEventStatusOKConsumed;
		}
		return status;
	}

	//------------------------------------------------------------------------
	void setUp( )
	{
		pAudioMPI_ = new CAudioMPI();
		pKernelMPI_ = new CKernelMPI();
		pAudioMPI_->SetAudioResourcePath( GetAudioRsrcFolder() );
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pKernelMPI_; 
		delete pAudioMPI_; 
	}
	

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		PRINT_TEST_NAME();
		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		PRINT_TEST_NAME();

		if (pAudioMPI_->IsValid()) {
			pName = pAudioMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "AudioMPI" );
			version = pAudioMPI_->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = pAudioMPI_->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Audio" );
			pURI = pAudioMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/Somewhere/AudioModule" );
		}
	}

    void testNoSuchFile()
	{
		tAudioID 				id;
		PRINT_TEST_NAME();
				
		id = pAudioMPI_->StartAudio("nosuchfile.ogg", kVolume, kPriority, kPan,
									NULL, kPayload, kFlags);
		
		TS_ASSERT(id == kNoAudioID);
	}

    void testUnsupportedExtension()
	{
		tAudioID 				id;
		PRINT_TEST_NAME();
				
		id = pAudioMPI_->StartAudio("dummy.foo", kVolume, kPriority, kPan,
									NULL, kPayload, kFlags);
		
		TS_ASSERT(id == kNoAudioID);
	}

    void testIsPlaying()
	{
		tAudioID 				id;
		PRINT_TEST_NAME();
		
		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									NULL, kPayload, kFlags);

		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(300);
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(id) == true);
		pKernelMPI_->TaskSleep(800);
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(id) == false);
	}
	
	//------------------------------------------------------------------------
    void testAudioStop()
	{
		tAudioID id;

		PRINT_TEST_NAME();

		gotAudioCallback = false;
		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									NULL, 0, 0);

		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopAudio(id, true);
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(id) == false);
		TS_ASSERT(gotAudioCallback == false);
	}
	
    void testTooManyVorbisFiles()
	{
		tAudioID id1, id2, id3, id4;

		PRINT_TEST_NAME();
		
		id1 = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									 NULL, kPayload, kFlags);
		TS_ASSERT(id1 != kNoAudioID);
		id2 = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									 NULL, kPayload, kFlags);
		TS_ASSERT(id2 != kNoAudioID);
		id3 = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									 NULL, kPayload, kFlags);
		TS_ASSERT(id3 != kNoAudioID);
		id4 = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									 NULL, kPayload, kFlags);
		TS_ASSERT(id4 == kNoAudioID);
		pAudioMPI_->StopAudio(id1, true);
		pAudioMPI_->StopAudio(id2, true);
		pAudioMPI_->StopAudio(id3, true);
		pAudioMPI_->StopAudio(id4, true);

	}

    void testVorbisStartStop()
	{
		tAudioID id;
		int i;

		PRINT_TEST_NAME();
		
		for( i=0; i<10; i++)
		{
			id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
										NULL, kPayload, kFlags);
			TS_ASSERT(id != kNoAudioID);
			pKernelMPI_->TaskSleep(100);		
			pAudioMPI_->StopAudio(id, true);
		}

	}

    void testAudioStopWithCallback()
	{
		// NOTE! This functionality is BROKEN and should remain so just in case
		// people are depending on the broken behavior.  Sigh.

		tAudioID id;
		PRINT_TEST_NAME();

		gotAudioCallback = false;
		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									NULL, 0, kAudioOptionsDoneMsgAfterComplete);

		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopAudio(id, false); //false means we SHOULD see done msg
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(id) == false);
		TS_ASSERT(gotAudioCallback == false); //...but we don't
	}

    void testPauseResumeAudioSystem()
	{
		tAudioID id;
		PRINT_TEST_NAME();

		id = pAudioMPI_->StartAudio("two-second.ogg", kVolume, kPriority, kPan,
									NULL, 0, 0);

		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(1000);
		pAudioMPI_->PauseAudioSystem();
		//NOTE: IsAudioPlaying should return false, but it returns true.  This
		//is broken functionality.  It will remain broken unless I'm instructed
		//to fix it on the grounds that developers may be depending on incorrect
		//functionality.
		TS_ASSERT(pAudioMPI_->IsAudioPlaying() == true)
		pKernelMPI_->TaskSleep(500);
		pAudioMPI_->ResumeAudioSystem();
		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100);
	}

    void testPauseResumeAudio()
	{
		tAudioID id;
		PRINT_TEST_NAME();

		id = pAudioMPI_->StartAudio("two-second.ogg", kVolume, kPriority, kPan,
									NULL, 0, 0);

		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(1000);
		pAudioMPI_->PauseAudio(id);
		//NOTE: IsAudioPlaying should return false, but it returns true.  This
		//is broken functionality.  It will remain broken unless I'm instructed
		//to fix it on the grounds that developers may be depending on incorrect
		//functionality.
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(id) == true)
		pKernelMPI_->TaskSleep(500);
		pAudioMPI_->ResumeAudio(id);
		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100);
	}

    void testGetAudioTime()
	{
		tAudioID id;
		PRINT_TEST_NAME();
		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									NULL, 0, 0);
		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(300);
		TS_ASSERT(pAudioMPI_->GetAudioTime(id) >= 0);
		pKernelMPI_->TaskSleep(300);
		TS_ASSERT(pAudioMPI_->GetAudioTime(id) >= 300);
		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100);
	}

	//------------------------------------------------------------------------
	void testVorbisSimple( )
	{
		tAudioID 				id;
		PRINT_TEST_NAME();

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		id = pAudioMPI_->StartAudio("Vivaldi.ogg", kVolume, kPriority, kPan,
									kNull, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100); 
	}
	
	//------------------------------------------------------------------------
	void testMIDISimpleUseCase1( )
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		U8 origVolume, volume = 0;
		PRINT_TEST_NAME();

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "2Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		//Wait for audio to terminate
		while(pAudioMPI_->IsMidiFilePlaying(midiPlayerID)) {
			pKernelMPI_->TaskSleep(100);
		}
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
	}

	void testMidiSimpleUseCase2()
	{
		tAudioID 				id;
		PRINT_TEST_NAME();

		id = pAudioMPI_->StartAudio("1Sec.mid", kVolume, kPriority, kPan,
									NULL, kPayload, kFlags);

		TS_ASSERT(id != kNoAudioID);
		//Wait for audio to terminate
		while(pAudioMPI_->IsAudioPlaying(id)) {
			pKernelMPI_->TaskSleep(100);
		}
	}
	
	//------------------------------------------------------------------------
    void testMIDIStopUseCase1()
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		PRINT_TEST_NAME();

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopMidiFile(midiPlayerID, true);
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == false);
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
	}


	void testTooManyMIDIUseCase1( )
	{
		tErrType 		err;
		tMidiPlayerID	id1, id2;
		U8 origVolume, volume = 0;
		PRINT_TEST_NAME();
		
		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &id1 );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( id1 != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( id1, "2Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &id2 );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( id2 == kNoMidiID );

		err = pAudioMPI_->StartMidiFile( id2, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err != kNoErr);

		pAudioMPI_->StopMidiFile(id1, true);
		pAudioMPI_->StopMidiFile(id2, true);

		err = pAudioMPI_->ReleaseMidiPlayer( id1 );
		TS_ASSERT(err == kNoErr);
	}
	
	void testMIDITwiceUseCase1( )
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		PRINT_TEST_NAME();

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopMidiFile(midiPlayerID, true);
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == false);
		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopMidiFile(midiPlayerID, true);

		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
	}

	void testMIDIAcquireFreeUseCase1( )
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		PRINT_TEST_NAME();

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopMidiFile(midiPlayerID, true);
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == false);
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopMidiFile(midiPlayerID, true);
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == false);
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );

	}

    void testMIDIStopWithCallbackUseCase1()
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		PRINT_TEST_NAME();

		gotMidiCallback = false;

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, kAudioCompletedEvent);
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopMidiFile(midiPlayerID, false);
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == false);

		//NOTE: This is incorrect functionality.  StopMidiFile should call the
		//midi callback.  We're not going to fix this unless instructed to do so
		//on the grounds that developers may be depending on this incorrect
		//functionality.
		TS_ASSERT(gotMidiCallback == false);
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
	}

    void testPauseResumeMidiUseCase1()
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		PRINT_TEST_NAME();

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->PauseMidiFile(midiPlayerID);
		//NOTE: This is incorrect functionality.  After calling PauseMidiFile,
		//IsMidiFilePlaying should return false.  We are not going to fix this
		//unless we're instructed to do so, however, because devleopers may be
		//depending on this broken functionality.
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == true);
		pAudioMPI_->ResumeMidiFile(midiPlayerID);
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == true);
		while(pAudioMPI_->IsMidiFilePlaying(midiPlayerID)) {
			pKernelMPI_->TaskSleep(100);
		}
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
	}

	void testAudioVolume()
	{
		tAudioID id;
		U8 volume = 0;
		PRINT_TEST_NAME();

		id = pAudioMPI_->StartAudio("two-second.ogg", kVolume, kPriority, kPan,
									kNull, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		while(pAudioMPI_->IsAudioPlaying(id)) {
			pAudioMPI_->SetMasterVolume(volume);
			volume += 1;
			pKernelMPI_->TaskSleep(20);
		}
	}

	void testAudioPan()
	{
		tAudioID id;
		S8 pan = -100;
		PRINT_TEST_NAME();

		id = pAudioMPI_->StartAudio("two-second.ogg", kVolume, kPriority, pan,
									kNull, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		while(pAudioMPI_->IsAudioPlaying(id)) {
			pan += 1;
			pAudioMPI_->SetAudioPan(id, pan);
			pKernelMPI_->TaskSleep(10);
		}
	}

    void testMidiVolumeUseCase1()
    {
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		U8 volume = 0;

		PRINT_TEST_NAME();

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "2Sec.mid", volume, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);
		
		//the only midi file we have right now is so long.
		while(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) && volume < 100) {
			volume += 1;
			pAudioMPI_->SetAudioVolume(midiPlayerID, volume);
			pKernelMPI_->TaskSleep(10);
		}
		//Wait for audio to terminate
		while(pAudioMPI_->IsMidiFilePlaying(midiPlayerID)) {
			pKernelMPI_->TaskSleep(100);
		}
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
    }

	//------------------------------------------------------------------------
	void testAudioCallback( )
	{
		tAudioID 	id;
		PRINT_TEST_NAME();

		gotAudioCallback = false;

		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority,
									kPan, this, kPayload, kAudioOptionsDoneMsgAfterComplete);
		
		//Wait for audio to terminate
		while(pAudioMPI_->IsAudioPlaying()) {
			pKernelMPI_->TaskSleep(100);
		}
		TS_ASSERT(gotAudioCallback == true);
	}

	//------------------------------------------------------------------------
	void testNoAudioCallback( )
	{
		tAudioID 	id;
		PRINT_TEST_NAME();

		gotAudioCallback = false;

		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority,
									kPan, this, kPayload, 0);
		
		//Wait for audio to terminate
		while(pAudioMPI_->IsAudioPlaying()) {
			pKernelMPI_->TaskSleep(100);
		}
		TS_ASSERT(gotAudioCallback == false);
	}

	//------------------------------------------------------------------------
	void testMidiCallbackUseCase1( )
	{
		
		tErrType err;
		tMidiPlayerID	midiPlayerID;
		PRINT_TEST_NAME();

		gotMidiCallback = false;

		err = pAudioMPI_->AcquireMidiPlayer(1, NULL, &midiPlayerID);		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile(midiPlayerID, "2Sec.mid", kVolume, 1,
										this, 0, kAudioOptionsDoneMsgAfterComplete);
		TS_ASSERT(err == kNoErr);
		
		while(pAudioMPI_->IsMidiFilePlaying(midiPlayerID))
			pKernelMPI_->TaskSleep(10);

		TS_ASSERT(gotMidiCallback == true);
		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
	}

	//------------------------------------------------------------------------
	void testVorbisLooping( )
	{
		tAudioID 	id;
		PRINT_TEST_NAME();
		
		numLoopEndEvents = 0;
		gotAudioCallback = false;

		id = pAudioMPI_->StartAudio("LoopContinuity.ogg", kVolume, kPriority,
									kPan, this, 3,
									kAudioOptionsLooped |
									kAudioOptionsDoneMsgAfterComplete);

		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100);
		TS_ASSERT(gotAudioCallback == true);
		TS_ASSERT(numLoopEndEvents == 0);
	}

	//------------------------------------------------------------------------
	void testVorbisLoopingCallbacks( )
	{
		tAudioID 	id;
		PRINT_TEST_NAME();

		numLoopEndEvents = 0;
		gotAudioCallback = false;
		id = pAudioMPI_->StartAudio("LoopContinuity.ogg", kVolume, kPriority,
									kPan, this, 3,
									kAudioOptionsLooped |
									kAudioOptionsLoopEndMsg |
									kAudioOptionsDoneMsgAfterComplete);

		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100);
		TS_ASSERT(numLoopEndEvents == 3);
		TS_ASSERT(gotAudioCallback == true);
	}

	//------------------------------------------------------------------------
	void testMidiLoopingCallbacksUseCase1( )
	{
		tErrType err;
		tMidiPlayerID	midiPlayerID;
		PRINT_TEST_NAME();

		gotMidiCallback = false;
		numLoopEndEvents = 0;

		err = pAudioMPI_->AcquireMidiPlayer(1, NULL, &midiPlayerID);
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile(midiPlayerID, "2Sec.mid", kVolume, 1,
										this, 3,
										kAudioOptionsLooped |
										kAudioOptionsLoopEndMsg |
										kAudioOptionsDoneMsgAfterComplete);
		TS_ASSERT(err == kNoErr);
		
		while(pAudioMPI_->IsMidiFilePlaying(midiPlayerID))
			pKernelMPI_->TaskSleep(10);

		TS_ASSERT(numLoopEndEvents == 3);
		TS_ASSERT(gotMidiCallback == true);

		err = pAudioMPI_->ReleaseMidiPlayer( midiPlayerID );
		TS_ASSERT(err == kNoErr);
	}

	void testThree16kStreams()
	{
		tAudioID id1, id2, id3;
		PRINT_TEST_NAME();

		id1 = pAudioMPI_->StartAudio("Voice-3sec.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id1 != kNoAudioID);

		id2 = pAudioMPI_->StartAudio("two-second.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id2 != kNoAudioID);

		id3 = pAudioMPI_->StartAudio("VH_16_st-3sec.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id3 != kNoAudioID);

		while(pAudioMPI_->IsAudioPlaying())
			pKernelMPI_->TaskSleep(100);
	}

	void testThree32kStreams()
	{
		tAudioID id1, id2, id3;
		PRINT_TEST_NAME();

		id1 = pAudioMPI_->StartAudio("Vivaldi-3sec.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id1 != kNoAudioID);

		id2 = pAudioMPI_->StartAudio("Music-5sec.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id2 != kNoAudioID);

		id3 = pAudioMPI_->StartAudio("Music-3sec.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id3 != kNoAudioID);

		while(pAudioMPI_->IsAudioPlaying())
			pKernelMPI_->TaskSleep(100);
	}

    // This test can generally be disabled.  It is not used to test
    // functionality and prevent regression, but to test performance.
	void testPerformanceBaseline()
	{
		tAudioID id1, id2, id3;
		tMidiPlayerID id4;
        tErrType err;

		PRINT_TEST_NAME();

		err = pAudioMPI_->AcquireMidiPlayer(1, NULL, &id4);
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( id4 != kNoMidiID );

		id1 = pAudioMPI_->StartAudio("wool-16kHz-mono.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id1 != kNoAudioID);

		id2 = pAudioMPI_->StartAudio("watermelon-16kHz-st.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id2 != kNoAudioID);

		id3 = pAudioMPI_->StartAudio("passin-16kHz-mono.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id3 != kNoAudioID);

		err = pAudioMPI_->StartMidiFile(id4, "POWMusic.mid", kVolume, 1,
										kNull, 0, 0);
		TS_ASSERT(err == kNoErr);
		
        while(pAudioMPI_->IsAudioPlaying() || pAudioMPI_->IsMidiFilePlaying(id4))
			pKernelMPI_->TaskSleep(100);
		
		err = pAudioMPI_->ReleaseMidiPlayer( id4 );
		TS_ASSERT(err == kNoErr);

	}


};
