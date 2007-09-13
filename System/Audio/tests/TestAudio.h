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

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//----------------------------------------------------------------------------
inline CPath GetAudioRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir + "Audio/";
#else	// EMULATION
	return "/Base/rsrc/Audio/";
#endif	// EMULATION
}

//============================================================================
// Audio Listener
//============================================================================
const tEventType kMyAudioTypes[] = { kAllAudioEvents };

//----------------------------------------------------------------------------
class AudioListener : public IEventListener
{
public:
	AudioListener( )
		: IEventListener(kMyAudioTypes, ArrayCount( kMyAudioTypes )), dbg_( kMyApp )
	{
//		dbg_.SetDebugLevel( kDbgLvlVerbose );
		dbg_.DebugOut( kDbgLvlVerbose, "Audio Listener created...\n" );
	}

	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{
		tEventStatus status = kEventStatusOK;
		CAudioMPI	audioMPI;

		const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
		if( msg.GetEventType() == kAudioCompletedEvent )
		{
			const tAudioMsgAudioCompleted& data = msg.audioMsgData.audioCompleted;
			dbg_.DebugOut(kDbgLvlVerbose, "Audio done: id=%d, payload=%d\n", 
					static_cast<int>(data.audioID), static_cast<int>(data.payload));
			status = kEventStatusOKConsumed;

//			TODO: test for starting audio during Notify() callback.	
//			audioMPI.StartAudio( handle1, 100, 1, 0, kNull, 0, 0 );
//			printf("TestAudio -- AudioListener:: Notify() back from calling StartAudio()\n" );
} 
		else if( msg.GetEventType() == kMidiCompletedEvent )
		{
			const tAudioMsgMidiCompleted& data = msg.audioMsgData.midiCompleted;
			dbg_.DebugOut(kDbgLvlVerbose, "MIDI File done: player id=%d, payload=%d\n", 
					static_cast<int>(data.midiPlayerID), static_cast<int>(data.payload));
			status = kEventStatusOKConsumed;
		}

		return status;
	}
private:
	CDebugMPI	dbg_;
};

// Task to test multithreading and multi-MPIs.
typedef struct
{
	int threadNum;
}thread_arg_t;

static void *myTask(void* arg)
{
	CAudioMPI				audioMPI;
	CKernelMPI				kernelMPI;

	U32						index;
	CPath*					filePath;
	tAudioID 				id;
	U32						time;
	U8						volume;
	S8						pan;
	tAudioPriority 			priority;
	AudioListener			audioListener;
	const IEventListener* 	pListener;

	thread_arg_t 			*ptr = (thread_arg_t *)arg; 
	int						threadNum = ptr->threadNum;

	printf("myTask( thread %d ) -- test task thread starting up.\n", threadNum );

	// Set the default location to search for resources.
	audioMPI.SetAudioResourcePath( GetAudioRsrcFolder() );
	
	// Each thread needs to play a different file so we can tell the difference.
	switch( threadNum ) {
		case 1:
			filePath = new CPath( "Vivaldi.ogg" );
			printf("myTask( thread %d ) loading Vivaldi, path = %s\n", threadNum, filePath->c_str() );	
		break;

		case 2:
			filePath = new CPath( "VH_16_mono.ogg" );
			printf("myTask( thread %d ) found VH_16_mono, path = %s\n", threadNum, filePath->c_str() );	
		break;

		case 3:
			filePath = new CPath( "SFX.ogg" );
			printf("myTask( thread %d ) found SFX, path = %s\n", threadNum, filePath->c_str() );	
		break;

		case 4:
			filePath = new CPath( "Voice.ogg" );
			printf("myTask( thread %d ) found Voice, path = %s\n", threadNum, filePath->c_str() );	
		break;

	}
	
	id = audioMPI.StartAudio( *filePath, 100, 1, 0, &audioListener, 0, 0 );
	printf("myTask( thread %d ) back from calling StartAudio()\n", threadNum );

	// sleep 2 seconds
	kernelMPI.TaskSleep( 2000 ); 

	// loop sleep 1 second
	for (index = 0; index < 20; index++) {
		time = audioMPI.GetAudioTime( id );
		volume = audioMPI.GetAudioVolume( id );
		priority = audioMPI.GetAudioPriority( id );
		pan = audioMPI.GetAudioPan( id );
		pListener = audioMPI.GetAudioEventListener( id );
		printf("myTask( thread %d ) -- id = %d, time = %u, vol = %d, priority = %d, pan = %d, listener = 0x%x.\n", 
				threadNum, (int)id, (unsigned int)time, (int)volume, (int)priority, (int)pan, (unsigned int)pListener  );
		
		kernelMPI.TaskSleep( 125 ); 

		audioMPI.SetAudioVolume( id, volume - (index*2) );
		audioMPI.SetAudioPriority( id, priority + index );
		audioMPI.SetAudioPan( id, pan - (index*4));

		kernelMPI.TaskSleep( 125 ); 
	}

	// Stop audio instead of just bailing out of the thread 
	// so we can get done msg from player.
	audioMPI.StopAudio( id, false );
	
	// sleep 2 seconds waiting for completion even to post to listener.
	kernelMPI.TaskSleep( 2000 ); 

	printf("myTask( thread %d ) Exiting... \n" , threadNum );

	return (void *)NULL;
}	

//============================================================================
// TestAudioMgr functions
//============================================================================
class TestAudio : public CxxTest::TestSuite, TestSuiteBase 
{
private:
	CAudioMPI*			pAudioMPI_;
	CKernelMPI*			pKernelMPI_;
	AudioListener		audioListener_;

public:
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
		delete pAudioMPI_; 
		delete pKernelMPI_; 
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
	
	void xxxtestDumpCoreInfo( )
	{
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		tErrType err;
		Boolean fValid;
		tVersion version;
		ConstPtrCString pName;
		ConstPtrCURI pURI;
				
		printf("Extra Debug Output from test enabled...\n");

		fValid = pAudioMPI_->IsValid();
		printf("MPI IsValid = %d\n");
		
		if (pAudioMPI_->IsValid()) {
			pName = pAudioMPI_->GetMPIName(); 
			printf("MPI name is: %s\n", pName->c_str());
		
			version = pAudioMPI_->GetModuleVersion();
			printf("Module version is: %d\n", version);
			
			pName = pAudioMPI_->GetModuleName();
			printf("Module name is: %s\n", pName->c_str());
			
			pURI = pAudioMPI_->GetModuleOrigin();
			printf("Module Origin name is: %s\n", pURI->c_str());
		}		
#endif
	}


	//------------------------------------------------------------------------
	void xxxtestVorbisSimple( )
	{
		tAudioID 				id;

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		printf("TestAudio -- testVorbisResources() starting audio driver output\n" );

		id = pAudioMPI_->StartAudio( "vivaldi.ogg", 100, 1, 0, &audioListener_, 0, 0 );

		pKernelMPI_->TaskSleep( 5000 ); 
	}
	
	//------------------------------------------------------------------------
	void xxxtestMIDISimple( )
	{
		tErrType 		err;
		tAudioID 		id1;
		tMidiPlayerID	midiPlayerID;
		
		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );

		id1 = pAudioMPI_->StartMidiFile( midiPlayerID, "POWMusic.mid", 100, 1, &audioListener_, 0, 0 );

		// sleep 10 seconds
		pKernelMPI_->TaskSleep( 15000 ); 
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

		id1 = pAudioMPI_->StartAudio( "Sine_500Hz.ogg", 100, 1, 0, &audioListener_, 0, 1 );
		printf("TestAudio -- testVorbisResources() back from calling StartAudio()\n" );

		// loop sleep 1 second
		while ( seconds < 20 ) {
			pKernelMPI_->TaskSleep( 1000 ); 
		}
	}
		
	//------------------------------------------------------------------------
	void xxxtestVorbisPlusMIDIResources( )
	{
		tErrType 		err;
		U32				index;
		tAudioID 		id1, id2, id3;
		U32				audioTime;
		tMidiPlayerID	midiPlayerID;

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		printf("TestAudio -- testVorbisPlusMIDIResources()\n" );

		id1 = pAudioMPI_->StartAudio( "VH_16_mono.ogg", 100, 1, 0, &audioListener_, 0, 0 );
		printf("TestAudio -- testVorbisPlusMIDIResources() back from calling StartAudio()\n" );

		pKernelMPI_->TaskSleep(4000 ); 

		id2 = pAudioMPI_->StartAudio( "vivaldi.ogg", 80, 1, 0, &audioListener_, 0, 0 );
	
		pKernelMPI_->TaskSleep(4000 ); 

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );

		id3 = pAudioMPI_->StartMidiFile( midiPlayerID, "Neutr_3_noDrums.mid", 100, 1, &audioListener_, 0, 0 );

		// loop sleep 1 second
		for (index = 0; index < 20; index++) {
			pKernelMPI_->TaskSleep( 1000 ); 
		}
	}

	//------------------------------------------------------------------------
	void xxtestManyAudioResources( )
	{
		tErrType 		err;
		U16				i;
		tAudioID 		id1;
		tAudioID 		id2;
		tAudioID 		id3;
		tMidiPlayerID 	playerID;
		
		const int kDuration = 1 * 3000;

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		id1 = pAudioMPI_->StartAudio( "Music.ogg", 			// path/file
										100, 				// volume
										1, 					// priority
										0, 					// pan
										&audioListener_, 	// event listener
										0, 					// payload
										0 );				// flags

		// sleep 1 seconds
		pKernelMPI_->TaskSleep( 1000 ); 
	
		pAudioMPI_->SetMasterVolume( 30 );
		
		// sleep 1 seconds
		pKernelMPI_->TaskSleep( 1000 );

		pAudioMPI_->SetMasterVolume( 100 );

		pAudioMPI_->StopAudio( id1, false );
				
		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &playerID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		
	 	/*  MidiNote Params
						tMidiID midiID,
						U8 			track, 
						U8			pitch, 
						U8			velocity, 
						tAudioOptionsFlags	flags)
		*/
		pAudioMPI_->MidiNoteOn( playerID, 1, 64, 120, 0 );
		pKernelMPI_->TaskSleep( 1000 );
		pAudioMPI_->MidiNoteOff( playerID, 1, 64, 120, 0 );
		pKernelMPI_->TaskSleep( 1000 );
		pAudioMPI_->MidiNoteOn( playerID, 1, 68, 120, 0 );		
		pKernelMPI_->TaskSleep( 1000 );
		pAudioMPI_->MidiNoteOff( playerID, 1, 68, 120, 0 );
		pKernelMPI_->TaskSleep( 1000 );
		
/*
		tAudioID CAudioMPI::PlayMidiFile( tMidiPlayerID	playerID,
							tRsrcHndl		hRsrc, 
							U8					volume, 
							tAudioPriority		priority,
							IEventListener*		pHandler,
							tAudioPayload		payload,
							tAudioOptionsFlags	flags )
*/
		printf("About to start MIDI file\n");
		pAudioMPI_->StartMidiFile( playerID, "POWMusic.mid", 50, 1, &audioListener_, 0, 1 );
		


		id2 = pAudioMPI_->StartAudio( "Sine_500Hz.ogg", 50, 1, 0, &audioListener_, 0, 0 );

		// sleep 1 seconds
		pKernelMPI_->TaskSleep( 1000 );

		for (i = 0; i < 10; i++) {
			id3 = pAudioMPI_->StartAudio( "Sine_500Hz.ogg", i*10, 1, 0, &audioListener_, 0, 0 );
//			printf("TestAudio -- StartAudio() returned ID # %d\n", static_cast<int>(id3) );
//			if (id3 < 0) {
//				printf("TestAudio -- StartAudio() returned error # %d\n", static_cast<int>(id3) );
//			}
			pKernelMPI_->TaskSleep( 400 );
		}

		// sleep3 seconds
		pKernelMPI_->TaskSleep( kDuration );
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

		pProperties.TaskMainFcn = (void* (*)(void*))myTask;
 		threadArg.threadNum = 1;
		pProperties.pTaskMainArgValues = &threadArg;
		
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



