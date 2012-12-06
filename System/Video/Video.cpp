//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Video.cpp
//
// Description:
//		Video module implementation.
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <VideoTypes.h>
#include <VideoPriv.h>
#include <AudioMPI.h>
#include <KernelMPI.h>
#include <DisplayMPI.h>
#include <errno.h>
#include <string.h>
#include <list>

#ifndef EMULATION
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

//#define ENABLE_PROFILING
#include <FlatProfiler.h>

#include <VideoPlayer.h>
#include <TheoraPlayer.h>
#include <AVIPlayer.h>
#include <SearchTree.h>

LF_BEGIN_BRIO_NAMESPACE()
//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Video";


//============================================================================
// CVideoModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CVideoModule::GetModuleVersion() const
{
	return kVideoModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CVideoModule::GetModuleName() const
{
	return &kVideoModuleName;
}

//----------------------------------------------------------------------------
const CURI* CVideoModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

//============================================================================
// Local variables
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	// Video MPI global vars
//	tVideoContext*		gpVidCtx = NULL;
	CPath				gpath = "";
	CPath				gfilepath = "";
//	FILE*				gfile = NULL;
	tMutex				gVidMutex;
//	Boolean				gbCodecReady = false;
//	Boolean				gbCentered = false;
	tVideoHndl			ghVideoHndl = kInvalidVideoHndl;
	std::list<tVideoHndl>	ghVideoList;	// list of video handles
}

//============================================================================
// C function support
//============================================================================

//----------------------------------------------------------------------------
// Set video scaler (embedded only)
static void SetScaler(int width, int height, bool centered)
{
	CDisplayMPI 	dispmgr;
	tDisplayHandle 	hvideo = dispmgr.GetCurrentDisplayHandle(kPixelFormatYUV420);
	
	dispmgr.SetVideoScaler(hvideo, width, height, centered);
}

//============================================================================
// Ctor & dtor
//============================================================================
CVideoModule::CVideoModule() : dbg_(kGroupVideo)
{
#if USE_MUTEX
	tMutexAttr attr;
	kernel_.InitMutexAttributeObject(attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK); 
	kernel_.InitMutex(gVidMutex, attr);
#endif
	dbg_.SetDebugLevel(kVideoDebugLevel);
	FlatProfilerInit(2, FLATPROF_NUM_TIMESTAMPS);
}

//----------------------------------------------------------------------------
CVideoModule::~CVideoModule()
{
	FlatProfilerDone();

	if (ghVideoHndl != kInvalidVideoHndl)
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::~VideoModule: video %p open\n", (void*)ghVideoHndl);
		StopVideo(ghVideoHndl);
	}

#if USE_MUTEX
	kernel_.DeInitMutex(gVidMutex);
#endif
}

//----------------------------------------------------------------------------
Boolean	CVideoModule::IsValid() const
{
	return true;
}

//============================================================================
// Video-specific Implementation
//============================================================================

//----------------------------------------------------------------------------
tVideoHndl CVideoModule::GetCurrentVideoHandle()
{
	return ghVideoHndl;
}

//----------------------------------------------------------------------------
tErrType CVideoModule::SetVideoResourcePath(const CPath &path)
{
	gpath = path;
	if (gpath.length() > 1 && gpath.at(gpath.length()-1) != '/')
		gpath += '/';
	return kNoErr;
}

//----------------------------------------------------------------------------
CPath* CVideoModule::GetVideoResourcePath() const
{
	return &gpath;
}

//----------------------------------------------------------------------------
tVideoHndl CVideoModule::StartVideo(const CPath& path, tVideoSurf* pSurf, Boolean bLoop, IEventListener* pListener)
{
	CPath pathAudio = "";
	return StartVideo(path, pathAudio, pSurf, bLoop, pListener);
}

//----------------------------------------------------------------------------
tVideoHndl CVideoModule::StartVideo(const CPath& path, const CPath& pathAudio, tVideoSurf* pSurf, Boolean bLoop, IEventListener* pListener)
{
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	tVideoHndl		hVideo = kInvalidVideoHndl;
	tVideoContext*	pVidCtx = NULL;
	bool			nopath = false;

	// We only support one video context active at a time
	if (ghVideoHndl != kInvalidVideoHndl)
	{
		pVidCtx = reinterpret_cast<tVideoContext*>(ghVideoHndl);

		if(pVidCtx->bPlaying)
		{
			goto ExitPt;
		}
		// Else it's a dead thread, StartVideoInt will reset the handle
	}


	// Start Theora codec for selected video file
	hVideo = StartVideoInt(path);
	if (hVideo == kInvalidVideoHndl)
		goto ExitPt;

	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	
#ifndef EMULATION
	// Set HW video scaler for video source width and height
	if(pSurf->format == kPixelFormatYUV420 || pSurf->format == kPixelFormatYUYV422)
		SetScaler(pVidCtx->info.width, pVidCtx->info.height, pVidCtx->bCentered);
#endif

	// Determine if audio track is available and at what path?
	nopath = (pathAudio.length() == 0) ? true : false;
	gfilepath = (nopath) ? "" : (pathAudio.at(0) == '/') ? pathAudio : gpath + pathAudio;

	pVidCtx->hVideo 	= hVideo;
	pVidCtx->hAudio 	= kNoAudioID; // handled inside video task
	pVidCtx->pPathAudio = (nopath) ? NULL : &gfilepath; // pass by pointer
	pVidCtx->pSurfVideo = pSurf;
	pVidCtx->pListener 	= pListener;
	pVidCtx->bLooped 	= bLoop;
	pVidCtx->bDropFramed	= false;
	pVidCtx->bPaused	= false;
	pVidCtx->bPlaying	= true;
//	pVidCtx->uFrameTime = 1000 / pVidCtx->info.fps; // handled inside init
	pVidCtx->pMutex		= &gVidMutex;
	pVidCtx->bSeeked	= false;
	pVidCtx->pModule 	= this;

	InitVideoTask(pVidCtx);	

ExitPt:
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return hVideo;
}

//----------------------------------------------------------------------------
tVideoHndl CVideoModule::StartVideo(const CPath& path)
{
	return StartVideoInt(path);
}

//----------------------------------------------------------------------------
tVideoHndl CVideoModule::StartVideoInt(const CPath& path)
{
	tVideoHndl	hVideo = kInvalidVideoHndl;
	Boolean		b;
	CPath		filepath = (path.at(0) == '/') ? path : gpath + path;
	const char*	filename = filepath.c_str();
	
	// Open Ogg file associated with resource
	FILE* file = fopen(filename, "r");
	if (file == NULL) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: LoadRsrc failed for %s\n", filename);
		return kInvalidVideoHndl;
	}

	// Create video context for tracking as video handle 
	tVideoContext* pVidCtx = static_cast<tVideoContext*>(kernel_.Malloc(sizeof(tVideoContext)));
	memset(pVidCtx, 0, sizeof(tVideoContext));
	hVideo = reinterpret_cast<tVideoHndl>(pVidCtx);
	pVidCtx->pFileVideo = file;
	pVidCtx->pPathVideo = new CPath(filepath);

	if (filepath.rfind(".ogg") != std::string::npos 
			|| filepath.rfind(".OGG") != std::string::npos) {
		// Create Theora video player object
		CTheoraPlayer* 	pPlayer = new CTheoraPlayer();
		pVidCtx->pPlayer = pPlayer;
	}
	else if (filepath.rfind(".avi") != std::string::npos
			|| filepath.rfind(".AVI") != std::string::npos
			|| filepath.rfind(".mp4") != std::string::npos
			|| filepath.rfind(".MP4") != std::string::npos) {
		// Create AVI video player object
		CAVIPlayer* 	pPlayer = new CAVIPlayer();
		pVidCtx->pPlayer = pPlayer;
	}
	
	// Init Theora video codec for Ogg file
	b = InitVideoInt(hVideo);
	if (!b)
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: InitVideoInt failed to init codec for %s\n", filename);
		delete pVidCtx->pPlayer;
		kernel_.Free(pVidCtx);
		fclose(file);
		return kInvalidVideoHndl;
	}

	// Track video handle	
	ghVideoList.push_back(hVideo);
	return ghVideoHndl = hVideo;
}	
	
//----------------------------------------------------------------------------
Boolean CVideoModule::InitVideoInt(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	if (!pVidCtx || !pVidCtx->pPlayer)
		return false;

	return pVidCtx->pPlayer->InitVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	if (!pVidCtx || !pVidCtx->pPlayer)
		return false;

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean	state = pVidCtx->pPlayer->GetVideoInfo(hVideo, pInfo);
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif

	return state;
}

//----------------------------------------------------------------------------
void CVideoModule::DeInitVideoInt(tVideoHndl hVideo)
{	
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	if (!pVidCtx || !pVidCtx->pPlayer)
		return;

	pVidCtx->pPlayer->DeInitVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVideoModule::StopVideo(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	if (!pVidCtx)
		return false;
	
	// Active video handle?
	std::list<tVideoHndl>::iterator it;
	for (it = ghVideoList.begin(); it != ghVideoList.end(); it++)
		if (*it == hVideo)
			break;
	// If not in list, it's been freed already
	if (it == ghVideoList.end())
		return false;

	// Avoid subsequent locks while stopping
	pVidCtx->bCodecReady = false;

#if USE_MUTEX
	do { 
		kernel_.TaskSleep(10);
	} while (kernel_.TryLockMutex(gVidMutex) == EBUSY);
#endif

	// Kill video task, if running
	if (pVidCtx->hVideoThread != kNull)
		DeInitVideoTask(pVidCtx);
	
	// Cleanup decoder stream resources
	DeInitVideoInt(hVideo);
	
	// Destroy video player object
	if (pVidCtx->pPlayer)
	{
		delete pVidCtx->pPlayer;
		pVidCtx->pPlayer = NULL;
	}

	// Close file associated with resource
	if (pVidCtx->pFileVideo != NULL)
	{
		fclose(pVidCtx->pFileVideo);
		pVidCtx->pFileVideo = NULL;
	}
	if (pVidCtx->pPathVideo != NULL)
	{
		delete pVidCtx->pPathVideo;
		pVidCtx->pPathVideo = NULL;
	}
	
	// Free video context created for task thread, if any
	if (pVidCtx) 
	{
		kernel_.Free(pVidCtx);
		pVidCtx = NULL;
	}

	// Remove from active video handle list
	ghVideoList.erase(it);
	
	ghVideoHndl = kInvalidVideoHndl;
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif

	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	// Internal codec state only changes during start/stop
	if (!pVidCtx || !pVidCtx->bCodecReady)
		return false;

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean status = pVidCtx->pPlayer->GetVideoTime(hVideo, pTime);
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return status;	
}

//----------------------------------------------------------------------------
Boolean CVideoModule::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	Boolean		ready = false;
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	
	// Internal codec state only changes during start/stop
	if (!pVidCtx || !pVidCtx->bCodecReady)
		return false;

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	ready = pVidCtx->pPlayer->SyncVideoFrame(hVideo, pCtx, bDrop);
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return ready;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay)
{
	Boolean		found = false;
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	
	if (!pVidCtx || !pVidCtx->bCodecReady)
		return false;

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	found = pVidCtx->pPlayer->SeekVideoFrame(hVideo, pCtx, bExact, bUpdateVideoDisplay);
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return found;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	// Internal codec state only changes during start/stop
	if (!pVidCtx || !pVidCtx->bCodecReady)
		return false;

	return pVidCtx->pPlayer->GetVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVideoModule::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	Boolean status = true;
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	
	// Internal codec state only changes during start/stop
	if (!pVidCtx || !pVidCtx->bCodecReady)
		return false;

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif
	
	status = pVidCtx->pPlayer->PutVideoFrame(hVideo, pCtx);

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	
	return status;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::PauseVideo(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (pVidCtx) ? pVidCtx->bPaused = true : false;
		
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::ResumeVideo(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (pVidCtx) ? pVidCtx->bPaused = false : false;

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoPaused(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (pVidCtx) ? pVidCtx->bPaused : false;

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoPlaying(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (pVidCtx) ? pVidCtx->bPlaying : false;

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
S64 CVideoModule::GetVideoLength(const CPath& path, int maxLength)
{
	tVideoHndl hVideo = StartVideo(path);
	if(hVideo == kInvalidVideoHndl)
		return -1;

	//Populate our search tree with all seconds possible from 0 to maxLength.
	NumNode *root = NULL;
	for(int i=0; i < maxLength; i++)
		InsertValue(&root,i);
	BalanceTree(root);	//Balance the tree, so that we can find the length as quickly as possible.

	//Get our video info, so that we can get the fps.
	tVideoInfo info;
	GetVideoInfo(hVideo,&info);
	tVideoTime time;
	S64 prevTime = 0;
	time.time = 0;

	//Cycle through our tree to find the length of our video.
	NumNode *node = root;
	while(node != NULL) {
		int i = node->num;	//i is our 'time'.
		time.frame = i*info.fps;	//The number of frames at i is i*fps.
		SeekVideoFrame(hVideo,&time,true,false);	//Seek to that frame and get the time returned.
		GetVideoTime(hVideo,&time);
		printf("  [%d] time= %lld\n",i,time.time);
		if(time.time < 0)	//If the time is < 0, then that means this time is too large.  Go to a smaller time and try that.
			node = node->left;
		else if(time.time == prevTime)	//If this time is == to the last time, then time may be too large.  Check anyone smaller than this node.
			node = node->left;			//If there's nothing smaller, then we've reached our maxTime.
		else
			node = node->right;	//Otherwise, try a bigger node to see if there's a more valid time.
		prevTime = time.time;
	}
	StopVideo(hVideo);
	DeleteTree(root);	//Remove the tree.
	return prevTime;
}

S64 CVideoModule::GetVideoLength(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	S64 length = pVidCtx->pPlayer->GetVideoLength(hVideo);

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return length;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoLooped(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (pVidCtx) ? pVidCtx->bLooped : false;

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif

	return state;
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CVideoModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version;	/* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CVideoModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
		(void )ptr;	/* Prevent unused variable warnings. */
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG


// EOF
