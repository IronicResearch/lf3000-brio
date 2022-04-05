//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VPUPlayer.cpp
//
// Description:
//		Implementation of the VPUPlayer class derived from AVIPlayer.
//
//==============================================================================

#if USE_VPU

#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <VideoPriv.h>
#include <VPUPlayer.h>
#include <DisplayMPI.h>

#include <nx_alloc_mem.h>
#include <nx_video_api.h>
#include <nx_fourcc.h>
#include <nx_dsp.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Locals
//==============================================================================

#define	NX_MAX_NUM_SPS		3
#define	NX_MAX_SPS_SIZE		1024
#define	NX_MAX_NUM_PPS		3
#define	NX_MAX_PPS_SIZE		1024

typedef struct {
	int				version;
	int				profile_indication;
	int				compatible_profile;
	int				level_indication;
	int				nal_length_size;
	int				num_sps;
	int				sps_length[NX_MAX_NUM_SPS];
	unsigned char	sps_data  [NX_MAX_NUM_SPS][NX_MAX_SPS_SIZE];
	int				num_pps;
	int				pps_length[NX_MAX_NUM_PPS];
	unsigned char	pps_data  [NX_MAX_NUM_PPS][NX_MAX_PPS_SIZE];
} NX_AVCC_TYPE;

#ifndef MKTAG
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#endif

#define PUT_LE32(_p, _var) \
	*_p++ = (unsigned char)((_var)>>0);  \
	*_p++ = (unsigned char)((_var)>>8);  \
	*_p++ = (unsigned char)((_var)>>16); \
	*_p++ = (unsigned char)((_var)>>24);

#define PUT_BE32(_p, _var) \
	*_p++ = (unsigned char)((_var)>>24);  \
	*_p++ = (unsigned char)((_var)>>16);  \
	*_p++ = (unsigned char)((_var)>>8); \
	*_p++ = (unsigned char)((_var)>>0);

#define PUT_LE16(_p, _var) \
	*_p++ = (unsigned char)((_var)>>0);  \
	*_p++ = (unsigned char)((_var)>>8);


#define PUT_BE16(_p, _var) \
	*_p++ = (unsigned char)((_var)>>8);  \
	*_p++ = (unsigned char)((_var)>>0);

namespace
{
	//----------------------------------------------------------------------------
	static int ParseAVCStream( AVPacket *pkt, int nalLengthSize, unsigned char *buffer, int outBufSize )
	{
		int nalLength;

		//	input
		unsigned char *inBuf = pkt->data;
		int inSize = pkt->size;
		int pos=0;

		//	'avcC' format
		do {
			nalLength = 0;

			if( nalLengthSize == 2 )
			{
				nalLength = inBuf[0]<< 8 | inBuf[1];
			}
			else if( nalLengthSize == 3 )
			{
				nalLength = inBuf[0]<<16 | inBuf[1]<<8  | inBuf[2];
			}
			else if( nalLengthSize == 4 )
			{
				nalLength = inBuf[0]<<24 | inBuf[1]<<16 | inBuf[2]<<8 | inBuf[3];
			}
			else if( nalLengthSize == 1 )
			{
				nalLength = inBuf[0];
			}

			inBuf  += nalLengthSize;
			inSize -= nalLengthSize;

			if( 0==nalLength || inSize<(int)nalLength )
			{
				printf("Error : avcC type nal length error (nalLength = %d, inSize=%d, nalLengthSize=%d)\n", nalLength, inSize, nalLengthSize);
				return -1;
			}

			//	put nal start code
			buffer[pos + 0] = 0x00;
			buffer[pos + 1] = 0x00;
			buffer[pos + 2] = 0x00;
			buffer[pos + 3] = 0x01;
			pos += 4;

			memcpy( buffer + pos, inBuf, nalLength );
			pos += nalLength;

			inSize -= nalLength;
			inBuf += nalLength;
		} while( 2<inSize );
		return pos;
	}

	//----------------------------------------------------------------------------
	static int NX_ParseSpsPpsFromAVCC( unsigned char *extraData, int extraDataSize, NX_AVCC_TYPE *avcCInfo )
	{
		int pos = 0;
		int i;
		int length;

		if( 1 != extraData[0] || 11 > extraDataSize ) {
			printf( "Error : Invalid \"avcC\" data\n" );
			return -1;
		}

		//	Parser "avcC" format data
		avcCInfo->version				= (int)extraData[pos];			pos++;
		avcCInfo->profile_indication	= (int)extraData[pos];			pos++;
		avcCInfo->compatible_profile	= (int)extraData[pos];			pos++;
		avcCInfo->level_indication		= (int)extraData[pos];			pos++;
		avcCInfo->nal_length_size		= (int)(extraData[pos]&0x03)+1;	pos++;
		//	parser SPSs
		avcCInfo->num_sps				= (int)(extraData[pos]&0x1f);	pos++;
		for( i=0 ; i<avcCInfo->num_sps ; i++) {
			length = avcCInfo->sps_length[i] = (int)(extraData[pos]<<8)|extraData[pos+1];
			pos+=2;
			if( (pos+length) > extraDataSize ) {
				printf("Error : extraData size too small(SPS)\n" );
				return -1;
			}
			memcpy( avcCInfo->sps_data[i], extraData+pos, length );
			pos += length;
		}

		//	parse PPSs
		avcCInfo->num_pps				= (int)extraData[pos];			pos++;
		for( i=0 ; i<avcCInfo->num_pps ; i++ ) {
			length = avcCInfo->pps_length[i] = (int)(extraData[pos]<<8)|extraData[pos+1];
			pos+=2;
			if( (pos+length) > extraDataSize ) {
				printf( "Error : extraData size too small(PPS)\n");
				return -1;
			}
			memcpy( avcCInfo->pps_data[i], extraData+pos, length );
			pos += length;
		}
		return 0;
	}

	//----------------------------------------------------------------------------
	static void NX_MakeH264StreamAVCCtoANNEXB( NX_AVCC_TYPE *avcc, unsigned char *pBuf, int *size )
	{
		int i;
		int pos = 0;
		for( i=0 ; i<avcc->num_sps ; i++ )
		{
			pBuf[pos++] = 0x00;
			pBuf[pos++] = 0x00;
			pBuf[pos++] = 0x00;
			pBuf[pos++] = 0x01;
			memcpy( pBuf + pos, avcc->sps_data[i], avcc->sps_length[i] );
			pos += avcc->sps_length[i];
		}
		for( i=0 ; i<avcc->num_pps ; i++ )
		{
			pBuf[pos++] = 0x00;
			pBuf[pos++] = 0x00;
			pBuf[pos++] = 0x00;
			pBuf[pos++] = 0x01;
			memcpy( pBuf + pos, avcc->pps_data[i], avcc->pps_length[i] );
			pos += avcc->pps_length[i];
		}
		*size = pos;
	}

	//----------------------------------------------------------------------------
	int GetSequenceHeader( AVFormatContext *format, AVStream *stream, NX_AVCC_TYPE *avcc, unsigned char *buffer, int size )
	{
		enum CodecID codecId = stream->codec->codec_id;
		int retSize = 0;
		int nMetaData = stream->codec->extradata_size;
		unsigned char *pbMetaData = stream->codec->extradata;

		// H264 codec
		if ( (codecId == CODEC_ID_H264) && (nMetaData > 0) )
		{
			if ( stream->codec->extradata[0] == 0x1 )
			{
				NX_ParseSpsPpsFromAVCC( pbMetaData, nMetaData, avcc );
				NX_MakeH264StreamAVCCtoANNEXB( avcc, buffer, &retSize );
				return retSize;
			}
		}

		memcpy( buffer, pbMetaData, nMetaData );
		return nMetaData;
	}

	//----------------------------------------------------------------------------
	int ReadStream( AVFormatContext *format, AVStream *stream, int reqSize, unsigned char *buffer, int *size, int *isKey, long long *timeStamp )
	{
		int ret;
		AVPacket pkt;
		enum CodecID codecId = stream->codec->codec_id;
		double timeStampRatio = (double)stream->time_base.num * 1000.0 / (double)stream->time_base.den; // FIXME

		*size = 0;
		*isKey = 0;
		*timeStamp = -1;
		do {
			ret = av_read_frame( format, &pkt );
			if ( ret < 0 )
				return -1;
			if ( pkt.stream_index == stream->index )
			{
				// H264 codec
				if ( codecId == CODEC_ID_H264 && stream->codec->extradata_size > 0 && stream->codec->extradata[0]==1 )
				{
					*size = ParseAVCStream( &pkt, reqSize, buffer, 0 );
					*isKey = (pkt.flags & AV_PKT_FLAG_KEY) ? 1 : 0;
					if ( pkt.pts != AV_NOPTS_VALUE )
						*timeStamp = pkt.pts * timeStampRatio;
					av_free_packet( &pkt );
					return 0;
				}
			}
			av_free_packet( &pkt );
		} while(true);

		return -1;
	}

	//----------------------------------------------------------------------------
	DISPLAY_HANDLE InitDisplay(tVideoSurf* surf)
	{
		DISPLAY_INFO 	dspInfo;
		DISPLAY_HANDLE	hDsp;
		CDisplayMPI 	dispmgr;
		tDisplayHandle 	hvideo = dispmgr.GetCurrentDisplayHandle(surf->format);
		S16 			dx, dy;
		U16 			dw, dh, sw, sh;
		Boolean			vis, ctr;

		// Get video window position and scaler size via DisplayMPI
		dispmgr.GetWindowPosition(hvideo, dx, dy, dw, dh, vis);
		dispmgr.GetVideoScaler(hvideo, sw, sh, ctr);

		// Setup equivalent video window params for the V4L output driver
		memset(&dspInfo, 0, sizeof(dspInfo));
		dspInfo.port = 0;
		dspInfo.module = 0;
		dspInfo.width  = sw; //surf->width;
		dspInfo.height = sh; //surf->height;
		dspInfo.numPlane = 1;
		dspInfo.dspSrcRect.left   = 0;
		dspInfo.dspSrcRect.top    = 0;
		dspInfo.dspSrcRect.right  = dspInfo.width;
		dspInfo.dspSrcRect.bottom = dspInfo.height;
		dspInfo.dspDstRect.left   = dx;
		dspInfo.dspDstRect.top    = dy;
		dspInfo.dspDstRect.right  = dx + dw;
		dspInfo.dspDstRect.bottom = dy + dh;
		hDsp = NX_DspInit( &dspInfo );

		return hDsp;
	}
}

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CVPUPlayer::CVPUPlayer()
	: CAVIPlayer(),
	  hDec(NULL),
	  hDsp(NULL),
	  reqSize(0),
	  outIdx(-1),
	  pStreamBuffer(NULL)
{
	dbg_.DebugOut(kDbgLvlCritical, "%s\n", __FUNCTION__);

	pStreamBuffer = new unsigned char[8192*1024];
}

//----------------------------------------------------------------------------
CVPUPlayer::~CVPUPlayer()
{
	dbg_.DebugOut(kDbgLvlCritical, "%s\n", __FUNCTION__);

	delete[] pStreamBuffer;
}

//----------------------------------------------------------------------------
Boolean	CVPUPlayer::InitVideo(tVideoHndl hVideo)
{
	// Init LibAV video context
	Boolean ret = CAVIPlayer::InitVideo(hVideo);
	if (ret == false) {
		dbg_.DebugOut(kDbgLvlCritical, "%s: CAVIPlayer::InitVideo failed\n", __FUNCTION__);
		return false;
	}

	AVStream* pStream = pFormatCtx->streams[iVideoStream];

	// VPU codec supported?
	if (pCodec->id != CODEC_ID_H264) {
		dbg_.DebugOut(kDbgLvlCritical, "%s: CVPUPlayer codec %d not supported\n", __FUNCTION__, (int)pCodec->id);
		return false;
	}

	// Init VPU decoder
	hDec = NX_VidDecOpen( NX_AVC_DEC, MAKEFOURCC('H', '2', '6', '4'), 0, NULL );
	if (hDec == NULL) {
		dbg_.DebugOut(kDbgLvlCritical, "%s: NX_VidDecOpen failed, returned %p\n", __FUNCTION__, hDec);
		return false;
	}

	// Get VPU codec sequence header
	unsigned char seqData[4096];
	int seqSize, readSize, isKey;
	long long timeStamp;
	NX_AVCC_TYPE avcc; // FIXME
	VID_ERROR_E vidret;

	memset(&avcc, 0, sizeof(avcc));
	seqSize = GetSequenceHeader( pFormatCtx, pStream, &avcc, seqData, sizeof(seqData) );
	if ( seqSize > 0 )
		memcpy( pStreamBuffer, seqData, seqSize );
	reqSize = avcc.nal_length_size; // format specific size

	// Read packet(s) from stream to init VPU codec
	do {

		int ret = ReadStream( pFormatCtx, pStream, reqSize, pStreamBuffer+seqSize, &readSize, &isKey, &timeStamp );
		if (ret != 0) {
			dbg_.DebugOut(kDbgLvlCritical, "%s: ReadStream failed, error = %d\n", __FUNCTION__, ret);
			return false;
		}

		memset( &seqIn, 0, sizeof(seqIn) );
		seqIn.addNumBuffers = 4;
		seqIn.enablePostFilter = 0;
		seqIn.seqInfo = pStreamBuffer;
		seqIn.seqSize = readSize+seqSize;
		seqIn.enableUserData = 0;
		seqIn.disableOutReorder = 0;
		vidret = NX_VidDecInit( hDec, &seqIn, &seqOut );
		if (vidret < VID_ERR_NONE) {
			dbg_.DebugOut(kDbgLvlCritical, "%s: NX_VidDecInit failed, error = %d\n", __FUNCTION__, (int)vidret);
			return false;
		}

		seqSize = 0; // if more to read

	} while (vidret == VID_NEED_STREAM);

	return true;
}

//----------------------------------------------------------------------------
Boolean	CVPUPlayer::DeInitVideo(tVideoHndl hVideo)
{
	NX_VidDecClose(hDec);

	NX_DspClose(hDsp);

	return CAVIPlayer::DeInitVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	return CAVIPlayer::GetVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	// Queue decoded frame via V4L output driver for multi-buffering
	if (hDec) {
		if (!hDsp)
			hDsp = InitDisplay(pCtx);

		if (decOut.outImgIdx != -1)
			NX_DspQueueBuffer( hDsp, &decOut.outImg );

		if (outIdx != -1)
			NX_DspDequeueBuffer( hDsp );

		outIdx = decOut.outImgIdx;
		return true;
	}

	return CAVIPlayer::PutVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	return CAVIPlayer::GetVideoInfo(hVideo, pInfo);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	return CAVIPlayer::GetVideoTime(hVideo, pTime);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	return CAVIPlayer::SyncVideoFrame(hVideo, pCtx, bDrop);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay)
{
	return CAVIPlayer::SeekVideoFrame(hVideo, pCtx, bExact, bUpdateVideoDisplay);
}

//----------------------------------------------------------------------------
S64 CVPUPlayer::GetVideoLength(tVideoHndl hVideo)
{
	return CAVIPlayer::GetVideoLength(hVideo);
}

//----------------------------------------------------------------------------
bool CVPUPlayer::GetNextFrameSW(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int iVideoStream, AVFrame *pFrame)
{
	int ret;
	int done = 0;
	AVPacket pkt, pkt2;

	av_init_packet(&pkt);

	do {

		// read packet for next frame
		do {
			ret = av_read_frame(pFormatCtx, &pkt);
			if (ret < 0)
				break;

			if (pkt.stream_index == iVideoStream)
				; // parse packet for VPU codec...
			else
				av_free_packet(&pkt);
		} while (pkt.stream_index != iVideoStream);

		// decode packet working copy, for more than one pass per frame
		pkt2 = pkt;
		while (pkt2.size > 0) {
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &done, &pkt2);
			if (ret < 0 || done)
				break;

			pkt2.data += ret;
			pkt2.size -= ret;
		}

		av_free_packet(&pkt);

	} while (!done && !(ret < 0));

	return (done != 0);
}

bool CVPUPlayer::GetNextFrameHW(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int iVideoStream, AVFrame *pFrame)
{
	int ret;
	int readSize, isKey;
	long long timeStamp;
	VID_ERROR_E vidret;
	AVStream* pStream = pFormatCtx->streams[iVideoStream];

	do {
		ret = ReadStream( pFormatCtx, pStream, reqSize, pStreamBuffer, &readSize, &isKey, &timeStamp );
		if (ret < 0) {
			dbg_.DebugOut(kDbgLvlCritical, "%s: ReadStream failed, error = %d\n", __FUNCTION__, ret);
			return false;
		}

		memset(&decIn, 0, sizeof(decIn));
		decIn.strmBuf = pStreamBuffer;
		decIn.strmSize = readSize;
		decIn.timeStamp = timeStamp;
		decIn.eos = 0;
		vidret = NX_VidDecDecodeFrame( hDec, &decIn, &decOut );
		if (vidret < VID_ERR_NONE) {
			dbg_.DebugOut(kDbgLvlCritical, "%s: NX_VidDecDecodeFrame failed, error = %d\n", __FUNCTION__, (int)vidret);
			return false;
		}
	} while (vidret == VID_NEED_STREAM);

	// Update LibAV context params for GetVideoTime()
	pCodecCtx->frame_number++;

	// Update LibAV context params for PutVideoFrame()
	if (decOut.outImgIdx >= 0) {
		pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
		pFrame->data[0] = (uint8_t*)decOut.outImg.luVirAddr;
		pFrame->data[1] = (uint8_t*)decOut.outImg.cbVirAddr;
		pFrame->data[2] = (uint8_t*)decOut.outImg.crVirAddr;
		pFrame->linesize[0] = decOut.outImg.luStride;
		pFrame->linesize[1] = decOut.outImg.cbStride;
		pFrame->linesize[2] = decOut.outImg.crStride;
		pFrame->pts = timeStamp;
		pFrame->key_frame = isKey;

		// Defer releasing buffer when multi-buffered display online
		NX_VidDecClrDspFlag( hDec, &decOut.outImg, decOut.outImgIdx );
	}
	else {
		pCodecCtx->pix_fmt = PIX_FMT_NONE;
	}

	return (vidret == VID_ERR_NONE);
}

bool CVPUPlayer::GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int iVideoStream, AVFrame *pFrame)
{
	if (hDec)
		return GetNextFrameHW(pFormatCtx, pCodecCtx, iVideoStream, pFrame);
	else
		return GetNextFrameSW(pFormatCtx, pCodecCtx, iVideoStream, pFrame);
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()	

#endif // USE_VPU

// EOF