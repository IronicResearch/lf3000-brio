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
#include <SystemResourceMPI.h>

//#define _GNU_SOURCE
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <ogg/ogg.h>
#include <theora/theora.h>

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
	tRsrcHndl			ghVideo;
}

//============================================================================
// C function support
//============================================================================

//----------------------------------------------------------------------------
// Grab some more compressed bitstream and sync it for page extraction
static int buffer_data(/* FILE *in, */ ogg_sync_state *oy)
{
	CSystemResourceMPI	rsrcmgr;
	char 	*buffer = ogg_sync_buffer(oy,4096);
	U32 	bytes; // = fread(buffer,1,4096,in);
	rsrcmgr.ReadRsrc(ghVideo, buffer, 4096, &bytes, kOpenRsrcOptionRead, NULL);
	ogg_sync_wrote(oy,bytes);
	return bytes;
}

//----------------------------------------------------------------------------
// Push a page into the steam for packetization
static int queue_page(ogg_page *page)
{
	ogg_stream_pagein(&to,&og);
	return 0;
}

//============================================================================
// Ctor & dtor
//============================================================================
CVideoModule::CVideoModule() : dbg_(kGroupVideo)
{
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

//----------------------------------------------------------------------------
tVideoHndl CVideoModule::StartVideo(tRsrcHndl hRsrc)
{
	tVideoHndl	hVideo = kInvalidVideoHndl;
	tErrType	r;

	// Sanity check
	if (hRsrc == kInvalidRsrcHndl) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: invalid resource handle\n");
		return kInvalidVideoHndl;
	}

	// Open Ogg file associated with resource
	CSystemResourceMPI	rsrcmgr;
	r = rsrcmgr.OpenRsrc(hRsrc, kOpenRsrcOptionRead, NULL);
	if (r != kNoErr) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: LoadRsrc failed for %0X\n", hRsrc);
		return kInvalidVideoHndl;
	}
	hVideo = (tVideoHndl)(ghVideo = hRsrc); 

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
				rsrcmgr.CloseRsrc(hRsrc);
				return kInvalidVideoHndl;
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
				rsrcmgr.CloseRsrc(hRsrc);
				return kInvalidVideoHndl;
			}
		}
	}

	// Theora packets should have been detected by now
	if (!theorapkts)
	{
		dbg_.DebugOut(kDbgLvlCritical, "VideoModule::StartVideo: Unable to find Theora stream headers\n");
		rsrcmgr.CloseRsrc(hRsrc);
		return kInvalidVideoHndl;
	}

	// Start decoder stream
	theora_decode_init(&td,&ti);

	// Queue remaining pages that did not contain headers
	while (ogg_sync_pageout(&oy,&og) > 0)
		queue_page(&og);

	return hVideo;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::StopVideo(tVideoHndl hVideo)
{
	// Cleanup decoder stream resources
	ogg_stream_clear(&to);
	theora_clear(&td);
	theora_comment_clear(&tc);
	theora_info_clear(&ti);
	ogg_sync_clear(&oy);
	
	// Close file associated with resource
	CSystemResourceMPI	rsrcmgr;
	rsrcmgr.CloseRsrc(ghVideo);
	ghVideo = kInvalidRsrcHndl;
			
	return true;
}

//----------------------------------------------------------------------------
Boolean CVideoModule::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	Boolean		ready = false;
	ogg_packet  op;
	int			bytes;

	// TODO/dm: Do we want this as a non-blocking call?

	// Modally loop until next frame found or end of file
	while (!ready)
	{
		// Decode Theora packet from Ogg input stream
		if (ogg_stream_packetout(&to,&op) > 0)
		{
			theora_decode_packetin(&td,&op);
			ready = true;
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
Boolean CVideoModule::PutVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	// Output decoded Theora packet to YUV surface
	yuv_buffer 	yuv;
	theora_decode_YUVout(&td,&yuv);

	// Pack into YUYV format surface
	tVideoSurf*	surf = (tVideoSurf*)pCtx;
	U8* 		s = yuv.y;
	U8*			u = yuv.u;
	U8*			v = yuv.v;
	U8*			d = surf->buffer; // + (y * surf->pitch) + (x * 2);
	int			i,j,k,m;
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
		if (i % 2) 
		{
			u += yuv.uv_stride;
			v += yuv.uv_stride;
		}
		d += surf->pitch;
	}

	return true;
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================

LF_USING_BRIO_NAMESPACE()

#ifndef LF_MONOLITHIC_DEBUG
	static CVideoModule*	sinst = NULL;
	//------------------------------------------------------------------------
	extern "C" ICoreModule* CreateInstance(tVersion version)
	{
		if( sinst == NULL )
			sinst = new CVideoModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	extern "C" void DestroyInstance(ICoreModule* ptr)
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
#endif	// LF_MONOLITHIC_DEBUG


// EOF
