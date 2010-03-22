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
#include <errno.h>
#include <string.h>

//#define _GNU_SOURCE
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <ogg/ogg.h>
#include <theora/theora.h>

#ifndef EMULATION
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/lf1000/mlc_ioctl.h>
#endif

//#define ENABLE_PROFILING
#include <FlatProfiler.h>

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
	CPath				gfilepath = "";
	FILE*				gfile = NULL;
	tMutex				gVidMutex = PTHREAD_MUTEX_INITIALIZER;
	Boolean				gbCodecReady = false;
	Boolean				gbCentered = false;
	tVideoHndl			ghVideoHndl = kInvalidVideoHndl;
	
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
	return ogg_stream_pagein(&to,&og);
}

//----------------------------------------------------------------------------
// Set video scaler (embedded only)
static void SetScaler(int width, int height, bool centered)
{
#ifndef EMULATION
	int	layer, r, dw, dh;
	union mlc_cmd c;

	// Open video layer device
	layer = open("/dev/layer2", O_RDWR|O_SYNC);
	if (layer < 0)
		return;

	// Get position info when video context was created
	r = ioctl(layer, MLC_IOCGPOSITION, &c);
	dw = (r == 0) ? c.position.right - c.position.left + 0 : 320;
	dh = (r == 0) ? c.position.bottom - c.position.top + 0 : 240;
	
	// Reposition with fullscreen centering instead of scaling
	if (centered) 
	{
		dw = width;
		dh = height;		
		c.position.left = (320 - width) / 2;
		c.position.right = c.position.left + width;
		c.position.top = (240 - height) / 2;
		c.position.bottom = c.position.top + height;
		ioctl(layer, MLC_IOCSPOSITION, &c);
	}
	// Special case handling for 320x176 scaling (non-letterbox)
	else if (176 == height && 320 == width) 
	{
		dw = 320;
		dh = 240;
		c.position.left = c.position.top = 0;
		c.position.right = c.position.left + dw;
		c.position.bottom = c.position.top + dh;
		ioctl(layer, MLC_IOCSPOSITION, &c);
	}

	// Set video scaler for video source and screen destination
	c.overlaysize.srcwidth = width;
	c.overlaysize.srcheight = height;
	c.overlaysize.dstwidth = dw; 
	c.overlaysize.dstheight = dh; 
	ioctl(layer, MLC_IOCSOVERLAYSIZE, &c);
	ioctl(layer, MLC_IOCTDIRTY, 0);

	close(layer);
#endif
}

//============================================================================
// Ctor & dtor
//============================================================================
CVideoModule::CVideoModule() : dbg_(kGroupVideo)
{
	dbg_.SetDebugLevel(kVideoDebugLevel);
	FlatProfilerInit(2, FLATPROF_NUM_TIMESTAMPS);
}

//----------------------------------------------------------------------------
CVideoModule::~CVideoModule()
{
	FlatProfilerDone();
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
	if (gpVidCtx != NULL)
		goto ExitPt;

	// Start Theora codec for selected video file
	hVideo = StartVideoInt(path);
	if (hVideo == kInvalidVideoHndl)
		goto ExitPt;

	pVidCtx = static_cast<tVideoContext*>(kernel_.Malloc(sizeof(tVideoContext)));
	memset(pVidCtx, 0, sizeof(tVideoContext));
	
#ifndef EMULATION
	// Set HW video scaler for video source width and height
	SetScaler(ti.width, ti.height, gbCentered);
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
	pVidCtx->uFrameTime = 1000 * ti.fps_denominator / ti.fps_numerator;
	pVidCtx->pMutex		= &gVidMutex;
	pVidCtx->bSeeked	= false;

	InitVideoTask(pVidCtx);	

	// TODO: Wrap pVidCtx into handle...
	gpVidCtx = pVidCtx;

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
	
	return ghVideoHndl = hVideo;
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
	dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::StartVideo: output %dx%d at %d,%d\n",
		ti.frame_width, ti.frame_height, ti.offset_x, ti.offset_y);
	dbg_.Assert(ti.fps_numerator != 0, "VideoModule::StartVideo: bad fps numerator\n");
	dbg_.Assert(ti.fps_denominator != 0, "VideoModule::StartVideo: bad fps denominator\n");
	dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::StartVideo: %d frames/sec, %d msec/frame\n",
		ti.fps_numerator / ti.fps_denominator, 1000 * ti.fps_denominator / ti.fps_numerator);

	// Loop thru Theora header comments for any tags of interest
	gbCentered = false;
	if (tc.comments)
	{
		char** pstr = tc.user_comments;
		dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::StartVideo: vendor: %s\n", tc.vendor);
		dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::StartVideo: comments: %d (tags)\n", tc.comments);
		for (int i = 0; i < tc.comments && pstr != NULL; i++, pstr++)
		{
			dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::StartVideo: tag %d: %s\n", i, *pstr);
			if (strncmp(*pstr, "LOCATION", 8) == 0)
			{
				char buf[40];
				sscanf(*pstr, "LOCATION=%s", buf);
				if (strncmp(buf, "center", 6) == 0)
					gbCentered = true;
			}
		}
	}

	// Queue remaining pages that did not contain headers
	while (ogg_sync_pageout(&oy,&og) > 0)
		queue_page(&og);

	theora_decode_packetin(&td,&op);
	theora_granule_frame(&td,td.granulepos);

	return gbCodecReady = true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean	state = false;
	if (gbCodecReady)
	{
		pInfo->width	= ti.width;
		pInfo->height	= ti.height;
		pInfo->fps		= ti.fps_numerator / ti.fps_denominator;
		state = true;
	}
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif

	return state;
}

//----------------------------------------------------------------------------
void CVideoModule::DeInitVideoInt(tVideoHndl hVideo)
{	
	(void )hVideo;	/* Prevent unused variable warnings. */
	// Cleanup decoder stream resources
	gbCodecReady = false;
	ogg_stream_clear(&to);
	theora_clear(&td);
	theora_comment_clear(&tc);
	theora_info_clear(&ti);
	ogg_sync_clear(&oy);
}

//----------------------------------------------------------------------------
Boolean CVideoModule::StopVideo(tVideoHndl hVideo)
{
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	// Kill video task, if running
	if (gpVidCtx != NULL && gpVidCtx->hVideoThread != kNull)
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
		kernel_.Free(gpVidCtx);
		gpVidCtx = NULL;
	}

	ghVideoHndl = kInvalidVideoHndl;
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif

	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	// Note theora_granule_time() returns only seconds
	if (gbCodecReady)
	{
		pTime->frame = theora_granule_frame(&td,td.granulepos);
		pTime->time  = pTime->frame * 1000 * ti.fps_denominator / ti.fps_numerator;
		dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::GetVideoTime: frame %ld, time %ld ms\n", 
			static_cast<long>(pTime->frame), static_cast<long>(pTime->time));
	}
	
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return true;	
}

//----------------------------------------------------------------------------
Boolean CVideoModule::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif
	Boolean		ready = false;
	ogg_packet  op;
	ogg_int64_t frame;
	ogg_int64_t ftime = 0;
	tVideoTime*	pTime = pCtx;
	int			bytes;

	(void )hVideo;	/* Prevent unused variable warnings. */
	
	// Internal codec state only changes during start/stop
	if (!gbCodecReady)
		return false;

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
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return ready;
}

//----------------------------------------------------------------------------
//Does a binary search for the a frame.  The framepos of the very first frame is 1.
//Modifies left_pos and right_pos as it goes, to help narrow future searches.
//
//Returns the Theora granulepos found (bounded by first and last frame).
ogg_int64_t BinarySeekFrame(ogg_int64_t target_framepos, long int &left_pos, long int &right_pos)
{
	ogg_int64_t granulepos;
	int			bytes;
	ogg_packet	op;
	CDebugMPI dbg_(kGroupVideo);
	
	long int mid_pos = left_pos + (right_pos - left_pos) / 2;
	fseek(gfile, mid_pos, SEEK_SET);
	ogg_sync_reset(&oy);
	ogg_stream_reset(&to);
	
	int theora_keyframe_shift = theora_granule_shift(&ti);
	ogg_int64_t first_frame_granulepos = 1 << theora_keyframe_shift;
	ogg_int64_t offset_mask = first_frame_granulepos - 1;
	ogg_int64_t left_page_granulepos = 0;
	
	long right_pageno = 0x7fffffff;
	long left_pageno = 0x7fffffff;
	
	//If we're looking for a frame before the first frame,
	//pretend we're looking for the first frame
	if(target_framepos < 1)
		target_framepos = 1;
	
	dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Looking for target_framepos=%llx\n", target_framepos);
	//Find the page left of frame
	bool page_found = false;
	while(!page_found)
	{
		int pageout_status = ogg_sync_pageout(&oy,&og);
		switch(pageout_status)
		{
		case -1:
			//not yet synced, try another page
			break;
		case 0:
			//Need more data to get a page
			bytes = buffer_data(&oy);
			if(bytes == 0)
			{
				page_found = true;//No more data, last page
				//why didn't Last Page B catch this?
				dbg_.DebugOut(kDbgLvlImportant, "BinarySeekFrame Last Page A Should not happen\n");
			}
			break;
		
		case 1:
			//We got a page, queue it up!
			if(!queue_page(&og))
			{
				//Grab the granulepos of the page, which is also the last packet in this page
				ogg_int64_t page_granulepos = ogg_page_granulepos(&og);
				long pageno = ogg_page_pageno(&og);
				
				//Negative granulepos mean that no packets finish in this page.
				if(page_granulepos > 0)
				{
					ogg_int64_t page_framepos;
					
					if(page_granulepos < first_frame_granulepos)
						page_framepos = 0;
					else
						page_framepos = (page_granulepos >> theora_keyframe_shift) + (page_granulepos & offset_mask);
					
					if(ogg_page_eos(&og) && page_framepos < target_framepos)
					{
						//Last page, frame we're looking for doesn't exist, past the end
						//Pretend the frame we're looking for is the last frame instead.
						dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Last Page B, look left\n");
						target_framepos = page_framepos;
					}
				
					if(page_framepos < target_framepos)
					{
						//Frame we're looking for is right of this page
						if(left_page_granulepos < page_granulepos)
						{
							
							//look to the right some more
							dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Page Right A page_framepos=%llx, page_granulepos=%llx, pageno=%lx\n", page_framepos, page_granulepos, pageno);
							left_pageno = right_pageno = 0x7fffffff;
							
							left_page_granulepos = page_granulepos;
							left_pos = mid_pos;
							mid_pos = left_pos + (right_pos - left_pos) / 2;
							fseek(gfile, mid_pos, SEEK_SET);
							ogg_sync_reset(&oy);
							ogg_stream_reset(&to);
						}
						else
						{
							//We were previously on this page, looked to the right and came back looking to the left.
							//This is the latest page that finishes a frame before target_framepos
							dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Page Found A page_framepos=%llx, page_granulepos=%llx, pageno=%lx\n", page_framepos, page_granulepos, pageno);
							page_found = true;
						}
					}
					else if(target_framepos <= page_framepos)
					{
						//Frame we're looking for is left of this page, or finishes on this page
						if(left_page_granulepos < page_granulepos)
						{
							//Look farther left
							if(left_pos == mid_pos)
							{
								//There isn't anything farther left to look
								//Just call this the page we're looking for, and hope for the best
								//It's unlikely this will happen with the new check for left_pageno and rightpageno
								dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Page Found B page_framepos=%llx, page_granulepos=%llx, pageno=%lx\n", page_framepos, page_granulepos, pageno);
								page_found = true;
							}
							else
							{
								//look to the left some more
								dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Page Left A page_framepos=%llx, page_granulepos=%llx, pageno=%lx\n", page_framepos, page_granulepos, pageno);
								if(left_pageno < pageno)
									right_pageno = left_pageno - 1;
								else
									right_pageno = pageno - 1;
								right_pos = mid_pos;
								mid_pos = left_pos + (right_pos - left_pos) / 2;
								fseek(gfile, mid_pos, SEEK_SET);
								ogg_sync_reset(&oy);
								ogg_stream_reset(&to);
							}
						}
						else
						{
							//How did we end up this with the frame left of left_page_granulepos?
							dbg_.DebugOut(kDbgLvlImportant, "BinarySeekFrame Page Found C should not happen, page_framepos=%llx, page_granulepos=%llx\n", page_framepos, page_granulepos);
							page_found = true;
						}
					}
				}
				else
				{
					//Try to exit early string of pages that lead to packet we want to look left from
					//right_pageno is the highest page number that could possibly come before the last look to the left.
					//It starts at pageno - 1 when we first found a granulepos and looked left.
					//left_pageno should be the start of the no finished packet zone
					//if we manage to reach right_pageno, then this string of pages eventually ends with packet we want to look left from
					//Reset right_pageno to left_pageno -1, so that if we reached the previous start of this known string
					//of pages, then we should look left some more.
					if(left_pos != mid_pos)
					{
						if(pageno == right_pageno)
						{
							//If the current pageno is the right_pageno, and we still haven't gotten to a page
							//that finishes a packet, then the next packet to finish will be the same one we started
							//looking left from
							//Try reading farther left
							dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Page Left B pageno=%lx\n", pageno);
							
							//The latest page that could possibly finish a page now is the page before
							//the start of the current chain of pages
							right_pageno = left_pageno - 1;
							right_pos = mid_pos;
							mid_pos = left_pos + (right_pos - left_pos) / 2;
							fseek(gfile, mid_pos, SEEK_SET);
							ogg_sync_reset(&oy);
							ogg_stream_reset(&to);
						}
						else if(pageno < left_pageno)
						{
							//We're reading from earlier than before
							//We might eventually wind up back at right_pageno, save the left_pageno just in case
							dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Page Right B pageno=%lx\n", pageno);
							left_pageno = pageno;
						}
						else
						{
							//Yet another page that still doesn't finish a packet
							dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Page Right C pageno=%lx\n", pageno);
						}
					}
				}
			}
			break;
		}
	}

	granulepos = left_page_granulepos;
	ogg_int64_t framepos = (granulepos >> theora_keyframe_shift) + (granulepos & offset_mask);
	
	if(!left_page_granulepos)
	{
		//It was in the first page, there is no page left of target frame
		//We should already be in the first page, look for the first frame
		//Grab Theora packets
		while (ogg_stream_packetout(&to,&op) > 0)
		{
			//Discard headers
			if(!theora_packet_isheader(&op))
			{
				//Found the first frame
				dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Frame First found\n");
				granulepos = first_frame_granulepos;
				framepos = 1;
				page_found = true;
				break;
			}
		}
	}
	else
	{
		//Page left of frame found, now find the frame
		//Skip to last packet of the page
		//Since we're not seeking anymore, have to make sure the stream is properly synchronized
		while(ogg_stream_packetout(&to,&op) != 0);
	}
	
	
	bool frame_found = false;
	if( framepos == target_framepos)
	{
		//By coincidence, this is frame we're looking for, early out
		dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Frame A found framepos=%llx, granulepos=%llx\n", framepos, granulepos);
		op.granulepos = granulepos;
		frame_found = true;
		if(theora_packet_iskeyframe(&op))
		{
			//Decode if the frame in question is a keyframe
			dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Decode A granulepos=%llx\n", granulepos);
			theora_control(&td, TH_DECCTL_SET_GRANPOS, &granulepos, sizeof(granulepos));
			theora_decode_packetin(&td,&op);
		}
	}
	
	//Edge case of frame is in first page, which is already read in at this point
	//We're reading in a second page for no reason.
	//Doesn't hurt, ogg_sync_state handles multiple pages fine
	//If we're going to continue playing from here, we would have read in this page later anyways.
	//If we're seeking again immediately, we read extra data for no reason
	while(!frame_found)
	{
		int pageout_status = ogg_sync_pageout(&oy,&og);
		switch(pageout_status)
		{
		case -1:
			//not yet synced, try another page
			break;
		case 0:
			//Need more data to get a page
			bytes = buffer_data(&oy);
			if(bytes == 0)
			{
				frame_found = true;//No more data, last page
				//How did we wind up at this point looking for a frame past the end?
				//Didn't we cap it at Last Page B?
				dbg_.DebugOut(kDbgLvlImportant, "BinarySeekFrame Last Page E, should not happen\n");
			}
			break;
		
		case 1:
			//We got a page, queue it up!
			if(!queue_page(&og))
			{
				//Grab Theora packets
				while (ogg_stream_packetout(&to,&op) > 0)
				{
					//Manual counting of framepos, to avoid unneeded theora_decode_packetin
					++framepos;
					
					//Calculate proper granulepos
					if(theora_packet_iskeyframe(&op))
						granulepos = framepos << theora_keyframe_shift;
					else
						++granulepos;
					
					if(framepos == target_framepos)
					{
						dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Frame B found framepos=%llx, granulepos=%llx\n", framepos, granulepos);
						frame_found = true;
						if(theora_packet_iskeyframe(&op))
						{
							dbg_.DebugOut(kDbgLvlVerbose, "BinarySeekFrame Decode B granulepos=%llx\n", granulepos);
							op.granulepos = granulepos;
							theora_control(&td, TH_DECCTL_SET_GRANPOS, &granulepos, sizeof(granulepos));
							theora_decode_packetin(&td,&op);
						}
						break;
					}
				}
			}
			break;
		}
	}
	
	return granulepos;
}

Boolean CVideoModule::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact)
{
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif
	tVideoTime	time;
	Boolean		found = false;
	ogg_int64_t	frame;
	ogg_packet	op;
	int			bytes;
	
	if (!gbCodecReady)
		return false;

	// Compare selected frame time to current frame time
	//GetVideoTime(hVideo, &time);
	time.frame = theora_granule_frame(&td,td.granulepos);
	time.time  = time.frame * 1000 * ti.fps_denominator / ti.fps_numerator;
	
	//If time.frame is -1, we haven't decoded a frame yet, do full file search
	//If the frame we're looking for is in the future, do a full file search
	//If neither is the case, it's ok to stop looking at the current file position.
	if(time.frame == -1 || time.frame < pCtx->frame)
		fseek(gfile, 0, SEEK_END);
	long int right_pos = ftell(gfile);
	
	//It is not safe to search with left_pos at any value other than 0.
	//Even if the the frame we're looking for is in the future, it's page may have started previous to the current time
	long int left_pos = 0;
	
	ogg_int64_t target_framepos = pCtx->frame + 1;
	dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::SeekVideoFrame looking for target_framepos=%llx\n", target_framepos);
	TimeStampOn(0);
	ogg_int64_t target_granulepos = BinarySeekFrame(target_framepos, left_pos, right_pos);
	
	int theora_keyframe_shift = theora_granule_shift(&ti);
	ogg_int64_t target_keyframepos = target_granulepos >> theora_keyframe_shift;
	//Do we need to look for a keyframe?
	if(target_keyframepos != target_framepos)
	{
		//Do another binary search for keyframe
		dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::SeekVideoFrame looking for target_keyframepos=%llx\n", target_keyframepos);
		left_pos = 0;
		target_granulepos = BinarySeekFrame(target_keyframepos, left_pos, right_pos);
	}
	else
	{
		found = true;
	}
	TimeStampOff(0);
	
	frame = theora_granule_frame(&td,td.granulepos);
	
	if(bExact)
	{
		TimeStampOn(1);
		//We have the KeyFrame before target frame, decode till we reach the target frame
		while(frame < pCtx->frame)
		{
			if (ogg_stream_packetout(&to,&op) > 0)
			{
				theora_decode_packetin(&td,&op);
				frame = theora_granule_frame(&td,td.granulepos);
				if (frame == pCtx->frame)
				{
					dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::SeekVideoFrame Found frame=%llx  td.granulepos=%llx\n", frame, td.granulepos);
					found = true;
					break;
				}
			}
			else
			{
				// Get more packet data from Ogg input stream
				bytes = buffer_data(&oy);
				if (bytes == 0)
				{
					dbg_.DebugOut(kDbgLvlVerbose, "VideoModule::SeekVideoFrame No more frames, exiting at last frame\n");
					break;
				}
				while (ogg_sync_pageout(&oy,&og) > 0)
					queue_page(&og);
			}
		}
		TimeStampOff(1);
	}
	
	gpVidCtx->bSeeked = true;
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
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
	Boolean status = true;
	
	// Internal codec state only changes during start/stop
	if (!gbCodecReady)
		return false;

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
		U8*		du = surf->buffer + surf->pitch/2; // U,V in double-width buffer
		U8*		dv = surf->buffer + surf->pitch/2 + surf->pitch * (surf->height/2);
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
		status = false;

	return status;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::PauseVideo(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (gpVidCtx) ? gpVidCtx->bPaused = true : false;
		
#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::ResumeVideo(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (gpVidCtx) ? gpVidCtx->bPaused = false : false;

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoPaused(tVideoHndl hVideo)
{

	(void )hVideo;	/* Prevent unused variable warnings. */
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (gpVidCtx) ? gpVidCtx->bPaused : false;

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoPlaying(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */
#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (gpVidCtx) ? gpVidCtx->bPlaying : false;

#if USE_MUTEX
	kernel_.UnlockMutex(gVidMutex);
#endif
	return state;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::IsVideoLooped(tVideoHndl hVideo)
{
	(void )hVideo;	/* Prevent unused variable warnings. */

#if USE_MUTEX
	kernel_.LockMutex(gVidMutex);
#endif

	Boolean state = (gpVidCtx) ? gpVidCtx->bLooped : false;

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
