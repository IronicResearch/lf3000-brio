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
			const tAudioMsgAudioCompleted& data = msg.audioMsgData.audioCompleted;
			// dbg_.DebugOut(kDbgLvlVerbose, 
			printf("****** Audio done: id=%d, payload=%d\n", 
					static_cast<int>(data.audioID), static_cast<int>(data.payload));
			status = kEventStatusOKConsumed;

//			TODO: test for starting audio during Notify() callback.	
//			audioMPI.StartAudio( handle1, kVolume, kPriority, kPan, kNull, kPayload, kFlags );
//			printf("TestAudio -- AudioListener:: Notify() back from calling StartAudio()\n" );
		} 
		else if( msg.GetEventType() == kMidiCompletedEvent )
		{
			const tAudioMsgMidiCompleted& data = msg.audioMsgData.midiCompleted;
			// dbg_.DebugOut(kDbgLvlVerbose, 
			printf("****** MIDI File done: player id=%d, payload=%d\n", 
					static_cast<int>(data.midiPlayerID), static_cast<int>(data.payload));
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
	void xxxtestVorbisLooping( )
	{
		tAudioID 	id1;
		U32			seconds = 0;

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		printf("TestAudio -- testVorbisLooping() starting, will run for 20 seconds, listen for looping... \n" );

		id1 = pAudioMPI_->StartAudio("Sine_500Hz.ogg", kVolume, kPriority,
									 kPan, this, kPayload, kAudioOptionsLooped);
		printf("TestAudio -- testVorbisResources() back from calling StartAudio()\n" );

		// loop sleep 1 second
		while ( seconds < 20 ) {
			pKernelMPI_->TaskSleep( 1000 ); 
		}
	}

	//------------------------------------------------------------------------
	void testThreading()
	{
		printf("testThreading starting... \n");

		const int 		kDuration = 1 * 3000;

		CKernelMPI		kernelMPI;

		tTaskHndl 		hndl_1;
		tTaskHndl 		hndl_2;
		tTaskHndl 		hndl_3;
		tTaskHndl 		hndl_4;
        thread_arg_t 	threadArg;
 	    tTaskProperties pProperties;
        tPtr 			status = NULL;

		threadArg.loopCount = 20;

		pProperties.TaskMainFcn = (void* (*)(void*))myTask;
		pProperties.pTaskMainArgValues = &threadArg;

		threadArg.threadNum = 1;
		
		TS_ASSERT_EQUALS( kNoErr, kernelMPI.CreateTask( hndl_1,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		// sleep3 seconds
		kernelMPI.TaskSleep( kDuration );

		threadArg.threadNum = 2;
		TS_ASSERT_EQUALS( kNoErr, kernelMPI.CreateTask( hndl_2,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		// sleep3 seconds
		kernelMPI.TaskSleep( kDuration );

		threadArg.threadNum = 3;
		TS_ASSERT_EQUALS( kNoErr,kernelMPI.CreateTask( hndl_3,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		// sleep3 seconds
		kernelMPI.TaskSleep( kDuration );

		threadArg.threadNum = 4;
		TS_ASSERT_EQUALS( kNoErr,kernelMPI.CreateTask( hndl_4,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		TS_ASSERT_EQUALS( kNoErr, kernelMPI.JoinTask( hndl_1, status ));
		TS_ASSERT_EQUALS( kNoErr, kernelMPI.JoinTask( hndl_2, status ));
		TS_ASSERT_EQUALS( kNoErr, kernelMPI.JoinTask( hndl_3, status ));
		TS_ASSERT_EQUALS( kNoErr, kernelMPI.JoinTask( hndl_4, status ));

		// this will exit when all threads exit.
	}
};



