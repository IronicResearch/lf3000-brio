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

//#define _GNU_SOURCE
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <ogg/ogg.h>
#include <theora/theora.h>

#define USE_PROFILE			0

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
	// Theora global state vars
	ogg_sync_state   	oy;
	ogg_page         	og;
	ogg_stream_state 	vo;
	ogg_stream_state 	to;
	theora_info      	ti;
	theora_comment   	tc;
	theora_state     	td;

	// Video MPI global vars
	tVideoContext*		gpVidCtx = NULL;
	CPath				gpath = "";
	FILE*				gfile = NULL;
	tMutex				gVidMutex ; // = PTHREAD_MUTEX_INITIALIZER;
	
#if USE_PROFILE
	// Profile vars
	U32					usecStart;
	U32					usecEnd;
	U32					usecDiff;
#endif
}

//============================================================================
// C function support
//============================================================================

//----------------------------------------------------------------------------
inline void PROFILE_BEGIN(void)
{
#if USE_PROFILE
	CKernelMPI	kernel;
	kernel.GetHRTAsUsec(usecStart);
#endif
}

//----------------------------------------------------------------------------
inline void PROFILE_END(const char* msg)
{
	(void )msg;	/* Prevent unused variable warnings. */
#if USE_PROFILE
	CKernelMPI	kernel;
	CDebugMPI	dbg(kGroupVideo);
	kernel.GetHRTAsUsec(usecEnd);
	usecDiff = usecEnd - usecStart;
	dbg.DebugOut(kDbgLvlVerbose, "%s: lapse time = %u\n", msg, static_cast<unsigned int>(usecDiff));
#endif
}

//----------------------------------------------------------------------------
// Grab some more compressed bitstream and sync it for page extraction
static int buffer_data(/* FILE *in, */ ogg_sync_state *oy)
{
	char 	*buffer = ogg_sync_buffer(oy,4096);
	U32 	bytes = fread(buffer,1,4096,gfile);
	ogg_sync_wrote(oy,bytes);
	return bytes;
}

//----------------------------------------------------------------------------
// Push a page into the steam for packetization
static int queue_page(ogg_page *page)
{
	(void )page;	/* Prevent unused variable warnings. */
	ogg_stream_pagein(&to,&og);
	return 0;
}

//============================================================================
// Ctor & dtor
//============================================================================
CVideoModule::CVideoModule() : dbg_(kGroupVideo)
{
	dbg_.SetDebugLevel(kVideoDebugLevel);
}

//----------------------------------------------------------------------------
CVideoModule::~CVideoModule()
{
}

//----------------------------------------------------------------------------
Boolean	CVideoModule::IsValid() const
{
	return true;
}

//============================================================================
// Video-specific Implementation
//============================================================================

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
	tVideoHndl		hVideo = StartVideoInt(path);
	if (hVideo == kInvalidVideoHndl)
		return kInvalidVideoHndl;
	
	CKernelMPI		kernel;
	tVideoContext*	pVidCtx = static_cast<tVideoContext*>(kernel.Malloc(sizeof(tVideoContext)));
//	memset(pVidCtx, 0, sizeof(tVideoContext));
	
	// Determine if audio track is available and at what path?
	bool			nopath = (pathAudio.length() == 0) ? true : false;
	const CPath		filepath = (nopath) ? "" : (pathAudio.at(0) == '/') ? pathAudio : gpath + pathAudio;

#if USE_MUTEX
	// Init mutex
	const tMutexAttr	attr = {0};
	kernel.InitMutex(gVidMutex, attr);
#endif
	
	pVidCtx->hVideo 	= hVideo;
	pVidCtx->hAudio 	= kNoAudioID; // handled inside video task
	pVidCtx->pPathAudio = (nopath) ? NULL : &filepath; // pass by pointer
	pVidCtx->pSurfVideo = pSurf;
	pVidCtx->pListener 	= pListener;
	pVidCtx->bLooped 	= bLoop;
	pVidCtx->bDropFramed	= false;
	pVidCtx->bPaused	= false;
	pVidCtx->bPlaying	= true;
	pVidCtx->uFrameTime = 1000 * ti.fps_denominator / ti.fps_numerator;
	pVidCtx->pMutex		= &gVidMutex;

	InitVideoTask(pVidCtx);	

	// TODO: Wrap pVidCtx into handle...
	gpVidCtx = pVidCtx;
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
//	tErrType	r;
	Boolean		b;
	CPath		filepath = (path.at(0) == '/') ? path : gpath + path;
	const char*	filename = filepath.c_str();
	
	// Open Ogg file associated with resource
	gfile = fopen(filename, "r");
	if (gfile == NULL) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: LoadRsrc failed for %s\n", filename);
		return kInvalidVideoHndl;
	}
	hVideo = reinterpret_cast<tVideoHndl>(gfile); // ???
	
	// Init Theora video codec for Ogg file
	b = InitVideoInt(hVideo);
	if (!b)
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: InitVideoInt failed to init codec for %s\n", filename);
		fclose(gfile);
		gfile = NULL;
		return kInvalidVideoHndl;
	}
	
	return hVideo;
}	
	
//----------------------------------------------------------------------------
Boolean CVideoModule::InitVideoInt(tVideoHndl hVideo)
{	
	(void )hVideo;	/* Prevent unused variable warnings. */
	// Start up Ogg stream synchronization layer
	ogg_sync_init(&oy);
	
	// Init supporting Theora structures needed in header parsing
	theora_comment_init(&tc);
	theora_info_init(&ti);

	// Vorbis and Theora both depend on some initial header packets
	// for decoder setup and initialization. We retrieve these first
	// before entering the main decode loop.
	ogg_packet op;
	int stateflag = 0;
	int theorapkts = 0;

	// Only interested in Theora streams
	while (!stateflag)
	{
		int ret = buffer_data(&oy);
		if (ret==0)
			break;
		while (ogg_sync_pageout(&oy,&og)>0)
		{
			ogg_stream_state test;
			// is this a mandated initial header? If not, stop parsing
			if (!ogg_page_bos(&og))
			{
				// don't leak the page; get it into the appropriate stream
    			if (theorapkts)
    				queue_page(&og);
    			stateflag = 1;
				break;
  			}
			ogg_stream_init(&test,ogg_page_serialno(&og));
			ogg_stream_pagein(&test,&og);
			ogg_stream_packetout(&test,&op);
			// identify the codec header as theora
			if (!theorapkts && theora_decode_header(&ti,&tc,&op) >= 0)
			{
 				// it is theora -- save this stream state
				memcpy(&to,&test,sizeof(test));
    			theorapkts = 1;
			}
			else
			{
				// it is vorbis, but we can't use vorbisfile interface
				memcpy(&vo,&test,sizeof(test));
				// whatever it is, we don't care about it
    			ogg_stream_clear(&test);
			}
		}
		// fall through to non-initial page parsing
	}

	// We're expecting more header packets
	while (theorapkts && theorapkts<3)
	{
		int ret;
		// look for further theora headers
		while (theorapkts && (theorapkts<3) && (ret = ogg_stream_packetout(&to,&op)))
		{
			if ((ret<0) || (theora_decode_header(&ti,&tc,&op)))
			{
				dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: Error parsing Theora stream headers\n");
				return false;
			}
			theorapkts++;
			if (theorapkts == 3)
				break;
		}
		// The header pages/packets will arrive before anything else we
		// care about, or the stream is not obeying spec
		if (ogg_sync_pageout(&oy,&og)>0)
		{
			// demux into the stream state
			queue_page(&og); 
		}
		else
		{
			// need more data
			ret = buffer_data(&oy); 
			if (ret==0)
			{
				dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: End of file while searching for codec headers\n");
				return false;
			}
		}
	}

	// Theora packets should have been detected by now
	if (!theorapkts)
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: Unable to find Theora stream headers\n");
		return false;
	}

	// Start decoder stream
	theora_decode_init(&td,&ti);
	dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::StartVideo: %dx%d, %d:%d aspect, %d:%d fps\n",
		ti.width, ti.height, ti.aspect_numerator, ti.aspect_denominator, ti.fps_numerator, ti.fps_denominator);
	dbg_.Assert(ti.fps_numerator != 0, "VideoModule::StartVideo: bad fps numerator\n");
	dbg_.Assert(ti.fps_denominator != 0, "VideoModule::StartVideo: bad fps denominator\n");

	// Queue remaining pages that did not contain headers
	while (ogg_sync_pageout(&oy,&og) > 0)
		queue_page(&og);

	theora_decode_packetin(&td,&op);
	theora_granule_frame(&td,td.granulepos);
	
	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	pInfo->width	= ti.width;
	pInfo->height	= ti.height;
	pInfo->fps		= ti.fps_numerator / ti.fps_denominator;
	return true;
}

//----------------------------------------------------------------------------
void CVideoModule::DeInitVideoInt(tVideoHndl hVideo)
{	
	(void )hVideo;	/* Prevent unused variable warnings. */
	// Cleanup decoder stream resources
	ogg_stream_clear(&to);
	theora_clear(&td);
	theora_comment_clear(&tc);
	theora_info_clear(&ti);
	ogg_sync_clear(&oy);
}

//----------------------------------------------------------------------------
Boolean CVideoModule::StopVideo(tVideoHndl hVideo)
{
	// Kill video task, if running
	if (gpVidCtx)
		DeInitVideoTask(gpVidCtx);
	
	// Cleanup decoder stream resources
	DeInitVideoInt(hVideo);
	
	// Close file associated with resource
	if (gfile != NULL)
	{
		fclose(gfile);
		gfile = NULL;
	}

	// Free video context created for task thread, if any
	if (gpVidCtx) 
	{
		CKernelMPI		kernel;
		kernel.Free(gpVidCtx);
		gpVidCtx = NULL;
		
#if USE_MUTEX
		kernel.DeInitMutex(gVidMutex);
#endif
	}

	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
#if USE_MUTEX
	if (gpVidCtx != NULL) {
		CKernelMPI kernel;
		kernel.LockMutex(*gpVidCtx->pMutex);
	}
#endif
	// Note theora_granule_time() returns only seconds
	pTime->frame = theora_granule_frame(&td,td.granulepos);
	pTime->time  = pTime->frame * 1000 * ti.fps_denominator / ti.fps_numerator;
	dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::GetVideoTime: frame %ld, time %ld ms\n", 
		static_cast<long>(pTime->frame), static_cast<long>(pTime->time));
#if USE_MUTEX
	if (gpVidCtx != NULL) {
		CKernelMPI kernel;
		kernel.UnlockMutex(*gpVidCtx->pMutex);
	}
#endif
	return true;	
}

//----------------------------------------------------------------------------
Boolean CVideoModule::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	Boolean		ready = false;
	ogg_packet  op;
	ogg_int64_t frame;
	ogg_int64_t ftime = 0;
	tVideoTime*	pTime = pCtx;
	int			bytes;

	(void )hVideo;	/* Prevent unused variable warnings. */
	// Compute next frame if time-based drop-frame mode selected 
	if (bDrop)
		pTime->frame = pTime->time * ti.fps_numerator / (ti.fps_denominator * 1000);

	// Modally loop until next frame found or end of file
	while (!ready)
	{
		// Decode Theora packet from Ogg input stream
		// (Dropped frames must decode from next key frame)
		if ((ogg_stream_packetout(&to,&op) > 0) /* && 
				(!bDrop || theora_packet_iskeyframe(&op) && 
					theora_granule_frame(&td,op.granulepos) >= pTime->frame) */ )
		{
			// Only decode if at selected time frame or no dropped frames
			PROFILE_BEGIN();
			theora_decode_packetin(&td,&op);
			PROFILE_END("theora_decode_packetin");
			frame = theora_granule_frame(&td,td.granulepos);
			if (!bDrop || frame >= pTime->frame)
			{
				// Note theora_granule_time() returns only seconds
				pTime->frame = frame;
				pTime->time  = frame * 1000 * ti.fps_denominator / ti.fps_numerator;
#ifdef EMULATION
				ftime = static_cast<ogg_int64_t>(theora_granule_time(&td,td.granulepos));
#endif
				dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::GetVideoFrame: Found frame %ld, time %ld (msec), time %ld (theora)\n", 
					static_cast<long>(frame), static_cast<long>(pTime->time), static_cast<long>(ftime));
				ready = true;
			}
			else
			{
				dbg_.DebugOut(kDbgLvlImportant, "VideoModule::GetVideoFrame: Dropped frame %ld, time %ld (msec)\n", 
					static_cast<long>(frame), static_cast<long>(pTime->time));
			}
		}
		else
		{
			// Get more packet data from Ogg input stream
			bytes = buffer_data(&oy);
			if (bytes == 0)
				break;
			while (ogg_sync_pageout(&oy,&og) > 0)
				queue_page(&og);
		}
	}
	
	return ready;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx)
{
	tVideoTime	time;
	Boolean		found = false;
	ogg_packet  op;
	ogg_int64_t frame;
	int			bytes;
	
	// Compare selected frame time to current frame time
	GetVideoTime(hVideo, &time);
	if (time.frame > pCtx->frame)
	{	
		// If we already past seek frame, we need to rewind from start
		DeInitVideoInt(hVideo);
		fseek(gfile, 0, SEEK_SET);
		InitVideoInt(hVideo);
	}
	
	// Loop until we find the selected frame or end of file
	while (!found)
	{
		// Get Theora packet from Ogg input stream
		if (ogg_stream_packetout(&to,&op) > 0)
		{
			// Find Theora packet at selected time frame
			theora_decode_packetin(&td,&op);
			frame = theora_granule_frame(&td,td.granulepos);
			if (frame == pCtx->frame)
			{
				found = true;
				break;
			}
		}
		else
		{
			// Get more packet data from Ogg input stream
			bytes = buffer_data(&oy);
			if (bytes == 0)
				break;
			while (ogg_sync_pageout(&oy,&og) > 0)
				queue_page(&og);
		}
	}

	return found;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	tVideoTime	time;
	(void)		pCtx;	// unused in original MPI
	return SyncVideoFrame(hVideo, &time, false);
}

//----------------------------------------------------------------------------
// YUV to RGB color conversion: <http://en.wikipedia.org/wiki/YUV>
inline 	U8 clip(S16 X)			{ return (X < 0) ? 0 : (X > 255) ? 255 : static_cast<U8>(X); }
inline	S16 C(U8 Y)  			{ return (Y - 16); }
inline 	S16 D(U8 U)  			{ return (U - 128); }
inline 	S16 E(U8 V)  			{ return (V - 128); }
inline 	U8 R(U8 Y,U8 U,U8 V)	{ (void )U; return clip(( 298 * C(Y)              + 409 * E(V) + 128) >> 8); }
inline 	U8 G(U8 Y,U8 U,U8 V)	{ return clip(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8); }
inline 	U8 B(U8 Y,U8 U,U8 V)	{ (void )V; return clip(( 298 * C(Y) + 516 * D(U)              + 128) >> 8); }

//----------------------------------------------------------------------------
Boolean CVideoModule::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	// Output decoded Theora packet to YUV surface
	yuv_buffer 	yuv;
	PROFILE_BEGIN();
	theora_decode_YUVout(&td,&yuv);
	PROFILE_END("theora_decode_YUVout");

	// Pack into YUV or RGB format surface
	tVideoSurf*	surf = pCtx;
	U8* 		s = yuv.y;
	U8*			u = yuv.u;
	U8*			v = yuv.v;
	U8*			d = surf->buffer; // + (y * surf->pitch) + (x * 2);
	int			i,j,k,m;
	if (surf->format == kPixelFormatYUV420)
	{
		// Pack into separate YUV planar surface regions
		U8*		du = surf->buffer + surf->width; // U,V in double-width buffer
		U8*		dv = surf->buffer + surf->width + surf->pitch * surf->height/2;
		for (i = 0; i < yuv.y_height; i++) 
		{
			memcpy(d, s, yuv.y_width);
			s += yuv.y_stride;
			d += surf->pitch;
			if (i % 2) 
			{
				memcpy(du, u, yuv.uv_width);
				memcpy(dv, v, yuv.uv_width);
				u += yuv.uv_stride;
				v += yuv.uv_stride;
				du += surf->pitch;
				dv += surf->pitch;
			}
		}
	}
	else if (surf->format == kPixelFormatYUYV422)
	{
		// Pack into YUYV format surface
		for (i = 0; i < yuv.y_height; i++) 
		{
			for (j = k = m = 0; k < yuv.y_width; j++, k+=2, m+=4) 
			{
				d[m+0] = s[k];
				d[m+1] = u[j];
				d[m+2] = s[k+1];
				d[m+3] = v[j];
			}
			s += yuv.y_stride;
			d += surf->pitch;
			if (i % 2) 
			{
				u += yuv.uv_stride;
				v += yuv.uv_stride;
			}
		}
	}
	else if (surf->format == kPixelFormatARGB8888)
	{
		// Convert YUV to RGB format surface
		for (i = 0; i < yuv.y_height; i++) 
		{
			for (j = k = m = 0; k < yuv.y_width; j++, k+=2, m+=8) 
			{
				U8 y0 = s[k];
				U8 y1 = s[k+1];
				U8 u0 = u[j];
				U8 v0 = v[j];
				d[m+0] = B(y0,u0,v0);
				d[m+1] = G(y0,u0,v0);
				d[m+2] = R(y0,u0,v0);
				d[m+3] = 0xFF;
				d[m+4] = B(y1,u0,v0);
				d[m+5] = G(y1,u0,v0);
				d[m+6] = R(y1,u0,v0);
				d[m+7] = 0xFF;
			}
			s += yuv.y_stride;
			d += surf->pitch;
			if (i % 2) 
			{
				u += yuv.uv_stride;
				v += yuv.uv_stride;
			}
		}
	}
	else
		return false;
	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::PauseVideo(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	if (!gpVidCtx)
		return false;
	gpVidCtx->bPaused = true;
	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::ResumeVideo(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	if (!gpVidCtx)
		return false;
	gpVidCtx->bPaused = false;
	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoPaused(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	return (gpVidCtx) ? gpVidCtx->bPaused : false;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoPlaying(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	return (gpVidCtx) ? gpVidCtx->bPlaying : false;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoLooped(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
	return (gpVidCtx) ? gpVidCtx->bLooped : false;
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
