//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// 
// Audio.cpp
//
// This is implementation of the AudioMPI.  It is the front-end of the audio
// system, and does little more than initialize the system and get and set the
// state of the mixer.  See Mixer.cpp for all of the gory details about how
// audio is rendered.
//
//==============================================================================
#include <errno.h>
#include <sys/stat.h>
#include <SystemTypes.h>
#include <SystemErrors.h>

#include <EventMPI.h>

#include <AudioTypes.h>
#include <AudioPriv.h>
#include <AudioTypesPriv.h>
#include <Mixer.h>
#include <AudioPlayer.h>
#include <MidiPlayer.h>

#include <algorithm>
#include <map>
#include <set>

LF_BEGIN_BRIO_NAMESPACE()

namespace
{
    struct tAudioContext {
        
        CAudioMixer*        pAudioMixer;        
        CMidiPlayer*        pMidiPlayer;
        
        U16                 outBufferSizeInBytes;       
        U16                 outBufferSizeInWords;   
        tAudioID            nextAudioID;    
        
        U8                  masterVolume;       // GK NOTE: free of the burden of "units"
        U8                  outputEqualizerEnabled;     
        U8                  numMixerChannels;
        U32                 samplingFrequency; // Hertz
        
        IEventListener*     pDefaultListener;
        CPath               gpath;
    };

}

tAudioContext gAudioContext;

//==============================================================================
// Defines
//==============================================================================

#define AUDIO_LOCK pDebugMPI_->Assert((kNoErr == pKernelMPI_->LockMutex(mpiMutex_)),\
                                      "Couldn't lock mutex.\n")

#define AUDIO_UNLOCK pDebugMPI_->Assert((kNoErr == pKernelMPI_->UnlockMutex(mpiMutex_)),\
                                        "Couldn't unlock mutex.\n");

//==============================================================================
// Global variables
//==============================================================================

const CURI  kModuleURI = "/Somewhere/AudioModule";

// single instance of module object.
static CAudioModule* sinst = NULL;

//============================================================================
// MPI state and utility functions
//============================================================================
namespace
{
    //------------------------------------------------------------------------
    // MPIInstanceState
    //------------------------------------------------------------------------
    struct MPIInstanceState
    {
        //--------------------------------------------------------------------
        // Each MPI interface object has some state (default listener, volume 
        // pan, etc.).  The Audio module stores this state on
        // behalf of an interface object in this MPIInstanceState class.
        // The CAudioMPI ctor calls CAudioModule::Register() to create
        // an instance of this class that gets added to a global "gMPIMap".
        // Register() returns a U32 key into this map, and every call from
        // the MPI interface object to the CResourceModule passes this U32
        // key so that the operation uses that interface object's state.
        // CAudioMPI instance.
        //--------------------------------------------------------------------
        MPIInstanceState( void )
                        : path(NULL),
                        pListener(NULL),
                        volume(100),
                        pan(0),
                        priority(0)
        { }
                        
        const CPath*            path;
        const IEventListener*   pListener;
        U8                      volume;
        S8                      pan;
        tAudioPriority          priority;
    };
    typedef std::map<U32, MPIInstanceState> MPIMap;
    
    MPIMap gMPIMap;
    
    //============================================================================
    // Utility functions
    //============================================================================
    //----------------------------------------------------------------------------
    MPIInstanceState& RetrieveMPIState(U32 id)
    {
        // Register() creates an MPIInstanceState entry in "gMPIMap" and 
        // returns a key/U32 id that the MPI instance uses to identify itself
        // in subsequent calls.
        // This function retrieves the MPIInstanceState for a given key.
        // FIXME/tp: multithreading issues.. MAYBE.  Should be safe
        // because all calling functions are already mutex protected.
        //
        MPIMap::iterator it = gMPIMap.find(id);
        if (it != gMPIMap.end())
            return it->second;
        
        CDebugMPI   dbg(kGroupAudio);
        dbg.Assert(false, "CAudioModule::RetrieveMPIState: configuration failure,"
                   "unregistered MPI id!");

        return it->second;  // dummy return to avoid compiler warning
    }
} // end anon namespace

//==============================================================================
// CAudioModule  Class constructor
//==============================================================================
CAudioModule::CAudioModule( void )
{

    tErrType            err = kNoErr;
    Boolean             ret = false;
    const tMutexAttr    attr = {0};

    gAudioContext.gpath = "";

    // Get Debug MPI
    pDebugMPI_ =  new CDebugMPI(kGroupAudio);
    ret = pDebugMPI_->IsValid();
    pDebugMPI_->Assert((true == ret),
                       "CAudioModule::ctor -- Couldn't create DebugMPI.\n");

    // Set debug level from a constant
    pDebugMPI_->SetDebugLevel(kAudioDebugLevel);
    

    // Get Kernel MPI
    pKernelMPI_ =  new CKernelMPI();
    ret = pKernelMPI_->IsValid();
    pDebugMPI_->Assert((true == ret),
                       "CAudioModule::ctor -- Couldn't create KernelMPI.\n");

    // Init mutex for serialization access to internal AudioMPI state.
    err = pKernelMPI_->InitMutex( mpiMutex_, attr );
    pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Couldn't init mutex.\n");

    pDebugMPI_->DebugOut(kDbgLvlVerbose, 
                         "CAudioModule::ctor -- Initalizing Audio Module...");

    // Hard code configuration resource
    gAudioContext.numMixerChannels  = kAudioNumMixerChannels;
    gAudioContext.samplingFrequency = kAudioSampleRate;

    // Set output buffer sizes.  These values are based on
    // 20ms buffer of stereo samples: see AudioConfig.h
    gAudioContext.outBufferSizeInWords = kAudioOutBufSizeInWords;
    gAudioContext.outBufferSizeInBytes = kAudioOutBufSizeInBytes;

    gAudioContext.nextAudioID = BRIO_MIDI_PLAYER_ID + 1;
    
    // Allocate global audio mixer
    gAudioContext.pAudioMixer = new CAudioMixer(kAudioNumMixerChannels);
    
    gAudioContext.pMidiPlayer = gAudioContext.pAudioMixer->GetMidiPlayerPtr();
    
    // GK FIXX TODO: hack, should get this from the mixer.
    gAudioContext.masterVolume = 100;
    gAudioContext.outputEqualizerEnabled = false;
}

//==============================================================================
// CAudioModule
//==============================================================================
CAudioModule::~CAudioModule( void )
{
    tErrType result = kNoErr;
    
    AUDIO_LOCK;
    
    pDebugMPI_->DebugOut( kDbgLvlVerbose, 
                          "CAudioModule::dtor: dtor called\n" );    
        
    delete gAudioContext.pAudioMixer;
    
    AUDIO_UNLOCK;
    //In theory, somebody could preempt us here and make an MPI call that would
    //lock the mutex.  However, we assume the programmer will not call this
    //destructor unless it no longer plans on accessing the MPI.
    pKernelMPI_->DeInitMutex( mpiMutex_ );

    delete pKernelMPI_;
    delete pDebugMPI_;

}

//============================================================================
// Informational functions
//============================================================================
Boolean CAudioModule::IsValid() const
{
    return (sinst != NULL) ? true : false; 
}

//----------------------------------------------------------------------------
tVersion CAudioModule::GetModuleVersion() const
{
    return kAudioModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CAudioModule::GetModuleName() const
{
    return &kAudioModuleName;
}

//----------------------------------------------------------------------------
const CURI* CAudioModule::GetModuleOrigin() const
{
    return &kModuleURI;
}   // ---- end GetModuleOrigin() ----

//==============================================================================
// MPI State Functions
//==============================================================================

// ==============================================================================
// Register
// ==============================================================================
U32 CAudioModule::Register( void )
{
    static U32  sNextIndex = 0;
    
    AUDIO_LOCK;

    gMPIMap.insert(MPIMap::value_type(++sNextIndex, MPIInstanceState()));

    AUDIO_UNLOCK;

    return sNextIndex;
}   // ---- end Register() ----

// ==============================================================================
// Unregister
// ==============================================================================
void CAudioModule::Unregister( U32 id )
{
    AUDIO_LOCK;
    MPIMap::iterator it = gMPIMap.find(id);
    if (it != gMPIMap.end())
    {
        gMPIMap.erase(it);
    }
    AUDIO_UNLOCK;
}

// ==============================================================================
// PauseAudioSystem
// ==============================================================================
tErrType CAudioModule::PauseAudioSystem( void )
{
    AUDIO_LOCK;
    gAudioContext.pAudioMixer->Pause();
    AUDIO_UNLOCK;

    return kNoErr;
}

// ==============================================================================
// ResumeAudioSystem
// ==============================================================================
tErrType CAudioModule::ResumeAudioSystem( void )
{
    AUDIO_LOCK;
    gAudioContext.pAudioMixer->Resume();
    AUDIO_UNLOCK;

    return kNoErr;
}

// ==============================================================================
// RegisterAudioEffectsProcessor
// ==============================================================================
tErrType
CAudioModule::RegisterAudioEffectsProcessor(CAudioEffectsProcessor * /* pChain */)
{
    return kNoImplErr;
}

// ==============================================================================
// RegisterGlobalAudioEffectsProcessor
// ==============================================================================
tErrType 
CAudioModule::RegisterGlobalAudioEffectsProcessor(CAudioEffectsProcessor * /* pChain */)
{
    return kNoImplErr;
}

// ==============================================================================
// ChangeAudioEffectsProcessor
// ==============================================================================
tErrType
CAudioModule::ChangeAudioEffectsProcessor(tAudioID /* id */,
                                          CAudioEffectsProcessor * /* pChain */)
{
    return kNoImplErr;
}

// ==============================================================================
// SetMasterVolume
// ==============================================================================
void CAudioModule::SetMasterVolume(U8 x)
{
    AUDIO_LOCK;
    gAudioContext.masterVolume = x;
    gAudioContext.pAudioMixer->SetMasterVolume(gAudioContext.masterVolume);
    AUDIO_UNLOCK;

}

// ==============================================================================
// GetMasterVolume
// ==============================================================================
U8 CAudioModule::GetMasterVolume( void )
{
    U8 v;

    AUDIO_LOCK;
    // TODO: This probably should call through to the mixer...
    v = gAudioContext.masterVolume;
    AUDIO_UNLOCK;

    return v;
}

// ==============================================================================
// SetOutputEqualizer
// ==============================================================================
void CAudioModule::SetOutputEqualizer(U8 x)
{
    
    AUDIO_LOCK;
    gAudioContext.outputEqualizerEnabled = x;
    gAudioContext.pAudioMixer->EnableSpeaker(gAudioContext.outputEqualizerEnabled);
    AUDIO_UNLOCK;
}

// ==============================================================================
// GetOutputEqualizer
// ==============================================================================
U8 CAudioModule::GetOutputEqualizer(void)
{
    U8 e;

    AUDIO_LOCK;
    // TODO: This probably should call through to the mixer...
    e = gAudioContext.outputEqualizerEnabled;
    AUDIO_UNLOCK;

    return e;
}

// ==============================================================================
// SetAudioResourcePath
// ==============================================================================
tErrType CAudioModule::SetAudioResourcePath( U32 mpiID, const CPath &path )
{
    CPath newPath = path;

    AUDIO_LOCK;
    if (newPath.length() > 1 && newPath.at(newPath.length()-1) != '/')
        newPath += '/';
    gAudioContext.gpath = newPath;    
    AUDIO_UNLOCK;

    return kNoErr;
}

// ==============================================================================
// GetAudioResourcePath
// ==============================================================================
CPath* CAudioModule::GetAudioResourcePath( U32 mpiID )
{
    CPath *p;

    AUDIO_LOCK;
    p = &gAudioContext.gpath;
    AUDIO_UNLOCK;

    return p;
}

// ==============================================================================
// StartAudio
// ==============================================================================
tAudioID CAudioModule::StartAudio( U32 mpiID,
                                   const CPath& path,
                                   U8 volume,  
                                   tAudioPriority priority,
                                   S8 pan,
                                   const IEventListener *pListener, 
                                   tAudioPayload payload,
                                   tAudioOptionsFlags flags )
{

    tAudioID id = kNoAudioID;
    struct stat fileStat;
    tErrType err;
    tAudioStartAudioInfo Ai;
    int strIndex;
    CPath filename, fileExt;
    
    AUDIO_LOCK;
    // Determine whether specified file exists
    CPath fullPath = (path.length() == 0) ? "" :
        (path.at(0) == '/') ? path : gAudioContext.gpath + path;
    if(stat(fullPath.c_str(), &fileStat) != 0)
    {
        printf("%s: file doesn't exist='%s\n", __FUNCTION__, path.c_str());
        goto error;
    }

    // Extract filename and extension
    strIndex = fullPath.rfind('/', fullPath.size());
    filename = fullPath.substr(strIndex + 1, fullPath.size());
    strIndex = fullPath.rfind('.', fullPath.size());
    fileExt  = fullPath.substr(strIndex + 1, strIndex + 4);
    
    // Generate audio ID
    gAudioContext.nextAudioID++;
    if (kNoAudioID == gAudioContext.nextAudioID) {
        // Quick hack:  see nextAudioID in startup above
        gAudioContext.nextAudioID = (2);
    }
    id = gAudioContext.nextAudioID;
    // Create player based on file extension

    Ai.path = &fullPath;
    Ai.volume = volume;
    Ai.priority = priority;
    Ai.pan = pan;
    Ai.pListener = pListener;
    Ai.payload = payload;
    Ai.flags = flags;
    err = gAudioContext.pAudioMixer->AddPlayer(&Ai, (char *)fileExt.c_str(), id);
    if (kNoErr != err)
    {
        printf("%s: unable to add File='%s'\n", __FUNCTION__, filename.c_str());
        id = kNoAudioID;
        goto error;
    }

 error:
    AUDIO_UNLOCK;

    return id;
}

// ==============================================================================
// StartAudio
// ==============================================================================
tAudioID CAudioModule::StartAudio( U32 mpiID,
                                   const CPath &path,
                                   tAudioPayload payload, 
                                   tAudioOptionsFlags flags)
{

    MPIInstanceState& mpiState = RetrieveMPIState(mpiID);

    return StartAudio(mpiID, path, mpiState.volume, mpiState.priority,
                      mpiState.pan, mpiState.pListener, payload, flags);
}

// ==============================================================================
// GetAudioTime
// ==============================================================================
U32 CAudioModule::GetAudioTime(tAudioID id)
{
    U32 time = 0;
    
    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel)
    {
        CAudioPlayer *pPlayer = pChannel->GetPlayer();
        if (pPlayer)
            time = pPlayer->GetAudioTime_mSec();
    }    
    AUDIO_UNLOCK;

    return time;
}

// ==============================================================================
// IsAudioPlaying
// ==============================================================================
Boolean CAudioModule::IsAudioPlaying(tAudioID id)
{
    Boolean playing = false;
    CChannel *c;

    AUDIO_LOCK;
    c = gAudioContext.pAudioMixer->FindChannel(id);
    if(c != kNull)
        playing = c->IsInUse();
    AUDIO_UNLOCK;
    return playing;
}

// ==============================================================================
// IsAudioPlaying
// ==============================================================================
Boolean CAudioModule::IsAudioPlaying()
{
    Boolean playing;
    
    AUDIO_LOCK;
    playing = gAudioContext.pAudioMixer->IsAnyAudioActive();
    AUDIO_UNLOCK;

    return playing;
}

// ==============================================================================
// StopAudio
// ==============================================================================
void CAudioModule::StopAudio( tAudioID id, Boolean noDoneMessage )
{

    AUDIO_LOCK;
    CChannel *pCh = gAudioContext.pAudioMixer->FindChannel(id);    
    if (pCh && pCh->IsInUse()) {
        //perhaps we should pass noDoneMessage?
        pCh->Release(true);
    }
    AUDIO_UNLOCK;
}

// ==============================================================================
// PauseAudio
// ==============================================================================
void CAudioModule::PauseAudio(tAudioID id)
{

    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel)
        pChannel->Pause();
    AUDIO_UNLOCK;
}

// ==============================================================================
// ResumeAudio
// ==============================================================================
void CAudioModule::ResumeAudio( tAudioID id )
{

    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel && pChannel->IsPaused())
        pChannel->Resume();    
    AUDIO_UNLOCK;
}

// ==============================================================================
// GetAudioState
// ==============================================================================
U8 CAudioModule::GetAudioState( tAudioState *d ) 
{

    AUDIO_LOCK;
    gAudioContext.pAudioMixer->GetAudioState(d);   
    AUDIO_UNLOCK;
    return (66); //Why 66??!!
}

// ==============================================================================
// SetAudioState
// ==============================================================================
void CAudioModule::SetAudioState( tAudioState *d ) 
{
    AUDIO_LOCK;
    gAudioContext.pAudioMixer->SetAudioState(d);
    AUDIO_UNLOCK;
}

// ==============================================================================
// GetAudioVolume
// ==============================================================================
U8 CAudioModule::GetAudioVolume( tAudioID id )
{
    U8 volume = 0;
 
    AUDIO_LOCK;
	if (id == BRIO_MIDI_PLAYER_ID && gAudioContext.pMidiPlayer &&
        gAudioContext.pMidiPlayer->IsFileActive() ) {
        volume = gAudioContext.pMidiPlayer->GetVolume();
    }
    else
    {
        CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
        if (pChannel)
            volume = pChannel->GetVolume();
    }
    AUDIO_UNLOCK;
        
    return volume;
}

// ==============================================================================
// SetAudioVolume
// ==============================================================================
void CAudioModule::SetAudioVolume( tAudioID id, U8 x ) 
{
    AUDIO_LOCK;
	if (id == BRIO_MIDI_PLAYER_ID && gAudioContext.pMidiPlayer &&
        gAudioContext.pMidiPlayer->IsFileActive() ) {
        gAudioContext.pMidiPlayer->SetVolume(x);
    } else {
        CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
        if (pChannel) 
            pChannel->SetVolume(x);
    }
    AUDIO_UNLOCK;
}

// ==============================================================================
// GetAudioPriority
// ==============================================================================
tAudioPriority CAudioModule::GetAudioPriority( tAudioID id ) 
{
    tAudioPriority priority = 0;

    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel) 
        priority = pChannel->GetPriority();
    AUDIO_UNLOCK;

    return priority;
}

// ==============================================================================
// SetAudioPriority
// ==============================================================================
void CAudioModule::SetAudioPriority( tAudioID id, tAudioPriority priority ) 
{
    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel) 
        pChannel->SetPriority(priority);
    AUDIO_UNLOCK;
}

// ==============================================================================
// GetAudioPan
// ==============================================================================
S8 CAudioModule::GetAudioPan( tAudioID id ) 
{
    S8 pan = 0;

    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel) 
        pan = pChannel->GetPan();
    AUDIO_UNLOCK;

    return pan;
}

// ==============================================================================
// SetAudioPan
// ==============================================================================
void CAudioModule::SetAudioPan( tAudioID id, S8 x ) 
{
    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel) 
        pChannel->SetPan(x);
    AUDIO_UNLOCK;
}   // ---- end SetAudioPan() ----

// ==============================================================================
// GetAudioEventListener
// ==============================================================================
const IEventListener* CAudioModule::GetAudioEventListener( tAudioID id ) 
{
    const IEventListener*       pListener = NULL;

    AUDIO_LOCK;
    CChannel*pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if (pChannel)
    {
        CAudioPlayer *pPlayer = pChannel->GetPlayer();
        if (pPlayer)
            pListener = pPlayer->GetEventListener();
    }    
    AUDIO_UNLOCK;

    return pListener;
}

// ==============================================================================
// SetAudioEventListener
// ==============================================================================
void CAudioModule::SetAudioEventListener( tAudioID id, const IEventListener *pListener) 
{

    AUDIO_LOCK;
    CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel(id);
    if(pChannel)
    {
        CAudioPlayer *pPlayer = pChannel->GetPlayer();
        if(pPlayer)
            pPlayer->SetEventListener(pListener);
    }
    AUDIO_UNLOCK;
}

// ==============================================================================
// GetDefaultAudioVolume
// ==============================================================================
U8 CAudioModule::GetDefaultAudioVolume( U32 mpiID ) 
{
    MPIInstanceState& mpiState = RetrieveMPIState(mpiID);
    return mpiState.volume;
}

// ==============================================================================
// SetDefaultAudioVolume
// ==============================================================================
void CAudioModule::SetDefaultAudioVolume( U32 mpiID, U8 x ) 
{
    MPIInstanceState& mpiState = RetrieveMPIState(mpiID);
    mpiState.volume = x;
}

// ==============================================================================
// GetDefaultAudioPriority
// ==============================================================================
tAudioPriority CAudioModule::GetDefaultAudioPriority(U32 mpiID) 
{
    MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
    return mpiState.priority;
}

// ==============================================================================
// SetDefaultAudioPriority
// ==============================================================================
void CAudioModule::SetDefaultAudioPriority( U32 mpiID, tAudioPriority priority ) 
{
    MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
    mpiState.pan = priority;
}

// ==============================================================================
// GetDefaultAudioPan
// ==============================================================================
S8 CAudioModule::GetDefaultAudioPan( U32 mpiID ) 
{
    MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
    return mpiState.pan;
}

// ==============================================================================
// SetDefaultAudioPan
// ==============================================================================
void CAudioModule::SetDefaultAudioPan( U32 mpiID, S8 pan ) 
{
    MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
    mpiState.pan = pan;
}

// ==============================================================================
// GetDefaultAudioEventListener
// ==============================================================================
const IEventListener* CAudioModule::GetDefaultAudioEventListener( U32 mpiID ) 
{
    MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
    return mpiState.pListener;
}

// ==============================================================================
// SetDefaultAudioEventListener
// ==============================================================================
void CAudioModule::SetDefaultAudioEventListener( U32 mpiID,
                                                 const IEventListener *pListener ) 
{
    MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
    mpiState.pListener = pListener;
}

// ==============================================================================
// AcquireMidiPlayer
// ==============================================================================
tErrType CAudioModule::AcquireMidiPlayer( tAudioPriority /* priority */,
                                          IEventListener * /* pHandler */,
                                          tMidiPlayerID *id )
{

    AUDIO_LOCK;
    if (!gAudioContext.pMidiPlayer)
        gAudioContext.pMidiPlayer =
            gAudioContext.pAudioMixer->CreateMIDIPlayer();
    *id = kNoMidiID;
    if (gAudioContext.pMidiPlayer) {
        gAudioContext.pMidiPlayer->Activate();
        *id = gAudioContext.pMidiPlayer->GetID();
    }
    AUDIO_UNLOCK;
    
    return kNoErr;
}

// ==============================================================================
// ReleaseMidiPlayer
// ==============================================================================
tErrType CAudioModule::ReleaseMidiPlayer( tMidiPlayerID /* id */)
{
    // GK FIXX:  MIDI id not used for now as there is only one MIDI player
    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
    {
        gAudioContext.pMidiPlayer->DeActivate();
        gAudioContext.pAudioMixer->DestroyMIDIPlayer();
        gAudioContext.pMidiPlayer = NULL;
    }
    AUDIO_UNLOCK;

    return kNoErr;
}   // ---- end ReleaseMidiPlayer() ----

// ==============================================================================
// GetAudioIDForMidiID
// ==============================================================================
tAudioID CAudioModule::GetAudioIDForMidiID( tMidiPlayerID /* id */) 
{
    return (2);  // GK FIXXXX: quick hack.  Fixed value
}

// ==============================================================================
// MidiNoteOn
// ==============================================================================
tErrType CAudioModule::MidiNoteOn( tMidiPlayerID /* id */,
                                   U8 channel,
                                   U8 note,
                                   U8 velocity,
                                   tAudioOptionsFlags flags )
{
    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        gAudioContext.pMidiPlayer->NoteOn(channel, note, velocity, flags);
    AUDIO_UNLOCK;
    
    return kNoErr;
}
    
// ==============================================================================
// MidiNoteOff
// ==============================================================================
tErrType CAudioModule::MidiNoteOff( tMidiPlayerID /* id */,
                                    U8 channel,
                                    U8 note,
                                    U8 velocity,
                                    tAudioOptionsFlags flags )
{

    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
       gAudioContext.pMidiPlayer->NoteOff(channel, note, velocity, flags);
    AUDIO_UNLOCK;
    return kNoErr;
}

// ==============================================================================
// SendMidiCommand
// ==============================================================================
tErrType CAudioModule::SendMidiCommand( tMidiPlayerID /* id */,
                                        U8 cmd, 
                                        U8 data1,
                                        U8 data2 )
{
    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        gAudioContext.pMidiPlayer->SendCommand(cmd, data1, data2);
    AUDIO_UNLOCK;
    return kNoErr;
}

// ==============================================================================
// StartMidiFile
// ==============================================================================
tErrType CAudioModule::StartMidiFile(   U32                 mpiID, 
                                        tMidiPlayerID       id,
                                        const CPath         &path, 
                                        U8                  volume, 
                                        tAudioPriority      priority,
                                        IEventListener*     pListener,
                                        tAudioPayload       payload,
                                        tAudioOptionsFlags  flags )
{

    AUDIO_LOCK;
    tAudioStartMidiFileInfo info;
    const CPath fullPath = (path.length() == 0) ? "" : (path.at(0) == '/')
        ? path : gAudioContext.gpath + path;

    // Generate command message to send to audio Mgr task
    info.id        = id;
    info.path      = &fullPath;
    info.volume    = volume;
    info.priority  = priority;
    info.pListener = pListener;
    info.priority  = priority;
    info.payload   = payload;
    info.flags     = flags;
    
    // Determine size of MIDI file
    struct stat fileStat;
    int err = stat(fullPath.c_str(), &fileStat);
    pDebugMPI_->Assert((kNoErr == err), 
                       "%s: file doesn't exist '%s' \n",
                       __FUNCTION__, fullPath.c_str());
    
    // Allocate buffer for file image
    // BC: This memory is never freed!  This malloc should be moved inside the
    // midi player.
    info.pMidiFileImage = (U8 *)pKernelMPI_->Malloc(fileStat.st_size);
    pDebugMPI_->Assert((info.pMidiFileImage != 0),
                       "%s: failed to allocate midi file buffer\n",
                       __FUNCTION__);
    info.imageSize = fileStat.st_size;
    
    // Load image
    FILE *file = fopen(fullPath.c_str(), "r" );
    int bytesRead = fread(info.pMidiFileImage, sizeof(char), fileStat.st_size, file);
    pDebugMPI_->Assert((bytesRead == fileStat.st_size),
                       "%s: failed to read midi file\n",
                       __FUNCTION__);
    fclose(file);
    
    tErrType result = kAudioMidiErr;
    if(gAudioContext.pMidiPlayer)
    {
        result = gAudioContext.pMidiPlayer->StartMidiFile(&info);
        pDebugMPI_->Assert((kNoErr == result), 
                           "%s: Failed to start MIDI file. result=%d \n",
                           __FUNCTION__, (int)result);
    }

    AUDIO_UNLOCK;
    return result;

}

// ==============================================================================
// StartMidiFile
// ==============================================================================
tErrType CAudioModule::StartMidiFile(   U32                 mpiID, 
                                        tMidiPlayerID       id,
                                        const CPath         &path, 
                                        tAudioPayload       payload,
                                        tAudioOptionsFlags  flags )
{
    MPIInstanceState& mpiState = RetrieveMPIState(mpiID);
    //BC: I cast the pListener to not be const.  The alternative would be to
    //change the StartMidiFile to accept a const, but this is an API change, and
    //that's not allowed.
    return StartMidiFile(mpiID, id, path, mpiState.volume, mpiState.priority,
                         (IEventListener *)mpiState.pListener, payload, flags);
}

// ==============================================================================
// IsMidiFilePlaying
// ==============================================================================
Boolean CAudioModule::IsMidiFilePlaying( tMidiPlayerID id )
{
    Boolean playing = false;

    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        playing = gAudioContext.pMidiPlayer->IsFileActive();
    AUDIO_UNLOCK;

    return playing;
}

// ==============================================================================
// IsMidiFilePlaying
// ==============================================================================
Boolean CAudioModule::IsMidiFilePlaying( void )
{
    return IsMidiFilePlaying(BRIO_MIDI_PLAYER_ID); //id should come from mixer?
}

// ==============================================================================
// PauseMidiFile
// ==============================================================================
void CAudioModule::PauseMidiFile( tMidiPlayerID /* id */ )
{
    
    AUDIO_LOCK;
    if(gAudioContext.pMidiPlayer)
        gAudioContext.pMidiPlayer->PauseMidiFile();
    AUDIO_UNLOCK;

}

//==============================================================================
// ResumeMidiFile
//==============================================================================
void CAudioModule::ResumeMidiFile( tMidiPlayerID /* id */)
{
    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        gAudioContext.pMidiPlayer->ResumeMidiFile();
    AUDIO_UNLOCK;
    
}

// ==============================================================================
// StopMidiFile
// ==============================================================================
void CAudioModule::StopMidiFile( tMidiPlayerID id, Boolean noDoneMessage )
{
    tAudioStopMidiFileInfo info;
    info.id = id;
    info.noDoneMsg = noDoneMessage;
    
    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        gAudioContext.pMidiPlayer->StopMidiFile(&info);
    AUDIO_UNLOCK;
    
}

// ==============================================================================
// GetEnabledMidiTracks
// ==============================================================================
tMidiTrackBitMask CAudioModule::GetEnabledMidiTracks( tMidiPlayerID /* id */ )
{

    tMidiTrackBitMask       trackBitMask = 0;

    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        gAudioContext.pMidiPlayer->GetEnableTracks(&trackBitMask);
    AUDIO_UNLOCK;

    return trackBitMask;
}

// ==============================================================================
// SetEnableMidiTracks
// ==============================================================================
tErrType CAudioModule::SetEnableMidiTracks(tMidiPlayerID /* id */,
                                           tMidiTrackBitMask trackBitMask)
{
    tErrType result = kAudioMidiUnavailable;

    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        result = gAudioContext.pMidiPlayer->SetEnableTracks(trackBitMask);
    AUDIO_UNLOCK;

    return result;
}

// ==============================================================================
// TransposeMidiTracks
// ==============================================================================
tErrType CAudioModule::TransposeMidiTracks(tMidiPlayerID /* id */,
                                           tMidiTrackBitMask trackBitMask,
                                           S8 transposeAmount)
{
    tErrType result = kAudioMidiUnavailable;

    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        result = gAudioContext.pMidiPlayer->TransposeTracks(trackBitMask,
                                                            transposeAmount);
    AUDIO_UNLOCK;

    return result;
}

// ==============================================================================
// ChangeMidiInstrument
// ==============================================================================
tErrType CAudioModule::ChangeMidiInstrument(tMidiPlayerID /* id */,
                                            tMidiTrackBitMask trackBitMask,
                                            tMidiPlayerInstrument instr)
{
    tErrType result = kAudioMidiUnavailable;

    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        result = gAudioContext.pMidiPlayer->ChangeProgram(trackBitMask, instr);
    AUDIO_UNLOCK;

    return result;
}

// ==============================================================================
// ChangeMidiTempo
// ==============================================================================
tErrType CAudioModule::ChangeMidiTempo(tMidiPlayerID /* id */, S8 tempo)
{
    tErrType result = kAudioMidiUnavailable;

    AUDIO_LOCK;
    if (gAudioContext.pMidiPlayer)
        result = gAudioContext.pMidiPlayer->ChangeTempo(tempo);
    AUDIO_UNLOCK;

    return result;
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()
extern "C"
{
    //------------------------------------------------------------------------
    ICoreModule* CreateInstance(tVersion /*version*/)
    {
        if( sinst == NULL )
            sinst = new CAudioModule;
        return sinst;
    }

    //------------------------------------------------------------------------
    void DestroyInstance(ICoreModule* /*ptr*/)
    {
//      assert(ptr == sinst);
        delete sinst;
        sinst = NULL;
    }
}
#endif  // LF_MONOLITHIC_DEBUG

// EOF

