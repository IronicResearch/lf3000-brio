//============================================================================
// TestMIDI : Torture Audio MPI  file from the command line.
//
//              Written by Sudhanshu Tewari and Gints Klimanis, 2007
//============================================================================

#define	PROGRAM_VERSION_STRING	"1.0"
#define	PROGRAM_DATE_STRING		"11/01/07"

// Includes
#include <assert.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <AudioMPI.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EmulationConfig.h>

#include "midi.h"

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

long doMIDINoteTest_All = false;

CAudioMPI*		pAudioMPI  = NULL;
CKernelMPI*		pKernelMPI = NULL;
tMidiPlayerID 	gMidiPlayerID;

// Leave active always
#define MIDI_PLAYER_ACQUIRE
// 
//#define TEST_MidiNoteOn /* works*/
//#define TEST_PlayMidi /* works*/
//#define TEST_StopMidi /* works -> clicks?*/
//#define TEST_LoopMidi /* works*/
//#define TEST_PauseMidi /* works -> clicks?*/
//#define TEST_EnableMidi //not working. needs correct Track Bit Mask
//#define TEST_ChangeMidiProgram_Special 
//#define TEST_ChangeMidiProgram //Not essential: more robust change many programs
//#define TEST_TransposeMidi // works!
//#define TEST_MidiTempoChange //to do
//#define TEST_SendMidiCommands // Test more. Needs help. Gints' code is broken
//#define TEST_RelativeMidiVolume // Test more. Play for Elise
//#define TEST_MidiFileAndNotes_BAD // code works fine. Not functional, this is okay
//#define TEST_MidiFileAndNotes_GOOD // code works fine. Not functional, this is okay
//#define TEST_DynamicMidiVolume // Works
//#define TEST_PlayTwoMidi // -> notes hang. Not a big issue.

//#define TEST_LoadInstrument // Needs work !!!!!

// Leave active always
#define MIDI_PLAYER_RELEASE

//#define TEST_PlayOgg
//#define TEST_PlayStereoOgg
//#define TEST_PlayStereoWav
//#define TEST_Play8kOgg
//#define TEST_Play16kOgg
//#define TEST_Play32kOgg
//#define TEST_Play8kWav
//#define TEST_Play16kWav
//#define TEST_Play32kWav
//#define TEST_Play8kWavMusic
//#define TEST_Play16kWavMusic
//#define TEST_Play32kWavMusic
//#define TEST_PauseOgg
//#define TEST_StopOgg
//#define TEST_Play4Ogg
//#define TEST_Play5thOgg
//#define TEST_OggDynamicPan
//#define TEST_OggDynamicVolume

//#define TEST_OggMidiWavRelativeVolumes
//#define TEST_PlayOggMidiWav
//#define TEST_PlayOggWav
//#define TEST_PlayOggMidi
//#define TEST_PlayWavMidi
#define TEST_SystemFunctions
#define TEST_MultiOggPause
#define TEST_MultiOggPause2
//#define TEST_GetAudioTime // Not working

U8 xStartU8 = 0;
U8 xEndU8   = 100;
U8 stepsU8  = 10;
U8 deltaU8;
U8 bank  = 0;

tMidiTrackBitMask trackbits;


S8 xStartS8 = 0;
S8 xEndS8   = 100;
S8 stepsS8  = 10;
S8 deltaS8;

long totalTime_milliSeconds = 5000;
long timeSlice_milliSeconds;

tErrType 		Ogg1;
tErrType 		Ogg2;
tErrType 		Ogg3;
tErrType 		Ogg4;
tErrType 		Ogg5;
tErrType 		Audio1;

//char *tMidiProgramList;

char *Movie2 = "/Didj/Data/TestMIDI/rsrc/Movie2_q2_16k.ogg";
char *Loop2 = "/Didj/Data/TestMIDI/rsrc/LoopForDennis_q2.ogg";
char *Belltree = "/Didj/Data/TestMIDI/rsrc/belltree_up2_32k.ogg";
char *NeutronOgg = "/Didj/Data/TestMIDI/rsrc/Neutr_3_noDrums.ogg";
char *NeutronWav = "/Didj/Data/TestMIDI/rsrc/Neutr_3_noDrums.wav";
char *Ogg8k = "/Didj/Data/TestMIDI/Ogg_8k/rsrc/beholdpowerofcheesealt.ogg";
char *Ogg16k = "/Didj/Data/TestMIDI/Ogg_16k/rsrc/beholdpowerofcheesealt.ogg";
char *Ogg32k = "/Didj/Data/TestMIDI/Ogg_32k/rsrc/beholdpowerofcheesealt.ogg";
char *Wav8k = "/Didj/Data/TestMIDI/Wav_8k/rsrc/beholdpowerofcheesealt.wav";
char *Wav16k = "/Didj/Data/TestMIDI/Wav_16k/rsrc/beholdpowerofcheesealt.wav";
char *Wav32k = "/Didj/Data/TestMIDI/Wav_32k/rsrc/beholdpowerofcheesealt.wav";

char *StereoWav16k = "/Didj/Data/TestMIDI/Wav_16k/rsrc/StereoTest.wav";
char *StereoOgg16k = "/Didj/Data/TestMIDI/Ogg_32k/rsrc/StereoTest.ogg";

U32 		    AudioTime;

U8 volume1 = 100; // 0-100 or 0-127?
U8 volume2 = 100;
U8 volume3 = 100;
U8 volume = 100;
U8 mastervolume = 100;
S8 pan = 0 ; // 0 center (-127-127)?
S8 rightpan = 127 ; 
S8 leftpan = -127 ; 
tAudioPriority priority = 0;
tAudioPayload loops = 0;

U8 midivol = 127;

// Note:  current bug , so use full file path, starting with '/'
char *midiFilePath = "/Didj/Data/TestMIDI/rsrc/CheeseLoop_Short.mid";
char *neutron = "/Didj/Data/TestMIDI/rsrc/Neutr_3_noDrums.mid";
char *france = "/Didj/Data/TestMIDI/rsrc/France_2Chan.mid";
char *francewdrum = "/Didj/Data/TestMIDI/rsrc/FrancewDrum.mid";
tAudioOptionsFlags flags = 0x02;


//pAudioMPI  = new CAudioMPI();
//pKernelMPI = new CKernelMPI();

//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif
//============================================================================
// GetAppRsrcFolder
//============================================================================
inline CPath GetAppRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir;
#else
	return "./";
#endif
}
// ==================================================================================
// EnableTrackBits:
// ==================================================================================
    void 	
EnableTrackBits( unsigned int bits )
// bits    sixteen bit fields
{
pAudioMPI->SetEnableMidiTracks( gMidiPlayerID, bits );
unsigned int gotbits = (unsigned int) pAudioMPI->GetEnabledMidiTracks( gMidiPlayerID );
printf("MIDIFileTest: TrackBits=$%04X \n", gotbits);
pKernelMPI->TaskSleep( 2000 );
}
// ==================================================================================
// EnableTrack:
// ==================================================================================
    void 	
EnableTrack( unsigned int number, unsigned int wait )
// number Range [1..16]
{
//unsigned int trackBits = pAudioMPI->GetEnabledMidiTracks(gMidiPlayerID);
unsigned int trackBits = pAudioMPI->GetEnabledMidiTracks(gMidiPlayerID);
trackBits |= 1<<(number-1);
printf("MIDIFileTest: EnableTrack=%d $%04X \n", number, trackBits);
pAudioMPI->SetEnableMidiTracks( gMidiPlayerID, trackBits);
unsigned int gottrack = (unsigned int) pAudioMPI->GetEnabledMidiTracks( gMidiPlayerID );
printf("MIDIFileTest: EnableTrack ActiveTracks=$%04X \n", gottrack);
pKernelMPI->TaskSleep( 1000*wait );
}
// ==================================================================================
// DisableTrack:
// ==================================================================================
    void 	
DisableTrack( unsigned int number )
// number Range [1..16]
{
unsigned int trackBits = pAudioMPI->GetEnabledMidiTracks(gMidiPlayerID);
printf("MIDIFileTest: DisableTrack%2d PRE trackBits=$%04X \n", number, trackBits);
unsigned int trackBitMask = ~(1<<(number-1));  // Take 1's complement = all tracks enabled but not 'number'

trackBits &= trackBitMask;   // logical AND operation: Remove any bit that is not inverted in the bit mask
//printf("MIDIFileTest: DisableTrack POST trackBits=$%04X \n", trackBits);

pAudioMPI->SetEnableMidiTracks( gMidiPlayerID, trackBits);
//printf("MIDIFileTest: DisableTrack=$%04X \n", trackBits);
//pKernelMPI->TaskSleep( 2000 );
}
// ==================================================================================
// EnableAllTracks:
// ==================================================================================
    void 	
EnableAllTracks( unsigned int wait )
// number Range [1..16]
{
pAudioMPI->SetEnableMidiTracks( gMidiPlayerID, 0xFFFF);
unsigned int gottrack = (unsigned int) pAudioMPI->GetEnabledMidiTracks( gMidiPlayerID );
printf("MIDIFileTest: EnableAllTracks=$%04X \n", gottrack);
pKernelMPI->TaskSleep( wait*1000 );
}
// ==================================================================================
// DisableAllTracks:
// ==================================================================================
    void 	
DisableAllTracks( unsigned int wait )
{
pAudioMPI->SetEnableMidiTracks( gMidiPlayerID, 0x0000);
unsigned int gottrack = (unsigned int) pAudioMPI->GetEnabledMidiTracks( gMidiPlayerID );
printf("MIDIFileTest: DisableAllTracks=$%04X \n", gottrack);
pKernelMPI->TaskSleep( wait*1000 );
}
// ==================================================================================
// Pitchbend:
// ==================================================================================
    void 	
Pitchbend(S8 bottom, S8 top)
//Pitchbend(S8 bottom, S8 top, unsigned int trackBitMask)
{
tMidiTrackBitMask trackBitMask = 0x0000;    // Silly, should be channels ???
printf("MIDIFileTest_PitchBend: START \n");

//trackBitMask = 0xFFFF;

S8 x = bottom;
for (S8 i = 0; i < 24; i++, x += 1)
    {
printf("Transpose to %d\n", x);
    pAudioMPI->TransposeMidiTracks( gMidiPlayerID, trackBitMask, x ); 
    pKernelMPI->TaskSleep( 1*20 );
    }
}
// ==================================================================================
// SleepSecs:
// ==================================================================================
    void 	
SleepSecs( int seconds )
{
pKernelMPI->TaskSleep( 1000*seconds );
}
// ==================================================================================
// Sleep:
// ==================================================================================
    void 	
Sleep( int milliSeconds )
{
pKernelMPI->TaskSleep( milliSeconds );
}
// ==================================================================================
// SleepMSec:
// ==================================================================================
    void 	
SleepMSec( int milliSeconds )
{
pKernelMPI->TaskSleep( milliSeconds );
}
/*
// ==================================================================================
// MIDIFileTest:
// ==================================================================================
    void 	
MIDIFileTest( void )
{
tErrType 		err;

err = pAudioMPI->AcquireMidiPlayer( 1, NULL, &gMidiPlayerID );		
pAudioMPI->StartMidiFile( gMidiPlayerID, "YouMIDIFileHere.mid", 100, 1, NULL, 0, 0 );

// ******************** doMIDIFileTest_List001 
{
printf("doMIDIFileTest_NewTest_001: START\n");
tErrType 		err;
err = pAudioMPI->AcquireMidiPlayer( 1, NULL, &gMidiPlayerID );		
pKernelMPI->TaskSleep( 3*1000 );
pAudioMPI->StartMidiFile( gMidiPlayerID, "joe.mid", 100, 1, NULL, 0, 0 );
pKernelMPI->TaskSleep( 5*1000 );
printf("doMIDIFileTest_NewTest: END\n");
printf("doMIDIFileTest_List001: START\n");

printf("MIDIFileTest_List001: END \n");
} // end doMIDIFileTest_List001

err = pAudioMPI->ReleaseMidiPlayer(gMidiPlayerID);
}   // ---- end MIDIFileTest() ----
*/
// ==================================================================================
// main:
// ==================================================================================
int	main( int argc, char* argv[] )
{
tErrType 		err;
//long i = 0;
pAudioMPI  = new CAudioMPI();
pKernelMPI = new CKernelMPI();

// Release Midi player before Acquire, just in case it was left active?
//err = pAudioMPI->ReleaseMidiPlayer(gMidiPlayerID);  // Deactivate MIDI engine

#ifdef MIDI_PLAYER_ACQUIRE
printf("Start Midi Player \n");
pAudioMPI->AcquireMidiPlayer( 1, NULL, &gMidiPlayerID );	// Starts up MIDI Engine & player
#endif

//----------------------------------------------
#ifdef TEST_LoadInstrument
printf("START: Test Load Instrument  \n");
printf("Start: Send Change Program to unloaded instr \n");
int chan = 0;
U8 bank  = 0;

char *france = "/Didj/Data/TestMIDI/rsrc/France_2Chan.mid";
// 2 tracks
// channel 1 = 56 trumpet melody
// channel 2 = 0 piano chords
printf("START: Play Midi  \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, france, 200, 1, kNull, 10, flags );	
SleepSecs(2);
printf("change program channel 1: instr 11 \n");
printf("Instr 11 not loaded, melody disappears \n");
printf("should hear melody on vibes \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, chan, 11);
SleepSecs(4);
printf("Stop Midi  \n");
pAudioMPI->StopMidiFile( gMidiPlayerID, true);

printf("Load Instr 11 \n");
// What is the name of the already used Midi Program list???
//pAudioMPI->AddToProgramList(*char tMidiProgramList, U8 bank, U8 program);
pAudioMPI->AddToProgramList(tMidiProgramList, bank, 11);

printf("Play Midi again \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, france, 200, 1, kNull, 10, flags );	
SleepSecs(2);
printf("change program channel 1: instr 11 \n");
printf("Instr 11 is loaded \n");
printf("should hear melody on vibes \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, chan, 11);

SleepSecs(6);

# endif // TEST_LoadInstrument

//----------------------------------------------
#ifdef TEST_MidiNoteOn
//----Test MidiNote On---------------------------
printf("START: Test MidiNote On \n");
printf("Note On: Piano \n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_C4, 100);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_C4);
SleepSecs(1);
printf("Note On: Trumpet C4\n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 56);	
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_C4, 100);
SleepSecs(2);
printf("Note Off: wrong note off G4. As Expected\n");
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_G5);
SleepSecs(2);
printf("Note Off: correct note off C4\n");
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_C4);
SleepSecs(1);
printf("Notes On: Vibes C major\n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 11);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 11);	
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 11);		
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_C4, 100);
SleepMSec(200);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 2, kMIDI_E4, 100);
SleepMSec(200);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 3, kMIDI_G4, 100);
SleepSecs(2);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 2, kMIDI_E4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 3, kMIDI_G4);
SleepSecs(1);
printf("Note On: Marimba \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 12);	
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_C4, 100, 0);
SleepSecs(3);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_C4, 100, 0);
printf("Note On: Calliope and Organ \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 82);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 17);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 2, kMIDI_C4, 100, 0);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 3, kMIDI_F5, 100, 0);
SleepSecs(2);
printf("Change Instrument 56 \n"); // not sure if this should work or not
printf("mid note program change doesn't work = expected \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 56);
SleepSecs(2);
printf("Note Off: Calliope and Organ \n");
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 2, kMIDI_C4, 100, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 3, kMIDI_F5, 100, 0);
printf("END: Test MidiNote On \n");
SleepSecs(2);
#endif //TEST_MidiNoteOn

#ifdef TEST_PlayMidi
//----Test Play Midi---------------------------
printf("Play Midi \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, 100, 1, kNull, 0, flags );	
SleepSecs(5);
printf("end of Midi file \n");
#endif // TEST_PlayMidi

#ifdef TEST_PlayTwoMidi 
//----Test Play Midi, 2nd Midi should cut off first--------------------------
printf("Play Midi #1 \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, 100, 1, kNull, 0, flags );	
SleepSecs(3);
printf("Play Midi #2 \n");
printf("Midi #2 should cut off Midi #1 \n");
printf("Notes hang from Midi #1 \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, 100, 1, kNull, 0, flags );	
SleepSecs(5);
printf("end of Midi file \n");
//pAudioMPI->StopMidiFile(gMidiPlayerID, true);
#endif // TEST_PlayTwoMidi 

#ifdef TEST_LoopMidi 
//----Test Play and Loop---------------------------
printf("Play Midi and Loop 1 times = Play twice \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, 100, 1, kNull, 1, flags );
SleepSecs(10);
//DAVE TEST: Use LoopDone flag to define loop point. This will use pListener
#endif // TEST_LoopMidi

#ifdef TEST_PauseMidi
//----Test Pause---------------------------
printf("Start Test: Pause Midi \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, midivol, 1, kNull, 4, flags );
SleepSecs(1);
printf("Pause Midi \n");
pAudioMPI->PauseMidiFile(gMidiPlayerID);
SleepSecs(2);
printf("Resume Midi \n");
pAudioMPI->ResumeMidiFile(gMidiPlayerID);
SleepSecs(4);
printf("Pause Midi \n");
pAudioMPI->PauseMidiFile(gMidiPlayerID);
SleepSecs(4);
printf("Resume Midi \n");
pAudioMPI->ResumeMidiFile(gMidiPlayerID);
SleepSecs(4);
printf("End Test: Pause Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif // TEST_PauseMidi

#ifdef TEST_StopMidi
//----- Test Stop (works)--------------------------------------
printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(1);
#endif // TEST_StopMidi

#ifdef TEST_DynamicMidiVolume
//----- Test DynamicMidiVolume --------------------------------------
pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, 100, 1, kNull, 4, flags );
printf("Dynamic Midi Volume \n");
printf("Play Midi  \n");
SleepSecs(4);
//Fade Out: vol from 100-0 make a function
//pAudioMPI->SetAudioVolume(Ogg1, vol);

xStartU8 = 0;
xEndU8   = 100;
stepsU8  = 10;
deltaU8  = (xEndU8 - xStartU8)/(stepsU8);
totalTime_milliSeconds = 5000;
timeSlice_milliSeconds = totalTime_milliSeconds/stepsU8;
printf("Volume ramp %3d -> %3d\n", xStartU8, xEndU8);
for (long i = 0; i <= stepsU8; i++, xStartU8 += deltaU8)
    {
printf("%3ld: SetAudioVolume %d\n", i, (int)xStartU8);
    pAudioMPI->SetAudioVolume(gMidiPlayerID, xStartU8);
    Sleep(timeSlice_milliSeconds);
    }
SleepSecs(4);
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif // TEST_DynamicMidiVolume

#ifdef TEST_EnableMidi
//----Test Enable Tracks (works)---------------------------
printf("Test Midi Track Enable\n");
pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, 100, 1, kNull, 20, flags );
SleepSecs(2);
//DAVE TEST: figure out how to get the correct Track Bit Mask in order to enable and disable (mute/unmute) tracks
printf("Try new method \n");
pAudioMPI->SetEnableMidiTracks( gMidiPlayerID, 0 );
unsigned int gotbits = (unsigned int) pAudioMPI->GetEnabledMidiTracks( gMidiPlayerID );
printf("MIDIFileTest: TrackBits=$%04X \n", gotbits);
SleepSecs(2);

printf("disable all tracks\n");
DisableAllTracks(1);
printf("enable all tracks\n");
EnableAllTracks(4);
printf("disable all tracks\n");
DisableAllTracks(1);

printf("enable channel 1\n");
EnableTrack(1,4);
printf("enable channel 2\n");
EnableTrack(2,2);
printf("enable channel 3\n");
EnableTrack(3,2);
printf("enable channel 4\n");
EnableTrack(4,2);
printf("enable channel 5\n");
EnableTrack(5,2);
printf("enable channel 10\n");
EnableTrack(10,2);
printf("disable all tracks\n");
DisableAllTracks(1);

printf("enable channel 10\n");
EnableTrack(10,4);
printf("enable channel 5\n");
EnableTrack(5,2);
printf("enable channel 3\n");
EnableTrack(3,2);
printf("enable channel 2\n");
EnableTrack(2,3);
printf("enable channel 4\n");
EnableTrack(4,2);
printf("enable channel 1\n");
EnableTrack(1,2);
printf("disable all tracks\n");
DisableAllTracks(0);
EnableTrack(5,2);
DisableTrack(5);
EnableTrack(1,2);
DisableTrack(1);
EnableTrack(2,2);
DisableTrack(2);
EnableTrack(3,2);
DisableTrack(3);
EnableTrack(4,2);
DisableTrack(4);
EnableTrack(10,2);
DisableTrack(10);
EnableTrack(5,2);
EnableAllTracks(4);
SleepSecs(2);

//pAudioMPI->PauseMidiFile(gMidiPlayerID);
//pAudioMPI->ResumeMidiFile(gMidiPlayerID);
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif// TEST_EnableMidi

#ifdef TEST_ChangeMidiProgram_Special
//----Test Change Midi Program Special test for loadable instr---------------------------
printf("Start: Test Change Program to unloaded instr \n");
int chan = 0;
char *france = "/Didj/Data/TestMIDI/rsrc/France_2Chan.mid";
// 2 tracks
// channel 1 = 56 trumpet melody
// channel 2 = 0 piano chords

pAudioMPI->StartMidiFile( gMidiPlayerID, france, 200, 1, kNull, 10, flags );	
SleepSecs(2);
//DAVE TEST: load instr 11 and make sure this is possible
printf("change program channel 1: instr 11 \n");
printf("Instr 11 not loaded, melody disappears \n");
printf("should hear melody on vibes \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, chan, 11);
SleepSecs(3);
printf("change program channel 1: instr 56, trumpet \n");
printf("Trumpet melody returns\n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, chan, 56);
SleepSecs(4);
printf("change program channel 2: instr 11 \n");
printf("Instr 11 not loaded, piano accomp disappears \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 11);
SleepSecs(4);
printf("change program channel 2: instr 0, piano \n");
printf("piano accomp returns \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 0);
SleepSecs(6);
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif // TEST_ChangeMidiProgram_Special

#ifdef TEST_ChangeMidiProgram
//----Test Change Midi Program (works)---------------------------
//Don't worry about this one
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 82);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 82);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 82);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 82);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 4, 82);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 5, 82);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 6, 82);
SleepSecs(4);

printf("Disable All Tracks \n");
DisableAllTracks(1);
printf("Enable Track 1 \n");
EnableTrack(1,2);
DisableTrack(1);
EnableTrack(2,2);
DisableTrack(2);
EnableTrack(3,2);
DisableTrack(3);
EnableTrack(4,2);
DisableTrack(4);
EnableTrack(5,2);
DisableTrack(5);
//EnableTrack(6,2);
//DisableTrack(6);
//EnableTrack(7,2);
//DisableTrack(7);
EnableTrack(8,2);
//printf("Test Change Program \n");
//pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 8, 82);
SleepSecs(2);
DisableTrack(8);
//EnableTrack(9,2);
//DisableTrack(9);
EnableTrack(10,2);
DisableTrack(10);
EnableTrack(11,2);
DisableTrack(11);
EnableAllTracks(4);
printf("Test Change Programs \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 9, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 8, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 10, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 11, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 4, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 5, 1);
SleepSecs(1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 9, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 8, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 10, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 11, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 4, 1);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 5, 1);
SleepSecs(10);
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif // TEST_ChangeMidiProgram

#ifdef TEST_TransposeMidi
//----Test Midi Transpose---------------------------
printf("START: Midi Transpose  \n");
printf("Play Midi File  \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, france, 100, 1, kNull, 0, flags );

// use france for 2 tracks
// channel 1 (midi channel bit = 0) = program 56 trumpet melody
// channel 2 (midi channel bit = 1) = program 0 piano chords
//use francewdrum for 3 tracks
// channel 15 (midi channel bit = 14) = program 56 trumpet melody
// channel 16 (midi channel bit = 15) = program 0 piano chords
// channel 10 (midi channel bit = 9) = drums

SleepSecs(2);
printf("Transpose melody: up a fifth\n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0001 , 7 ); 
SleepSecs(2);
printf("Transpose melody: up 2 octaves\n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0001 , 24 ); 
SleepSecs(2);
printf("Transpose melody: down 2 octaves\n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0001 ,-24 ); 
SleepSecs(2);
printf("Transpose accomp: down 2 octaves\n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0002 ,-24 ); 
SleepSecs(2);
printf("Transpose All: back to normal\n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0003 ,0 ); 
SleepSecs(2);
printf("Transpose All: up \n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0003 ,17 ); 
SleepSecs(2);
printf("Transpose All: back to normal \n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0003 , 0 ); 
SleepSecs(2);
printf("Transpose melody: up major 2nd\n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0001 , 2 ); 
SleepSecs(4);
printf("Transpose accomp: up major 2nd \n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0002 , 2 ); 
SleepSecs(4);
printf("Transpose All: up major 3rd\n");
pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0x0003 , 6 ); 
SleepSecs(6);
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif // TEST_TransposeMidi

#ifdef TEST_MidiTempoChange
//----Test Midi Tempo Changes---------------------------
printf("START: Midi Tempo Change  \n");
printf("Play Midi File  \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, france, 100, 1, kNull, 6, flags );
SleepSecs(2);
pAudioMPI->ChangeMidiTempo( gMidiPlayerID, -24 );
//DAVE TEST: figure out what the tempo scaling is and finish this test
//probably -127 to 127 or -100 to 100
SleepSecs(4);
pAudioMPI->ChangeMidiTempo( gMidiPlayerID, 0 );
SleepSecs(4);
pAudioMPI->ChangeMidiTempo( gMidiPlayerID, 127 );
SleepSecs(2);
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif // TEST_MidiTempoChange

#ifdef TEST_SendMidiCommands
//----Test SendMidiCommand---------------------------
printf("Test SendMidiCommand \n");
printf("Play Midi File  \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, france, 100, 1, kNull, 6, flags );
//pAudioMPI->SendMidiCommand( gMidiPlayerID, command, data1, data2 );

//Sudhu's code not written correctly
//printf("SendMidiCommand: Volume  \n");
//pAudioMPI->SendMidiCommand( gMidiPlayerID, 7, 1, 80);
//SleepSecs(2);
//printf("SendMidiCommand: Pan  \n");
//pAudioMPI->SendMidiCommand( gMidiPlayerID, 10, 1, 127);
//SleepSecs(2);

U8 byte1 = 0;
U8 byte2 = 0;
U8 midiChannel = 0;

#ifdef SAFE
printf("SendMidiCommand: Modulation \n");
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_ControlChange|midiChannel, kMIDI_Controller_Modulation, 127);
SleepSecs(1);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_ControlChange|midiChannel, kMIDI_Controller_Modulation, 32);
SleepSecs(1);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_ControlChange|midiChannel, kMIDI_Controller_Modulation, 0);
SleepSecs(1);
#endif // SOMETHING

#ifdef NOTWORKIN//kMIDIPitchWheel not defined
printf("SendMidiCommand: PitchBend  \n");
midiChannel = 0; byte1 = 0; byte2 = 63;
printf("SendMidiCommand: PitchBend ch%2x %03d %03d\n", midiChannel+1, byte1, byte2);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_PitchWheel|midiChannel, byte1, byte2); 
SleepSecs(4);
midiChannel = 0; byte1 = 64; byte2 = 0;
printf("SendMidiCommand: PitchBend ch%2x %03d %03d\n", midiChannel+1, byte1, byte2);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_PitchWheel|midiChannel, byte1, byte2);  // last two: LSB, MSB
SleepSecs(4);
midiChannel = 0; byte1 = 0; byte2 = 63;
printf("SendMidiCommand: PitchBend ch%2x %03d %03d\n", midiChannel+1, byte1, byte2);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_PitchWheel|midiChannel, byte1, byte2); 
SleepSecs(4);
midiChannel = 0; byte1 = 0; byte2 = 127;
printf("SendMidiCommand: PitchBend ch%2x %03d %03d\n", midiChannel+1, byte1, byte2);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_PitchWheel|midiChannel, byte1, byte2);   
SleepSecs(4);
midiChannel = 0; byte1 = 0; byte2 = 0;
printf("SendMidiCommand: PitchBend ch%2x %03d %03d\n", midiChannel+1, byte1, byte2);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_PitchWheel|midiChannel, byte1, byte2);  
SleepSecs(4);
midiChannel = 0; byte1 = 0; byte2 = 63;
printf("SendMidiCommand: PitchBend ch%2x %03d %03d\n", midiChannel+1, byte1, byte2);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_PitchWheel|midiChannel, byte1, byte2); 
SleepSecs(4);
midiChannel = 0; byte1 = 0; byte2 = 0;
printf("SendMidiCommand: PitchBend ch%2x %03d %03d\n", midiChannel+1, byte1, byte2);
pAudioMPI->SendMidiCommand( gMidiPlayerID, kMIDI_PitchWheel|midiChannel, byte1, byte2);  
SleepSecs(4);
#endif 
SleepSecs(2);
//DAVE TEST: get Gints to find kMIDIPicthWheel and get this working again. Maybe test AllNotesOff
printf("SendMidiCommand: All Notes Off  \n");
pAudioMPI->SendMidiCommand( gMidiPlayerID, 123, 1, 127);
SleepSecs(2);
printf("End Test  \n");
SleepSecs(2);
#endif // TEST_SendMidiCommands


#ifdef TEST_MidiFileAndNotes_BAD
//pAudioMPI->MidiNoteOn( gMidiPlayerID, channel, note , velocity, flags );
printf("START: Test Midi and Notes overlaid \n");
printf("One note \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 56);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_D6, 127);
SleepSecs(1);
printf("No Note Off \n");
//pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_D6);
SleepSecs(1);

printf("Play Midi \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, 100 /*127?*/, 1, kNull, 0, flags );
SleepSecs(4);

printf("One note over Mid file\n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 56);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C6, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C6);
SleepSecs(2);
printf("16 notes over Midi file \n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 2, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 3, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 4, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 5, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 6, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 7, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 8, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 9, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 10, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 11, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 12, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 13, kMIDI_C7, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 14, kMIDI_C5, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 15, kMIDI_C6, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 2, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 3, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 4, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 5, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 6, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 7, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 8, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 9, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 10, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 11, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 12, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 13, kMIDI_C7);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 14, kMIDI_C5);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 15, kMIDI_C6);
printf("One note over Mid file\n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 56);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4);
SleepSecs(1);
printf("One note over Mid file\n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4);
SleepSecs(1);
printf("program change on all channels \n");
printf("16 notes over Midi file \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 4, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 5, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 6, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 7, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 8, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 9, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 10, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 11, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 12, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 13, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 14, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 15, 58);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 2, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 3, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 4, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 5, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 6, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 7, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 8, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 9, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 10, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 11, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 12, kMIDI_D4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 13, kMIDI_D7, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 14, kMIDI_D5, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 15, kMIDI_D6, 127);
SleepSecs(2);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 2, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 3, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 4, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 5, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 6, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 7, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 8, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 9, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 10, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 11, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 12, kMIDI_D4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 13, kMIDI_D7);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 14, kMIDI_D5);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 15, kMIDI_D6);
SleepSecs(3);
printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(1);
printf("END: Test Midi and Notes overlaid \n");
#endif // TEST_MidiFileAndNotes_BAD

#ifdef TEST_MidiFileAndNotes_GOOD
//pAudioMPI->MidiNoteOn( gMidiPlayerID, channel, note , velocity, flags );
printf("START: Test Midi and Notes overlaid correctly \n");

printf("Play Midi \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, 20 /*127?*/, 1, kNull, 0, flags );
SleepSecs(4);

pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 11, 58);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 12, 15);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 13, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 14, 41);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 15, 8);

printf("One note over Mid file\n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 15, 56);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 15, kMIDI_C6, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 15, kMIDI_C6);
SleepSecs(2);
printf("Some notes over Midi file \n");
printf("note: channel 11 \n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 11, kMIDI_C4, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 11, kMIDI_C4);
SleepSecs(1);
printf("note: channel 12 \n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 12, kMIDI_C4, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 12, kMIDI_C4);
SleepSecs(1);
printf("note: channel 13 \n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 13, kMIDI_C7, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 13, kMIDI_C7);
SleepSecs(1);
printf("note: channel 14 \n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 14, kMIDI_C5, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 14, kMIDI_C5);
SleepSecs(1);
printf("note: channel 15 \n");
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 15, kMIDI_C6, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 15, kMIDI_C6);
SleepSecs(1);

printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(1);
printf("END: Test Midi and Notes overlaid \n");
#endif // TEST_MidiFileAndNotes_GOOD


#ifdef TEST_RelativeMidiVolume
//pAudioMPI->MidiNoteOn( gMidiPlayerID, channel, note , velocity, flags );
printf("START: Test Relative Midi volumes \n");
printf("One note \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 56);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4);
SleepSecs(1);
printf("16 notes (all same) \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 4, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 5, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 6, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 7, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 8, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 9, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 10, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 11, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 12, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 13, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 14, 56);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 15, 56);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 2, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 3, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 4, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 5, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 6, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 7, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 8, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 9, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 10, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 11, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 12, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 13, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 14, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 15, kMIDI_C4, 127);
SleepMSec(1000);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 2, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 3, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 4, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 5, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 6, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 7, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 8, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 9, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 10, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 11, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 12, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 13, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 14, kMIDI_C4);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 15, kMIDI_C4);
SleepMSec(1000);
printf("One note \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 56);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4);
SleepSecs(1);

printf("One note \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 18);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127, 0);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4, 127, 0);
SleepSecs(1);
printf("16 note chord \n");
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 0, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 1, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 2, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 3, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 4, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 5, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 6, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 7, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 8, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 9, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 10, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 11, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 12, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 13, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 14, 18);
pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, 15, 18);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 0, kMIDI_C4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 1, kMIDI_E4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 2, kMIDI_G4, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 3, kMIDI_C3, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 4, kMIDI_E3, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 5, kMIDI_G3, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 6, kMIDI_C2, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 7, kMIDI_E2, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 8, kMIDI_C1, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 9, kMIDI_G2, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 10, kMIDI_C5, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 11, kMIDI_E5, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 12, kMIDI_G5, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 13, kMIDI_C6, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 14, kMIDI_E6, 127);
pAudioMPI->MidiNoteOn ( gMidiPlayerID, 15, kMIDI_G6, 127);
SleepSecs(1);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 0, kMIDI_C4, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 15, kMIDI_G6, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 13, kMIDI_C6, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 12, kMIDI_G5, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 10, kMIDI_C5, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 11, kMIDI_E5, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 3, kMIDI_C3, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 4, kMIDI_E3, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 5, kMIDI_G3, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 14, kMIDI_E6, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 6, kMIDI_C2, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 9, kMIDI_G2, 127, 0);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 1, kMIDI_E4, 127, 0);
SleepMSec(500);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 7, kMIDI_E2, 127, 0);
SleepMSec(500);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 2, kMIDI_G4, 127, 0);
SleepMSec(500);
pAudioMPI->MidiNoteOff ( gMidiPlayerID, 8, kMIDI_C1, 127, 0);
SleepMSec(500);

printf("Play Midi File vol = 100\n");
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, 100 /*127?*/, 1, kNull, 0, flags );
SleepSecs(3);
printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepMSec(200);
printf("Play Midi File vol = 127 \n");
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, 100 /*127?*/, 1, kNull, 0, flags );
SleepSecs(6);

printf("END: Test Relative Midi volumes \n");
#endif // TEST_RelativeMidiVolume


//------------------------------------------------------------------------------------------
#ifdef TEST_OggMidiWavRelativeVolumes
//-----Test Relative Volumes: Ogg vs Midi----------------------------------------------------
volume = 100;
//printf("Volume %d\n", (int)volume);

//printf("Play Midi File: vol = 127 \n");
//pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, volume /*127?*/, 1, kNull, 0, flags );
//SleepSecs(6);
//printf("Stop Midi \n");
//pAudioMPI->StopMidiFile(gMidiPlayerID, true);

printf("Test Relative volumes: Ogg vs Midi \n");

printf("Play Wav \n");
printf("Volume %d\n", (int)volume);
Audio1 = pAudioMPI->StartAudio( NeutronWav, volume, kNull, pan, kNull, 0, flags );
SleepSecs(9);
printf("Stop Wav \n");
pAudioMPI->StopAudio( Audio1, kNull);
SleepSecs(1);

printf("Play Ogg Vorbis \n");
printf("Volume %d\n", (int)volume);
Ogg1 = pAudioMPI->StartAudio( NeutronOgg, volume, kNull, pan, kNull, 0, flags );
SleepSecs(9);
printf("Stop Ogg \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepSecs(1);

printf("Play Midi \n");
printf("Volume %d\n", (int)volume);
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, volume, 1, kNull, 0, flags );
SleepSecs(9);
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(1);
#endif // TES_OggMidiWavRelativeVolumes
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#ifdef TEST_PlayOggMidiWav
//-----Test Play Ogg, Midi, Wav----------------------------------------------------
volume = 100;
printf("Play Wav Right \n");
printf("Play Ogg Left \n");
printf("Play Midi Center \n");
printf("Volume %d\n", (int)volume);
Audio1 = pAudioMPI->StartAudio( NeutronWav, volume, kNull, 127, kNull, 0, flags );
Ogg1 = pAudioMPI->StartAudio( NeutronOgg, volume, kNull, -127, kNull, 0, flags );
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, volume, 1, kNull, 0, flags );
SleepSecs(9);
printf("Stop Wav \n");
pAudioMPI->StopAudio( Audio1, kNull);
SleepSecs(2);
printf("Stop Ogg \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepSecs(2);
printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif //TEST_PlayOggMidiWav
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#ifdef TEST_PlayOggWav
//-----Test Play Ogg, Wav----------------------------------------------------
volume = 100;
printf("Play Wav Right \n");
printf("Play Ogg Left \n");
printf("Volume %d\n", (int)volume);
Audio1 = pAudioMPI->StartAudio( NeutronWav, volume, kNull, 127, kNull, 0, flags );
Ogg1 = pAudioMPI->StartAudio( NeutronOgg, volume, kNull, -127, kNull, 0, flags );
SleepSecs(9);
printf("Stop Wav \n");
pAudioMPI->StopAudio( Audio1, kNull);
SleepSecs(2);
printf("Stop Ogg \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepSecs(2);
#endif //TEST_PlayOggWav
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#ifdef TEST_PlayOggMidi
//-----Test Play Ogg, Midi----------------------------------------------------
volume = 100;
printf("Play Wav \n");
printf("Play Midi \n");
printf("Volume %d\n", (int)volume);
Ogg1 = pAudioMPI->StartAudio( NeutronOgg, volume, kNull, 0, kNull, 0, flags );
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, volume, 1, kNull, 0, flags );
SleepSecs(9);
printf("Stop Ogg \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepSecs(2);
printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif //TEST_PlayOggMidi
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#ifdef TEST_PlayWavMidi
//-----Test Play Midi, Wav----------------------------------------------------
volume = 100;
printf("Play Wav \n");
printf("Play Midi \n");
printf("Volume %d\n", (int)volume);
Audio1 = pAudioMPI->StartAudio( NeutronWav, volume, kNull, 0, kNull, 0, flags );
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, volume, 1, kNull, 0, flags );
SleepSecs(9);
printf("Stop Wav \n");
pAudioMPI->StopAudio( Audio1, kNull);
SleepSecs(2);;
printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);
SleepSecs(2);
#endif //TEST_PlayWavMidi
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
#ifdef TEST_SystemFunctions
//-----Test Play, pause,resume Audio System----------------------------------------------------
volume = 100;
mastervolume = 127;
printf("Play Wav Right \n");
printf("Play Ogg Left \n");
printf("Play Midi Center \n");
printf("Volume %d\n", (int)volume);
Audio1 = pAudioMPI->StartAudio( NeutronWav, volume, kNull, 127, kNull, 0, flags );
Ogg1 = pAudioMPI->StartAudio( NeutronOgg, volume, kNull, -127, kNull, 0, flags );
pAudioMPI->StartMidiFile( gMidiPlayerID, neutron, volume, 1, kNull, 0, flags );
SleepSecs(4);

printf("Pause AudioSystem \n");
pAudioMPI->PauseAudioSystem();
SleepSecs(1);
printf("Resume AudioSystem \n");
pAudioMPI->ResumeAudioSystem();
SleepSecs(2);
printf("Pause AudioSystem \n");
pAudioMPI->PauseAudioSystem();
SleepSecs(1);
printf("Resume AudioSystem \n");
pAudioMPI->ResumeAudioSystem();
SleepSecs(4);
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(1);
mastervolume=80;
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(1);
mastervolume=60;
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(1);
mastervolume=20;
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(1);
mastervolume=100;
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(1);
mastervolume=127;
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(1);

xStartU8 = 0;
xEndU8   = 100;
stepsU8  = 10;
deltaU8  = (xEndU8 - xStartU8)/(stepsU8);
totalTime_milliSeconds = 5000;
timeSlice_milliSeconds = totalTime_milliSeconds/stepsU8;
printf("Master Volume ramp %3d -> %3d\n", xStartU8, xEndU8);

for (long i = 0; i <= stepsU8; i++, xStartU8 += deltaU8)
    {
printf("%3ld: SetAudioSystemVolume %d\n", i, (int)xStartU8);
    pAudioMPI->SetMasterVolume(xStartU8);
    Sleep(timeSlice_milliSeconds);
    }
SleepSecs(4);

#endif //TEST_SystemFunctions
//--------------------------------------------------------------------

//--------------------------------------------------------------------
#ifdef MIDI_PLAYER_RELEASE
printf("Stop Midi \n");
pAudioMPI->StopMidiFile(gMidiPlayerID, true);

//Causes hung notes to continue into Ogg playback
//pAudioMPI->PauseMidiFile(gMidiPlayerID);

printf("END Midi Test  \n");
SleepSecs(1);
printf("Stop Midi Player \n");
err = pAudioMPI->ReleaseMidiPlayer(gMidiPlayerID);  // Deactivate MIDI engine
#endif  // MIDI_PLAYER_RELEASE

#ifdef TEST_PlayOgg
//============================================================================================
// Test Audio
//============================================================================================
printf("Start Ogg Vorbis Test  \n");

Movie2 = "/Didj/Data/TestMIDI/rsrc/Movie2_q2_16k.ogg";
Loop2 = "/Didj/Data/TestMIDI/rsrc/LoopForDennis_q2.ogg";
Belltree = "/Didj/Data/TestMIDI/rsrc/belltree_up2_32k.ogg";

volume1 = 100;
volume2 = 100;
volume3 = 100;
pan = 0;
priority = 0;
loops = 10;

printf("Play Ogg Vorbis  \n");
Ogg1 = pAudioMPI->StartAudio( Loop2, volume1, kNull, pan, kNull, loops, flags );
SleepSecs(4);
printf("Play Ogg Vorbis 2 left  \n");
Ogg2 = pAudioMPI->StartAudio( Belltree, volume2, kNull, -127, kNull, 0, flags );
SleepSecs(1);
#endif // TEST_PlayOgg

//----------------------------------------------------------------------------------
#ifdef TEST_PlayStereoOgg

printf("START: 32k Stereo Ogg Vorbis Test  \n");
loops = 2;
printf("Play 32k Stereo Ogg Vorbis  \n");
printf("Loop 2 times  \n");
Ogg1 = pAudioMPI->StartAudio( StereoOgg16k, volume, kNull, pan, kNull, loops, flags );
SleepSecs(14);
printf("END: Stereo Ogg Vorbis Test \n");
#endif // TEST_PlayStereoOgg

//----------------------------------------------------------------------------------
#ifdef TEST_PlayStereoWav

printf("START: Stereo Wav Test  \n");
loops = 3;
printf("Play Stereo Wav  \n");
printf("Loop 3 times, total = 4  \n");
Ogg1 = pAudioMPI->StartAudio( StereoWav16k, volume, kNull, pan, kNull, loops, flags );
SleepSecs(12);
printf("Drop Master Volume  \n");
mastervolume=40;
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(2);
mastervolume=100;
printf("Set AudioSystem Volume: %d\n", (int)mastervolume);
printf("<click>\n");
pAudioMPI->SetMasterVolume(mastervolume);
SleepSecs(4);
printf("END: Stereo Wav Test \n");
#endif // TEST_PlayStereoOgg

//----------------------------------------------------------------
#ifdef TEST_Play8kOgg
printf("Play 8k Ogg Vorbis  \n");
Ogg1 = pAudioMPI->StartAudio( Ogg8k, volume, kNull, pan, kNull, 0, flags );
SleepSecs(4);
#endif // TEST_Play8kOgg

//----------------------------------------------------------------
#ifdef TEST_Play16kOgg
printf("Play 16k Ogg Vorbis  \n");
Ogg1 = pAudioMPI->StartAudio( Ogg16k, volume, kNull, pan, kNull, 0, flags );
SleepSecs(4);
#endif // TEST_Play16kOgg

//----------------------------------------------------------------
#ifdef TEST_Play32kOgg
printf("Play 32k Ogg Vorbis  \n");
Ogg1 = pAudioMPI->StartAudio( Ogg32k, volume, kNull, pan, kNull, 0, flags );
SleepSecs(4);
#endif // TEST_Play32kOgg

//----------------------------------------------------------------
#ifdef TEST_Play8kWav
printf("Play 8k Wav with one loop \n");
Wav8k = "/Didj/Data/TestMIDI/Wav_8k/rsrc/beholdpowerofcheesealt.wav";
Ogg1 = pAudioMPI->StartAudio( Wav8k, volume, kNull, pan, kNull, 1, flags );
SleepSecs(4);
#endif // TEST_Play8kWav

//----------------------------------------------------------------
#ifdef TEST_Play16kWav
printf("Play 16k Wav with one loop \n");
Ogg1 = pAudioMPI->StartAudio( Wav16k, volume, kNull, pan, kNull, 1, flags );
SleepSecs(4);
#endif // TEST_Play16kWav

//----------------------------------------------------------------
#ifdef TEST_Play32kWav
printf("Play 32k Wav with one loop \n");
Ogg1 = pAudioMPI->StartAudio( Wav32k, volume, kNull, pan, kNull, 1, flags );
SleepSecs(8);
SleepMSec(500);
printf("Stop 32k Wav \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepMSec(500);
#endif // TEST_Play32kWav

//----------------------------------------------------------------
#ifdef TEST_Play8kWavMusic
printf("Play 8k Wav Music \n");
Wav8k = "/Didj/Data/TestMIDI/Wav_8k/rsrc/FairlyOddBG.wav";
Ogg1 = pAudioMPI->StartAudio( Wav8k, volume, kNull, pan, kNull, 0, flags );
SleepSecs(8);
SleepMSec(500);
pAudioMPI->StopAudio( Ogg1, kNull);
SleepMSec(500);
#endif // TEST_Play8kWav

//----------------------------------------------------------------
#ifdef TEST_Play16kWavMusic
Wav16k = "/Didj/Data/TestMIDI/Wav_16k/rsrc/FairlyOddBG.wav";
printf("Play 16k Wav Music \n");
Ogg1 = pAudioMPI->StartAudio( Wav16k, volume, kNull, pan, kNull, 0, flags );
SleepSecs(8);
SleepMSec(500);
printf("Stop 16k Wav Music \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepMSec(500);
#endif // TEST_Play16kWavMusic

//----------------------------------------------------------------
#ifdef TEST_Play32kWavMusic
Wav32k = "/Didj/Data/TestMIDI/Wav_32k/rsrc/FairlyOddBG.wav";
printf("Play 32k Wav Music \n");
Ogg1 = pAudioMPI->StartAudio( Wav32k, volume, kNull, pan, kNull, 0, flags );
SleepSecs(8);
printf("Stop 32k Wav Music \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepMSec(500);
#endif // TEST_Play32kWavMusic

//----------------------------------------------------------------
#ifdef TEST_PauseOgg
//------ Test Pause, Resume------------------------------------
printf("Pause Ogg Vorbis \n");
pAudioMPI->PauseAudio( Ogg1 );
//pAudioMPI->PauseAudio( Ogg2 );
//pAudioMPI->PauseAudio( Ogg3 );
SleepSecs(1);

printf("Play Ogg Vorbis 2 right  \n");
Ogg2 = pAudioMPI->StartAudio( Belltree, volume2, kNull, 127, kNull, 0, flags );
SleepSecs(1);

printf("Resume Ogg Vorbis \n");
pAudioMPI->ResumeAudio( Ogg1 );
//pAudioMPI->ResumeAudio( Ogg2 );
//pAudioMPI->ResumeAudio( Ogg3 );
SleepSecs(4);
#endif // TEST_PauseOgg

#ifdef TEST_StopOgg
//---------Test Stop----------------------------------
printf("Stop Ogg Vorbis \n");
pAudioMPI->StopAudio( Ogg1, kNull);
pAudioMPI->StopAudio( Ogg2, kNull );
pAudioMPI->StopAudio( Ogg3, kNull );
SleepSecs(1);
#endif // TEST_StopOgg

#ifdef TEST_Play4Ogg
//---------Test 4 channel playback----------------
printf("Play More Ogg Vorbis +1 \n");
Ogg1 = pAudioMPI->StartAudio( Movie2, volume2, kNull, -127, kNull, 0, flags );
SleepMSec(1000);
printf("Play More Ogg Vorbis +2 \n");
Ogg2 = pAudioMPI->StartAudio( Movie2, volume2, kNull, 127, kNull, 0, flags );
SleepMSec(1000);
printf("Play More Ogg Vorbis +3 \n");
Ogg3 = pAudioMPI->StartAudio( Movie2, volume2, kNull, 0, kNull, 0, flags );
SleepMSec(1000);
printf("Play More Ogg Vorbis +4 \n");
Ogg4 = pAudioMPI->StartAudio( Movie2, volume2, kNull, 0, kNull, 0, flags );
SleepMSec(1000);
#endif // TEST_Play4Ogg

#ifdef TEST_Play5thOgg
//---------Test 4 channel playback----------------
// Additional channel return no channels available error
printf("Play More Ogg Vorbis +5 \n");
Ogg5 = pAudioMPI->StartAudio( Movie2, volume2, kNull, 0, kNull, 0, flags );
SleepMSec(1000);
SleepSecs(4);
printf("Stop Ogg Vorbis \n");
pAudioMPI->StopAudio( Ogg1, kNull);
pAudioMPI->StopAudio( Ogg2, kNull );
pAudioMPI->StopAudio( Ogg3, kNull );
pAudioMPI->StopAudio( Ogg4, kNull );
SleepSecs(1);
#endif // TEST_Play5thOgg

#ifdef TEST_OggDynamicPan
//-----Test Dynamic Pan--------------------------------------
printf("Play Ogg Vorbis and Pan \n");
volume2 = 100;
flags   = 0;
Ogg1 = pAudioMPI->StartAudio( Loop2, volume2, 1, 0, kNull, 0, flags );

printf("---- Smooth pan:  L->R \n");
xStartS8 = -100;
xEndS8   =  100;
stepsS8  = 10;
deltaS8  = (xEndS8 - xStartS8)/(stepsS8);
long totalTime_milliSeconds = 5000;
long timeSlice_milliSeconds = totalTime_milliSeconds/stepsS8;
//Fade In: vol from 0-100
for (long i = 0; i <= stepsS8; i++, xStartS8 += deltaS8)
    {
printf("%3ld: SetAudioPan %d\n", i, (int)xStartS8);
    pAudioMPI->SetAudioPan(Ogg1, xStartS8);
    Sleep(timeSlice_milliSeconds);
    }

printf("---- Smooth pan:  R->L \n");
xStartS8 =  100;
xEndS8   = -100;
stepsS8  = 10;
deltaS8  = (xEndS8 - xStartS8)/(stepsS8);
totalTime_milliSeconds = 5000;
timeSlice_milliSeconds = totalTime_milliSeconds/stepsS8;
//Fade In: vol from 0-100
for (long i = 0; i <= stepsS8; i++, xStartS8 += deltaS8)
    {
printf("%3ld: SetAudioPan %d\n", i, (int)xStartS8);
    pAudioMPI->SetAudioPan(Ogg1, xStartS8);
    Sleep(timeSlice_milliSeconds);
    }

printf("Crazy pans\n");
pAudioMPI->SetAudioPan(Ogg1, 100);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 90);
pAudioMPI->SetAudioPan(Ogg1, 90);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 80);
pAudioMPI->SetAudioPan(Ogg1, 80);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 70);
pAudioMPI->SetAudioPan(Ogg1, 70);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 60);
pAudioMPI->SetAudioPan(Ogg1, 60);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 50);
pAudioMPI->SetAudioPan(Ogg1, 50);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 40);
pAudioMPI->SetAudioPan(Ogg1, 40);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 30);
pAudioMPI->SetAudioPan(Ogg1, 30);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 20);
pAudioMPI->SetAudioPan(Ogg1, 20);
SleepMSec(200);
pAudioMPI->SetAudioVolume(Ogg1, 10);
pAudioMPI->SetAudioPan(Ogg1, 10);
SleepMSec(200);
AudioMPI->SetAudioVolume(Ogg1, 0);
pAudioMPI->SetAudioPan(Ogg1, 0);
SleepMSec(200);

pAudioMPI->SetAudioVolume(Ogg1, 100);

printf("Ping pong pans ...\n");
pAudioMPI->SetAudioPan(Ogg1, 0);
for (long i = 0; i < 10; i++)
    {
    S8 panV;
    Sleep(500);
    if (i & 1)
        panV = -100;
    else
        panV =  100;
    pAudioMPI->SetAudioPan(Ogg1,  panV);
printf("%3ld: SetAudioPan %3d\n", i, (int)panV);
    }
pAudioMPI->SetAudioPan(Ogg1, 0);

#endif // TEST_OggDynamicPan

#ifdef TEST_OggDynamicVolume
//-----Test Dynamic Volume--------------------------------------
printf("Dynamic Volume \n");
Ogg1 = pAudioMPI->StartAudio( Loop2, volume2, kNull, pan, kNull, 0, flags );
SleepSecs(1);

//Fade Out: vol from 100-0 make a function
//pAudioMPI->SetAudioVolume(Ogg1, vol);

xStartU8 = 0;
xEndU8   = 100;
stepsU8  = 10;
deltaU8  = (xEndU8 - xStartU8)/(stepsU8);
totalTime_milliSeconds = 5000;
timeSlice_milliSeconds = totalTime_milliSeconds/stepsU8;
printf("Volume ramp %3d -> %3d\n", xStartU8, xEndU8);
//Fade In: vol from 0-100
for (long i = 0; i <= stepsU8; i++, xStartU8 += deltaU8)
    {
printf("%3ld: SetAudioVolume %d\n", i, (int)xStartU8);
    pAudioMPI->SetAudioVolume(Ogg1, xStartU8);
    Sleep(timeSlice_milliSeconds);
    }
SleepSecs(4);
#endif // TEST_OggDynamicVolume
//------------------------------------------------------------------------

//------------------------------------------------------------------------
#ifdef TEST_MultiOggPause
//-----Test Ogg multi channel pause---------------------------------------
printf("Test: Start multi OGG Pause  \n");
volume=100;
printf("Play Ogg Vorbis  \n");
Ogg1 = pAudioMPI->StartAudio( Loop2, volume, kNull, 0, kNull, 2, flags );
printf("Play Ogg Vorbis 2 left \n");
Ogg2 = pAudioMPI->StartAudio( Loop2, volume, kNull, -127, kNull, 2, flags );
printf("Play Ogg Vorbis 3 right \n");
Ogg3 = pAudioMPI->StartAudio( Loop2, volume, kNull, 127, kNull, 2, flags );

SleepSecs(4);
printf("Pause Ogg 1  \n");
pAudioMPI->PauseAudio(Ogg1);
SleepSecs(1);
printf("Pause Ogg 2  \n");
pAudioMPI->PauseAudio(Ogg2);
SleepSecs(1);
printf("Pause Ogg 3  \n");
pAudioMPI->PauseAudio(Ogg3);
SleepSecs(1);
printf("Resume Ogg 3  \n");
pAudioMPI->ResumeAudio(Ogg3);
SleepSecs(1);
printf("Resume Ogg 2  \n");
pAudioMPI->ResumeAudio(Ogg2);
SleepSecs(2);
printf("Resume Ogg 1  \n");
pAudioMPI->ResumeAudio(Ogg1);
SleepSecs(2);
printf("Pause Ogg 3  \n");
pAudioMPI->PauseAudio(Ogg3);
SleepSecs(3);
printf("Pause Ogg 2  \n");
pAudioMPI->PauseAudio(Ogg2);
SleepSecs(2);
printf("Back in Sync  \n");
printf("Resume Ogg 3  \n");
pAudioMPI->ResumeAudio(Ogg3);
SleepSecs(1);
printf("Resume Ogg 2  \n");
pAudioMPI->ResumeAudio(Ogg2);
SleepSecs(8);
printf("Stop Ogg 3  \n");
pAudioMPI->StopAudio(Ogg3, kNull);
SleepSecs(6);
printf("Stop Ogg 2  \n");
pAudioMPI->StopAudio(Ogg2, kNull);
printf("Stop Ogg 1  \n");
pAudioMPI->StopAudio(Ogg1, kNull);
SleepSecs(1);
printf("End multi Ogg pause test  \n");
#endif // TEST_MultiOggPause
//------------------------------------------------------------------------

//------------------------------------------------------------------------
#ifdef TEST_MultiOggPause2
//-----Test Ogg multi channel pause, with 4th channel insertion---------------------------------------
printf("Test: Start multi OGG Pause  \n");
volume=100;
printf("Play Ogg Vorbis  \n");
Ogg1 = pAudioMPI->StartAudio( Loop2, volume, kNull, 0, kNull, 2, flags );
printf("Play Ogg Vorbis 2 left \n");
Ogg2 = pAudioMPI->StartAudio( Loop2, volume, kNull, -127, kNull, 2, flags );
printf("Play Ogg Vorbis 3 right \n");
Ogg3 = pAudioMPI->StartAudio( Loop2, volume, kNull, 127, kNull, 2, flags );
SleepSecs(4);
printf("Pause Ogg 1  \n");
pAudioMPI->PauseAudio(Ogg1);
printf("Pause Ogg 2  \n");
pAudioMPI->PauseAudio(Ogg2);
printf("Pause Ogg 3  \n");
pAudioMPI->PauseAudio(Ogg3);

SleepMSec(250);
Ogg4 = pAudioMPI->StartAudio( Belltree, volume, kNull, 0, kNull, 0, flags );
SleepSecs(3);

printf("Resume Ogg 3  \n");
pAudioMPI->ResumeAudio(Ogg3);
printf("Resume Ogg 2  \n");
pAudioMPI->ResumeAudio(Ogg2);
printf("Resume Ogg 1  \n");
pAudioMPI->ResumeAudio(Ogg1);
SleepSecs(4);

Ogg4 = pAudioMPI->StartAudio( Belltree, volume, kNull, 0, kNull, 0, flags );
SleepSecs(3);

printf("Stop Ogg 1 Center \n");
pAudioMPI->StopAudio(Ogg3, kNull);
SleepMSec(1000);
printf("Stop Ogg 2 Left \n");
pAudioMPI->StopAudio(Ogg2, kNull);
SleepMSec(1000);
printf("Stop Ogg 3 Right \n");
pAudioMPI->StopAudio(Ogg1, kNull);
SleepSecs(1);
printf("End multi Ogg pause test  \n");
#endif // TEST_MultiOggPause2
//------------------------------------------------------------------------

//----------------------------------------------------------------
#ifdef TEST_GetAudioTime
printf("Play 8k Wav Music \n");
Wav8k = "/Didj/Data/TestMIDI/Wav_8k/rsrc/FairlyOddBG.wav";
Ogg1 = pAudioMPI->StartAudio( Wav8k, volume, kNull, pan, kNull, 0, flags );
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
AudioTime = pAudioMPI->GetAudioTime( Ogg1);
printf("AudioTime = %3d \n", (int)AudioTime);
SleepSecs(1);
printf("Stop 8k Wav Music \n");
pAudioMPI->StopAudio( Ogg1, kNull);
SleepMSec(500);
#endif // TEST_GetAudioTime

//printf("Sleeping for 30 secs\n");
//SleepSecs(30);

//******************************************
 	delete pAudioMPI; 
	delete pKernelMPI;
	return 0;

printf("timeout  \n");

}   // ---- end main() ----
//******************************************



//---REFERENCE----------------------------------------------------------------------------
//pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, 200, 1, kNull, 10, flags );	
// ID, file path, 
// volume [0..100] -> [-96 dB .. 0 dB]
// priority not implemented - no need to test for noew
// Listener for audio done message -. no need to test for now
// payload , currently, just # of addition iterations for looping
// flags  = kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped
//	kAudioOptionsNoDoneMsg				= 0x00,	// No done message
//	kAudioOptionsDoneMsgAfterComplete	= 0x01,	// Done message after audio is complete 
//	kAudioOptionsLooped					= 0x02	// Loop audio sample playback
//
//pAudioMPI->StartMidiFile( gMidiPlayerID, midiFilePath, 200, 1, kNull, 10, flags );	
//pAudioMPI->PauseMidiFile(gMidiPlayerID);
//pAudioMPI->ResumeMidiFile(gMidiPlayerID);
//pAudioMPI->StopMidiFile(gMidiPlayerID, true);
//pAudioMPI->ChangeMidiTempo( gMidiPlayerID, gMIDI_TempoChange ); 
//pAudioMPI->TransposeMidiTracks( gMidiPlayerID, 0xFFFF, gMIDI_TransposeTracks ); 
//pAudioMPI->ChangeMidiInstrument(gMidiPlayerID, channel, gMIDI_ProgramNumber);

//pAudioMPI->SendMidiCommand( gMidiPlayerID, command, data1, data2 );

//pAudioMPI->MidiNoteOn( gMidiPlayerID, channel, note , velocity, flags );
//pAudioMPI->MidiNoteOff( gMidiPlayerID, channel, note , velocity, flags );
//
//channel  = 0;
//note     = kMIDI_C4;
//velocity = 127;
//flags    = 0;
//delay    = 125;
//    NoteOn_Wait( channel, kMIDI_C2 , velocity );
//    NoteOn_Wait( channel, kMIDI_F2 , velocity );
//    NoteOn_Wait( channel, kMIDI_Bb2, velocity );
//    NoteOn_Wait( channel, kMIDI_C3 , velocity );
//
//    NoteOff( channel, kMIDI_C2 );
//    NoteOff( channel, kMIDI_F2 );
//    NoteOff( channel, kMIDI_Bb2);
//    NoteOff( channel, kMIDI_C3 );
//    pKernelMPI->TaskSleep( delay );
//    }
//
//----------------------------------------------------------------------------------------


