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
#include <Utility.h>

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

// These limits should really be available to all Brio Apps
#define myAudioMaxRawStreams	16
#define myAudioMaxVorbisStreams	3

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
	U32					numMidiEvents;
	CEventMPI			evtmgr_;
	
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
		else if( msg.GetEventType() == kAudioMidiEvent )
		{
			numMidiEvents++;
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
		sleep(1);
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

	//------------------------------------------------------------------------
    void testNoSuchFile()
	{
		tAudioID 				id;
		PRINT_TEST_NAME();
				
		id = pAudioMPI_->StartAudio("nosuchfile.ogg", kVolume, kPriority, kPan,
									NULL, kPayload, kFlags);
		
		TS_ASSERT(id == kNoAudioID);
	}

	//------------------------------------------------------------------------
    void testUnsupportedExtension()
	{
		tAudioID 				id;
		PRINT_TEST_NAME();
				
		id = pAudioMPI_->StartAudio("dummy.foo", kVolume, kPriority, kPan,
									NULL, kPayload, kFlags);
		
		TS_ASSERT(id == kNoAudioID);
	}

	//------------------------------------------------------------------------
    void testAudioStart()
	{
		tAudioID 				id;
		PRINT_TEST_NAME();
		
		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		id = pAudioMPI_->StartAudio("two-second.ogg", kVolume, kPriority, kPan,
									NULL, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		pAudioMPI_->StopAudio(id, false);
	}
	
	//------------------------------------------------------------------------
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
		pKernelMPI_->TaskSleep(900);
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
	
	//------------------------------------------------------------------------
    void testTooManyVorbisFiles()
	{
		tAudioID id[myAudioMaxVorbisStreams + 1];
		int		 i;

		PRINT_TEST_NAME();
	
		for (i=0; i < myAudioMaxVorbisStreams; i++)	{
			id[i] = pAudioMPI_->StartAudio("one-second.ogg",
				kVolume, kPriority, kPan, NULL, kPayload, kFlags);
			TS_ASSERT(id[i] != kNoAudioID);
		}

		id[myAudioMaxVorbisStreams] = pAudioMPI_->StartAudio("one-second.ogg",
			kVolume, kPriority, kPan, NULL, kPayload, kFlags);
		TS_ASSERT(id[myAudioMaxVorbisStreams] == kNoAudioID);

		for (i=0; i <= myAudioMaxVorbisStreams; i++) {
			pAudioMPI_->StopAudio(id[i], true);
		}
	}

	//------------------------------------------------------------------------
    void testTooManyAdpcmFiles()
	{
		tAudioID id[myAudioMaxRawStreams + 1];
		int		 i;

		PRINT_TEST_NAME();
	
		for (i=0; i < myAudioMaxRawStreams; i++)	{
			id[i] = pAudioMPI_->StartAudio("two-second.wav",
				kVolume, kPriority, kPan, NULL, kPayload, kFlags);
			TS_ASSERT(id[i] != kNoAudioID);
		}

		id[myAudioMaxRawStreams] = pAudioMPI_->StartAudio("two-second.wav",
			kVolume, kPriority, kPan, NULL, kPayload, kFlags);
		TS_ASSERT(id[myAudioMaxRawStreams] == kNoAudioID);

		for (i=0; i <= myAudioMaxRawStreams; i++) {
			pAudioMPI_->StopAudio(id[i], true);
		}
	}

	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
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
		id = pAudioMPI_->StartAudio("Vivaldi-3sec.ogg", kVolume, kPriority, kPan,
									kNull, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100); 
	}
	
	//------------------------------------------------------------------------
	void testAdpcmSimple( )
	{
		tAudioID 				id;
		PRINT_TEST_NAME();

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		id = pAudioMPI_->StartAudio("Vivaldi-3sec.wav", kVolume, kPriority, kPan,
									kNull, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		while(pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100); 
	}
	
	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
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

	//------------------------------------------------------------------------
    // This test can generally be disabled.  It is not used to test
    // functionality and prevent regression, but to test performance.
	void testPerformanceBaseline()
	{
		tAudioID id1, id2, id3;
        tErrType err;

		PRINT_TEST_NAME();

		id1 = pAudioMPI_->StartAudio("wool-16kHz-mono.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id1 != kNoAudioID);

		id2 = pAudioMPI_->StartAudio("watermelon-16kHz-st.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id2 != kNoAudioID);

		id3 = pAudioMPI_->StartAudio("passin-16kHz-mono.ogg", kVolume, kPriority,
									 kPan, kNull, 0, 0);
		TS_ASSERT(id3 != kNoAudioID);

        while(pAudioMPI_->IsAudioPlaying())
			pKernelMPI_->TaskSleep(100);
		
	}

	//------------------------------------------------------------------------
	void testPriorityPolicyGetSet()
	{
        tErrType err;
		tPriorityPolicy policy;

		PRINT_TEST_NAME();
		policy = pAudioMPI_->GetPriorityPolicy();
		TS_ASSERT(policy == kAudioPriorityPolicyNone);
		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicyNone);
		TS_ASSERT(err == kNoErr);

		err = pAudioMPI_->SetPriorityPolicy(100);
		TS_ASSERT(err == kNoImplErr);
		policy = pAudioMPI_->GetPriorityPolicy();
		TS_ASSERT(policy == kAudioPriorityPolicyNone);
	}

	//------------------------------------------------------------------------
    void testSimplePriorityAllPriorityEqual()
	{
		tErrType err;
		tAudioID id;
		int i;

		PRINT_TEST_NAME();

		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicySimple);
		TS_ASSERT(err == kNoErr);
		
		for( i=0; i<10; i++)
		{
			id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
										NULL, kPayload, kFlags);
			TS_ASSERT(id != kNoAudioID);
			pKernelMPI_->TaskSleep(200);
		}
		while(pAudioMPI_->IsAudioPlaying() == true)
			pKernelMPI_->TaskSleep(200);
	}

	//------------------------------------------------------------------------
    void testSimpleVorbisPriorityIncreasing()
	{
		tErrType err;
		tAudioID id;
		int i;
		tAudioPriority priority = 0;

		PRINT_TEST_NAME();

		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicySimple);
		TS_ASSERT(err == kNoErr);
		
		for( i=0; i<10; i++)
		{
			id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, priority,
										kPan, NULL, kPayload, kFlags);
			TS_ASSERT(id != kNoAudioID);
			pKernelMPI_->TaskSleep(200);
			priority++;
		}
		while(pAudioMPI_->IsAudioPlaying() == true)
			pKernelMPI_->TaskSleep(200);
	}

	//------------------------------------------------------------------------
    void testSimpleVorbisPriorityDecreasing()
	{
		tErrType err;
		tAudioID id;
		int i;
		tAudioPriority priority = 255;

		PRINT_TEST_NAME();

		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicySimple);
		TS_ASSERT(err == kNoErr);

		// Launch 3 players
		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, priority--,
									kPan, NULL, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(100);

		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, priority--,
									kPan, NULL, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(100);

		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, priority--,
									kPan, NULL, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(100);

		// Try to launch N more players before the 1st 3 finish playing 
		for( i=0; i<6; i++)
		{
			id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, priority--,
										kPan, NULL, kPayload, kFlags);
			TS_ASSERT(id == kNoAudioID);
			pKernelMPI_->TaskSleep(100);
		}
		while(pAudioMPI_->IsAudioPlaying() == true)
			pKernelMPI_->TaskSleep(200);
	}

	//------------------------------------------------------------------------
    void testSimpleVorbisPriorityTypical()
	{
		tErrType err;
		tAudioID bg, incidental, dialog;
		int i;
	
		PRINT_TEST_NAME();

		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicySimple);
		TS_ASSERT(err == kNoErr);

		//Launch background music.  It should never be interrupted
		bg = pAudioMPI_->StartAudio("watermelon-16kHz-st.ogg", kVolume, 255,
									kPan, NULL, kPayload, kFlags);
		TS_ASSERT(bg != kNoAudioID);
		pKernelMPI_->TaskSleep(100);

		//Launch a bunch of incidentals.  They should pre-empt eachother, but
		//not the background music.
		for( i=0; i<7; i++)
		{
			incidental = pAudioMPI_->StartAudio("Vivaldi-3sec.ogg", kVolume, 0,
												kPan, NULL, kPayload, kFlags);
			TS_ASSERT(incidental != kNoAudioID);
			pKernelMPI_->TaskSleep(1000);
		}
		
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(bg) == true);

		//This would be like dialog.  It should pre-empt one of the incidentals
		dialog = pAudioMPI_->StartAudio("Voice-3sec.ogg", kVolume, 100,
										kPan, NULL, kPayload, kFlags);
		TS_ASSERT(dialog != kNoAudioID);

		TS_ASSERT(pAudioMPI_->IsAudioPlaying(bg) == true);

		// keep generating incidentals.  Only one at a time will be able to
		// play.
		while(pAudioMPI_->IsAudioPlaying(dialog))
		{
			incidental = pAudioMPI_->StartAudio("Vivaldi-3sec.ogg", kVolume, 0,
												kPan, NULL, kPayload, kFlags);
			TS_ASSERT(incidental != kNoAudioID);
			pKernelMPI_->TaskSleep(1000);
		}	
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(bg) == true);
		  
		while(pAudioMPI_->IsAudioPlaying(incidental))
			pKernelMPI_->TaskSleep(200);

		pAudioMPI_->StopAudio(bg, true);

	}
	
	//------------------------------------------------------------------------
    void testSimpleAdpcmPriorityIncreasing()
	{
		tErrType err;
		tAudioID id;
		int i;
		tAudioPriority priority = 0;

		PRINT_TEST_NAME();

		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicySimple);
		TS_ASSERT(err == kNoErr);
		
		for( i=0; i < (myAudioMaxRawStreams * 2); i++)
		{
			id = pAudioMPI_->StartAudio("one-second.wav", kVolume, priority,
										kPan, NULL, kPayload, kFlags);
			TS_ASSERT(id != kNoAudioID);
			pKernelMPI_->TaskSleep(200);
			priority++;
		}
		while(pAudioMPI_->IsAudioPlaying() == true)
			pKernelMPI_->TaskSleep(200);
	}

	//------------------------------------------------------------------------
    void testSimpleAdpcmPriorityDecreasing()
	{
		tErrType err;
		tAudioID id;
		int i;
		tAudioPriority priority = 255;

		PRINT_TEST_NAME();

		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicySimple);
		TS_ASSERT(err == kNoErr);

		// Launch max players
		for (i=0; i < myAudioMaxRawStreams; i++) {
			id = pAudioMPI_->StartAudio("two-second.wav", kVolume, priority--,
									kPan, NULL, kPayload, kFlags);
			TS_ASSERT(id != kNoAudioID);
			pKernelMPI_->TaskSleep(50);
		}

		// Try to launch more players before the 1st batch finish playing 
		for( i=0; i<myAudioMaxRawStreams; i++)
		{
			id = pAudioMPI_->StartAudio("one-second.wav", kVolume, priority--,
										kPan, NULL, kPayload, kFlags);
			TS_ASSERT(id == kNoAudioID);
			pKernelMPI_->TaskSleep(50);
		}
		while(pAudioMPI_->IsAudioPlaying() == true)
			pKernelMPI_->TaskSleep(100);
	}

	//------------------------------------------------------------------------
    void testSimpleAdpcmPriorityTypical()
	{
		tErrType err;
		tAudioID bg, incidental, dialog;
		int i;
	
		PRINT_TEST_NAME();

		err = pAudioMPI_->SetPriorityPolicy(kAudioPriorityPolicySimple);
		TS_ASSERT(err == kNoErr);

		// Launch background music on different player.
		// It should never be interrupted
		bg = pAudioMPI_->StartAudio("watermelon-16kHz-st.ogg", kVolume, 0,
									kPan, NULL, kPayload, kFlags);
		TS_ASSERT(bg != kNoAudioID);
		pKernelMPI_->TaskSleep(100);

		//Launch a bunch of incidentals.  They should pre-empt eachother, but
		//not the background music.
		for (i=0; i < myAudioMaxRawStreams + 5; i++)
		{
			incidental = pAudioMPI_->StartAudio("Vivaldi.wav", kVolume, 0,
												kPan, NULL, kPayload, kFlags);
			TS_ASSERT(incidental != kNoAudioID);
			pKernelMPI_->TaskSleep(500);
		}
		
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(bg) == true);

		//This would be like dialog.  It should pre-empt one of the incidentals
		dialog = pAudioMPI_->StartAudio("Voice-3sec.wav", kVolume, 100,
										kPan, NULL, kPayload, kFlags);
		TS_ASSERT(dialog != kNoAudioID);

		TS_ASSERT(pAudioMPI_->IsAudioPlaying(bg) == true);

		// keep generating incidentals.  Only one at a time will be able to
		// play.
		while(pAudioMPI_->IsAudioPlaying(dialog))
		{
			incidental = pAudioMPI_->StartAudio("Vivaldi.wav", kVolume, 0,
												kPan, NULL, kPayload, kFlags);
			TS_ASSERT(incidental != kNoAudioID);
			pKernelMPI_->TaskSleep(1000);
		}	
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(bg) == true);
		  
		while(pAudioMPI_->IsAudioPlaying(incidental))
			pKernelMPI_->TaskSleep(200);

		pAudioMPI_->StopAudio(bg, true);

	}

    //------------------------------------------------------------------------
    void testAudioMemPlayer()
	{
		tAudioID 				id;
		tAudioHeader			hdr;
		size_t					len;
		void*					praw;
		FILE*					fp;
		S16*					pmem;
		CPath					path = GetAudioRsrcFolder() + "LeftRightCenter.raw"; 
		
		PRINT_TEST_NAME();
		
		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pKernelMPI_ != NULL );

		len = FileSize(path.c_str());
		TS_ASSERT( len != 0 );
		praw = pKernelMPI_->Malloc(len);
		TS_ASSERT( praw != NULL );
		
		fp = fopen(path.c_str(), "r");
		TS_ASSERT( fp != NULL );
		fread(praw, 1, len, fp);
		fclose(fp);
	
		memcpy(&hdr, praw, sizeof(tAudioHeader));
		pmem = (S16*)((U8*)praw + hdr.offsetToData);
		
		id = pAudioMPI_->StartAudio(hdr, pmem, NULL, kVolume, kPriority, kPan);
		TS_ASSERT(id != kNoAudioID);
		while (pAudioMPI_->IsAudioPlaying(id))
			pKernelMPI_->TaskSleep(100);

		pAudioMPI_->StopAudio(id, false);
		pKernelMPI_->Free(praw);
	}

};
