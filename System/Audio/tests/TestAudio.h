// TestAudio.h 

#include <cxxtest/TestSuite.h>
#include <boost/shared_array.hpp>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <AudioMPI.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <ResourceMPI.h>
#include <EventListener.h>
#include <EventMessage.h>
#include <EventMPI.h> 
#include <UnitTestUtils.h> 
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
 
// For lots of text output, enable this:
//#define	LF_BRIO_VERBOSE_TEST_OUTPUT

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// Audio Listener
//============================================================================
const tEventType kMyAudioTypes[] = { kAllAudioEvents };

//----------------------------------------------------------------------------
class AudioListener : public IEventListener
{
public:
	AudioListener( )
		: IEventListener(kMyAudioTypes, ArrayCount(kMyAudioTypes)), dbg_(kMyApp)
	{
	}

	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{
		const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
		if( msg.GetEventType() == kAudioCompletedEvent )
		{
			const tAudioMsgDataCompleted& data = msg.audioMsgData.audioCompleted;
			dbg_.DebugOut(kDbgLvlVerbose, "Audio done: id=%d, payload=%d\n", 
					static_cast<int>(data.audioID), static_cast<int>(data.payload));
			return kEventStatusOKConsumed;
		}
		return kEventStatusOK;
	}
private:
	CDebugMPI	dbg_;
};


//============================================================================
// TestAudioMgr functions
//============================================================================
class TestAudio : public CxxTest::TestSuite, TestSuiteBase 
{
private:
	CAudioMPI*			pAudioMPI_;
	CKernelMPI*			pKernelMPI_;
	CResourceMPI*		pResourceMPI_;
	AudioListener		audioListener_;
	tPackageHndl		pkg_;

public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pAudioMPI_ = new CAudioMPI();
		pKernelMPI_ = new CKernelMPI();
		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Audio");

		pkg_ = pResourceMPI_->FindPackage("SampleTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg_ );
		TS_ASSERT_EQUALS( kNoErr, pResourceMPI_->OpenPackage(pkg_) );
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pAudioMPI_; 
		delete pKernelMPI_; 

		pResourceMPI_->ClosePackage(pkg_);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
	}
	

	//------------------------------------------------------------------------
	void xxxtestWasCreated( )
	{
		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		TS_ASSERT( pResourceMPI_ != NULL );
		TS_ASSERT( pResourceMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void xxtestCoreMPI( )
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
	void xxxtestStartStopAudioSystem( )
	{
		tErrType err;
		const int kDuration = 1 * 1000;
		
		// Start up sine output.
		err = pAudioMPI_->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep3 seconds
		pKernelMPI_->TaskSleep( kDuration );

		// stop the engine.
		err = pAudioMPI_->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// Start up sine output.
		err = pAudioMPI_->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep 3 seconds
		pKernelMPI_->TaskSleep( kDuration );

		// stop the engine.
		err = pAudioMPI_->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// Start up sine output.
		err = pAudioMPI_->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep 3 seconds
		pKernelMPI_->TaskSleep( kDuration );

		// stop the engine.
		err = pAudioMPI_->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );
	}
	
	//------------------------------------------------------------------------
	void xxxtestVorbisResources( )
	{
		tErrType 	err;
		U32			index;
		tRsrcHndl	handle1;
		tRsrcHndl	handle2;
		tAudioID 	id1;
		tAudioID 	id2;
		U32			audioTime;

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		TS_ASSERT( pResourceMPI_ != NULL );
		TS_ASSERT( pResourceMPI_->IsValid() == true );

		printf("TestAudio -- testVorbisResources() starting audio driver output\n" );

		// Start up audio system.
		err = pAudioMPI_->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// Package is already opened in setup
		handle1 = pResourceMPI_->FindRsrc( "VH_16_mono" );
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		printf("TestAudio -- testVorbisResources() found VH_16_mono, rsrcHandle = %d\n", (int)handle1 );

		handle2 = pResourceMPI_->FindRsrc( "vivaldi" );
		TS_ASSERT( handle2 != kInvalidRsrcHndl );
		printf("TestAudio -- testVorbisResources() found vivaldi, rsrcHandle = %d\n", (int)handle2 );
//		handle = pResourceMPI_->FindRsrc( "BlueNile" );
//		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		
		
		// tRsrcHndl hRsrc, U8 volume,  tAudioPriority priority, S8 pan, 
		// IEventListener* pHandler, tAudioPayload payload, tAudioOptionsFlags flags)
		// volume is faked by right shift at this point
//		printf("TestAudio -- testVorbisResources() about to call StartAudio()\n" );
		id1 = 1;
//		id1 = pAudioMPI_->StartAudio( handle1, 100, 1, 0, &audioListener_, 0, 0 );
//		printf("TestAudio -- testVorbisResources() back from calling StartAudio()\n" );

		// sleep 2 seconds
//		pKernelMPI_->TaskSleep( 2000 ); 

//		pAudioMPI_->SetMasterVolume( 80 );
		id2 = pAudioMPI_->StartAudio( handle2, 100, 1, 0, &audioListener_, 0, 0 );

		// loop sleep 1 second
		for (index = 0; index < 20; index++) {
			audioTime = pAudioMPI_->GetAudioTime( id2 );
			pKernelMPI_->TaskSleep( 250 ); 
		}
	}
	
	//------------------------------------------------------------------------
	void xxxtestRawResources( )
	{
		tErrType 		err;
		tRsrcHndl		handle1;
		tRsrcHndl		handle2;
		tAudioID 		id1;
		tAudioID 		id2;

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		TS_ASSERT( pResourceMPI_ != NULL );
		TS_ASSERT( pResourceMPI_->IsValid() == true );

		printf("TestAudio -- testRawResources() starting audio driver output\n" );

		// Start up audio system.
		err = pAudioMPI_->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// Package is already opened in setup
		handle1 = pResourceMPI_->FindRsrc( "vivaldi44stereo" );
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		printf("TestAudio -- testRawResources() found vivaldi44stereo, rsrcHandle = %d\n", (int)handle1 );

		handle2 = pResourceMPI_->FindRsrc( "BlueNile" );
		TS_ASSERT( handle2 != kInvalidRsrcHndl );
		
		// tRsrcHndl hRsrc, U8 volume,  tAudioPriority priority, S8 pan, 
		// IEventListener* pHandler, tAudioPayload payload, tAudioOptionsFlags flags)
		// volume is faked by right shift at this point
		id1 = pAudioMPI_->StartAudio( handle1, 100, 1, 0, &audioListener_, 0, 0 );
		id2 = pAudioMPI_->StartAudio( handle2, 100, 1, 0, &audioListener_, 0, 0 );

		// sleep 10 seconds
		pKernelMPI_->TaskSleep( 15000 ); 
	}

	//------------------------------------------------------------------------
	void testMIDIResources( )
	{
		tErrType 		err;
		tRsrcHndl		handle1;
		tAudioID 		id1;
		tMidiPlayerID	midiPlayerID;
		
		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		TS_ASSERT( pResourceMPI_ != NULL );
		TS_ASSERT( pResourceMPI_->IsValid() == true );

		// Start up audio system.
		err = pAudioMPI_->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiPlayerID );		
		TS_ASSERT_EQUALS( kNoErr, err );

		// Package is already opened in setup
//		handle1 = pResourceMPI_->FindRsrc("Girl_noDrums");
		handle1 = pResourceMPI_->FindRsrc("Neutr_3_noDrums");
//		handle1 = pResourceMPI_->FindRsrc("NewHampshireGamelan");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		
		// tRsrcHndl hRsrc, U8 volume,  tAudioPriority priority, S8 pan, 
		// IEventListener* pHandler, tAudioPayload payload, tAudioOptionsFlags flags)
		// volume is faked by right shift at this point
		id1 = pAudioMPI_->StartMidiFile( midiPlayerID, handle1, 100, 1, &audioListener_, 0, 0 );

		// sleep 10 seconds
		pKernelMPI_->TaskSleep( 15000 ); 
	}
	
	//------------------------------------------------------------------------
	void xxxtestAudioResources( )
	{
		tErrType 		err;
		U16				i;
		tRsrcHndl		handle1;
		tRsrcHndl		handle2;
		tRsrcHndl		handle3;
		tRsrcHndl		handle4;
		tRsrcHndl		handle5;
		tAudioID 		id1;
		tAudioID 		id2;
		tAudioID 		id3;
		tMidiPlayerID 		midiID;
		
		const int kDuration = 1 * 3000;

		TS_ASSERT( pAudioMPI_ != NULL );
		TS_ASSERT( pAudioMPI_->IsValid() == true );
				
		TS_ASSERT( pKernelMPI_ != NULL );
		TS_ASSERT( pKernelMPI_->IsValid() == true );
		
		TS_ASSERT( pResourceMPI_ != NULL );
		TS_ASSERT( pResourceMPI_->IsValid() == true );

		// Start up audio system.
		err = pAudioMPI_->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// Package is already opened in setup
		handle1 = pResourceMPI_->FindRsrc("BlueNile");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		handle2 = pResourceMPI_->FindRsrc("sine44");
		TS_ASSERT( handle2 != kInvalidRsrcHndl );
		handle3 = pResourceMPI_->FindRsrc("vivaldi");
		TS_ASSERT( handle3 != kInvalidRsrcHndl );
		handle4 = pResourceMPI_->FindRsrc("FortyTwo");
		TS_ASSERT( handle4 != kInvalidRsrcHndl );
		handle5 = pResourceMPI_->FindRsrc("Girl_noDrums");
		TS_ASSERT( handle5 != kInvalidRsrcHndl );
		
		// tRsrcHndl hRsrc, U8 volume,  tAudioPriority priority, S8 pan, 
		// IEventListener* pHandler, tAudioPayload payload, tAudioOptionsFlags flags)
		// volume is faked by right shift at this point
		id1 = pAudioMPI_->StartAudio( handle1, 100, 1, 0, &audioListener_, 0, 0 );

		// sleep 1 seconds
		pKernelMPI_->TaskSleep( 1000 ); 
	
		pAudioMPI_->SetMasterVolume( 30 );
		
		// sleep 1 seconds
		pKernelMPI_->TaskSleep( 1000 );

		pAudioMPI_->SetMasterVolume( 100 );

		pAudioMPI_->StopAudio( id1, false );
				
		err = pAudioMPI_->AcquireMidiPlayer( 1, NULL, &midiID );		
		TS_ASSERT_EQUALS( kNoErr, err );
		
	 	/*  MidiNote Params
						tMidiID midiID,
						U8 			track, 
						U8			pitch, 
						U8			velocity, 
						tAudioOptionsFlags	flags)
		*/
		pAudioMPI_->MidiNoteOn( midiID, 1, 64, 120, 0 );
		pKernelMPI_->TaskSleep( 1000 );
		pAudioMPI_->MidiNoteOff( midiID, 1, 64, 120, 0 );
		pKernelMPI_->TaskSleep( 1000 );
		pAudioMPI_->MidiNoteOn( midiID, 1, 68, 120, 0 );		
		pKernelMPI_->TaskSleep( 1000 );
		pAudioMPI_->MidiNoteOff( midiID, 1, 68, 120, 0 );
		pKernelMPI_->TaskSleep( 1000 );
		
/*
		tAudioID CAudioMPI::PlayMidiFile( tMidiID	midiID,
							tRsrcHndl		hRsrc, 
							U8					volume, 
							tAudioPriority		priority,
							IEventListener*		pHandler,
							tAudioPayload		payload,
							tAudioOptionsFlags	flags )
*/
		printf("About to start MIDI file\n");
		pAudioMPI_->StartMidiFile( midiID, handle5, 50, 1, &audioListener_, 0, 1 );
		


		id2 = pAudioMPI_->StartAudio( handle2, 50, 1, 0, &audioListener_, 0, 0 );

		// sleep 1 seconds
		pKernelMPI_->TaskSleep( 1000 );

		for (i = 0; i < 10; i++) {
			id3 = pAudioMPI_->StartAudio( handle4, i*10, 1, 0, &audioListener_, 0, 0 );
//			printf("TestAudio -- StartAudio() returned ID # %d\n", static_cast<int>(id3) );
//			if (id3 < 0) {
//				printf("TestAudio -- StartAudio() returned error # %d\n", static_cast<int>(id3) );
//			}
			pKernelMPI_->TaskSleep( 200 );
		}

		// sleep3 seconds
		pKernelMPI_->TaskSleep( kDuration + kDuration + kDuration );

		// stop the engine.
		err = pAudioMPI_->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep3 seconds
		pKernelMPI_->TaskSleep( kDuration );

		// Start up sine output.
//		err = pAudioMPI_->StartAudio();
//		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep 3 seconds
//		pKernelMPI_->TaskSleep( kDuration );

		// stop the engine.
//		err = pAudioMPI_->StopAudio();
//		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep3 seconds
		pKernelMPI_->TaskSleep( kDuration );
	}

};



