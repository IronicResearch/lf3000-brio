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
//#define	LF_BRIO_VERBOSE_TEST_OUTPUT

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

// ============================================================================
// BoundS32  :   Add increment to value and bound to range
// 
//    Warning :   Will fail when values reach limits of signed 32-bit
// ============================================================================
	long
BoundS32(long d, long lower, long upper)
{	     
if      (d < lower)
    return (lower);
else if (d > upper)
    return (upper);

return (d);
}	// ---- end BoundS32() ----

// ============================================================================
// IncrementAndBoundS32  :   Add increment to value and bound to range
//                              Warning :   will fail when values reach limits of
//                          Signed 32-bit
// ============================================================================
	long
IncrementAndBoundS32(long d, long inc, long lower, long upper)
{	     
return (BoundS32(d+inc, lower, upper));
}	// ---- end IncrementAndBoundS32() ----

//============================================================================
// Audio Listener
//============================================================================
const tEventType kMyAudioTypes[] = { kAllAudioEvents };

//----------------------------------------------------------------------------
// global audio IDs, one for each thread.  This makes sure that the thread doesn't
// exit until the audio done event has been received by the listener.
// TODO: kind of a hack.
tAudioID 				id1, id2, id3, id4;

// Task to test multithreading and multi-MPIs.
typedef struct
{
	int threadNum;
	int loopCount;
}thread_arg_t;

static void *myTask(void* arg)
{
	CAudioMPI				audioMPI;
	CKernelMPI				kernelMPI;

	int						index;
	CPath*					filePath = kNull;
	tAudioID 				id = 0;
	U32						time;
	U8						volume;
	S32						pan;
	tAudioPriority 			priority;

	thread_arg_t 			*ptr = (thread_arg_t *)arg; 
	int						threadNum = ptr->threadNum;
	int						loopCount = ptr->loopCount;

	printf("==== myTask( thread %d ) -- test task thread starting up.\n", threadNum );

	// Set the default location to search for resources.
	audioMPI.SetAudioResourcePath( GetAudioRsrcFolder() );
	
	// Each thread needs to play a different file so we can tell the difference.
	switch( threadNum ) {
		case 1:
			filePath = new CPath( "Vivaldi.ogg" );
			printf("myTask( thread %d ) loading Vivaldi, path = %s\n", threadNum, filePath->c_str() );	
			id = audioMPI.StartAudio( *filePath, kVolume, kPriority, kPan, kNull, kPayload, kFlags );
			id1 = id;
			delete filePath;
			break;

		case 2:
			filePath = new CPath( "VH_16_mono.ogg" );
			printf("myTask( thread %d ) found VH_16_mono, path = %s\n", threadNum, filePath->c_str() );	
			id = audioMPI.StartAudio( *filePath, kVolume, kPriority, kPan, kNull, kPayload, kFlags );
			id2 = id;
			delete filePath;
		break;

		case 3:
			filePath = new CPath( "SFX.ogg" );
			printf("myTask( thread %d ) found SFX, path = %s\n", threadNum, filePath->c_str() );	
			id = audioMPI.StartAudio( *filePath, kVolume, kPriority, kPan, kNull, kPayload, kFlags );
			id3 = id;
			delete filePath;
		break;

		case 4:
			filePath = new CPath( "Voice.ogg" );
			printf("myTask( thread %d ) found Voice, path = %s\n", threadNum, filePath->c_str() );	
			id = audioMPI.StartAudio( *filePath, kVolume, kPriority, kPan, kNull, kPayload, kFlags );
			id4 = id;
			delete filePath;
		break;
	}
	
	printf("===== myTask( thread %d ) back from calling StartAudio(); id = %d\n", threadNum, (int)id );

	// sleep 2 seconds
	kernelMPI.TaskSleep( 2000 ); 

	// loop sleep 1 second
	for (index = 0; index < loopCount; index++) {
		time     = audioMPI.GetAudioTime(     id );
		volume   = audioMPI.GetAudioVolume(   id );
		priority = audioMPI.GetAudioPriority( id );
		pan      = audioMPI.GetAudioPan(      id );
		printf("==== myTask( thread%d loop#%2d) id=%2ld pri=%d time=%5lu vol=%3d pan=%ld\n", 
				threadNum, index, id, priority, time, volume, pan );
		
		kernelMPI.TaskSleep( 125 ); 

		audioMPI.SetAudioVolume( id, volume - (index*2) );
                
		audioMPI.SetAudioPan( id, (S8) BoundS32(pan - index*4, -100, 100));

		kernelMPI.TaskSleep( 125 ); 
	}

	printf("----------------- myTask( thread %d ) done looping over API tests.()\n", threadNum );

	// Stop audio instead of just bailing out of the thread 
	// so we can get done msg from player.
	audioMPI.StopAudio( id, false );
	printf("==== myTask( thread %d ) back from calling StopAudio()\n", threadNum );
	
	printf("+++++++++++ myTask( thread %d ) Exiting... \n" , threadNum );

	return (void *)NULL;
}	

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

    void testIsPlaying()
	{
		tAudioID 				id;
		
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
		gotAudioCallback = false;
		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority, kPan,
									NULL, 0, 0);

		TS_ASSERT(id != kNoAudioID);
		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopAudio(id, true);
		TS_ASSERT(pAudioMPI_->IsAudioPlaying(id) == false);
		TS_ASSERT(gotAudioCallback == false);
	}
	
    void testAudioStopWithCallback()
	{
		// NOTE! This functionality is BROKEN and should remain so just in case
		// people are depending on the broken behavior.  Sigh.

		tAudioID id;
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
	void testMIDISimple( )
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		U8 origVolume, volume = 0;

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
	}

	//------------------------------------------------------------------------
    void testMIDIStop()
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		TS_ASSERT( midiPlayerID != kNoMidiID );

		err = pAudioMPI_->StartMidiFile( midiPlayerID, "1Sec.mid", 100, 1,
										 kNull, 0, 0 );
		TS_ASSERT(err == kNoErr);

		pKernelMPI_->TaskSleep(300);
		pAudioMPI_->StopMidiFile(midiPlayerID, true);
		TS_ASSERT(pAudioMPI_->IsMidiFilePlaying(midiPlayerID) == false);
	}
	
    void testMIDIStopWithCallback()
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;

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
	}

    void testPauseResumeMidi()
	{
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;

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
	}

	void testAudioVolume()
	{
		tAudioID id;
		U8 volume = 0;
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
		id = pAudioMPI_->StartAudio("two-second.ogg", kVolume, kPriority, pan,
									kNull, kPayload, kFlags);
		TS_ASSERT(id != kNoAudioID);
		while(pAudioMPI_->IsAudioPlaying(id)) {
			pan += 1;
			pAudioMPI_->SetAudioPan(id, pan);
			pKernelMPI_->TaskSleep(10);
		}
	}

    void testMidiVolume()
    {
		tErrType 		err;
		tMidiPlayerID	midiPlayerID;
		U8 volume = 0;

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
    }

	//------------------------------------------------------------------------
	void testAudioCallback( )
	{
		tAudioID 	id;

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
	void testMidiCallback( )
	{
		
		tErrType err;
		tMidiPlayerID	midiPlayerID;

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
	}

	//------------------------------------------------------------------------
	void testVorbisLooping( )
	{
		tAudioID 	id;
		
		numLoopEndEvents = 0;
		gotAudioCallback = false;

		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority,
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

		numLoopEndEvents = 0;
		gotAudioCallback = false;
		id = pAudioMPI_->StartAudio("one-second.ogg", kVolume, kPriority,
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
	void testMidiLoopingCallbacks( )
	{
		tErrType err;
		tMidiPlayerID	midiPlayerID;

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
	}

	void testThree16kStreams()
	{
		tAudioID id1, id2, id3;

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

	void testPerformanceBaseline()
	{
		tAudioID id1, id2, id3;
		tMidiPlayerID id4;
        tErrType err;

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
	}


};
