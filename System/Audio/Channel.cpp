//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Channel.cpp
//
// Description:
//		The class to manage the processing of audio data on an audio channel.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Channel.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CChannel implementation
//==============================================================================
#if 0
static void InitConversionRate(tAudioConversion *pRate, U16 inRateInHz, U16 outRateInHz);
static void ResetConversionRate(tAudioConversion *pRate);
static void ConvertData(tAudioConversion *pRate, U32 numSamples, S16 *pInBuf, S16 *pOutBuf);
#endif

CChannel::CChannel()
{
	// Initialize the class variables
//	mpChain_ = kNull;
	pPlayer_ = kNull;
	bInUse_ = 0;
	bPaused_ = 0;
	bOwnProcessor_ = 0;
	volume_ = 0;
	pan_ = 0;
	
	// Allocate the channel's output buffer
	pOutBuffer_ = new S16[ kAudioOutBufSizeInWords ];
}

//==============================================================================
//==============================================================================
CChannel::~CChannel()
{
	// Deallocate the channel buffers
	if (pOutBuffer_)
		delete pOutBuffer_;
}

//==============================================================================
//==============================================================================
tErrType CChannel::InitChanWithPlayer( CAudioPlayer* pPlayer )
{
	pPlayer_ = pPlayer;
	bInUse_ = 1;
	SetPan( pPlayer->GetPan() );
	SetVolume( pPlayer->GetVolume() );
//	printf("CChannel::InitChanWithPlayer - setting volume %f\n", volume_);
	inSampleRate_ = pPlayer_->GetSampleRate();
//	InitConversionRate(&convRate_, inSampleRate_, kAudioSampleRate);
	
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CChannel::Release()
{
//	mpChain_ = kNull;
	reinterpret_cast<CRawPlayer*>(pPlayer_)->SendDoneMsg();
	delete pPlayer_;
	pPlayer_ = kNull;
	bInUse_ = 0;
	bPaused_ = 0;
	bOwnProcessor_ = 0;
	pan_ = 0;
	volume_ = 0;
	inSampleRate_ = 0;

	return kNoErr;
}
/*
//==============================================================================
//==============================================================================
tErrType CChannel::KeepAlive(tAudioHeader *pHeader)
{
	mpHeader = pHeader;
	bInUse = 1;
	InitConversionRate( &mConvRate, mpHeader->sampleRateInHz, kAudioSampleRate );
	return kNoErr;
}
*/

//==============================================================================
//==============================================================================
U32 CChannel::RenderBuffer( S16 *pMixBuff, U32 numStereoFrames  )
{
	U32 playerFramesRendered;
	U32 numStereoSamples = numStereoFrames * kAudioBytesPerSample;
		
	// Initialize the output buffer to 0.  This has the side effect of zero
	// padding the output if the player runs out of data.
	bzero( pOutBuffer_, kAudioOutBufSizeInBytes );

	// Have player render its data into our output buffer.  If the player
	// contains mono data, it will be rendered out as stereo data.
	playerFramesRendered = pPlayer_->RenderBuffer( pOutBuffer_, numStereoFrames );
	
	// decide how to deal with player done i.e. playerFramesRendered comes back 
	// less than numStereoFrames: does player send done, or channel.
	
// fixme/dg:  needs CAudioEffectsProcessor
#if 0
	// If the channel has an effects processor, render the effect into the
	// pOutBuffer_, then sample rate convert after.
	if (pFxChain_ != kNull)
		pFxChain->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuffer_ );
#endif

	// First Check if conversion is necesssary
//		if ( kAudioSampleRate != inSampleRate_)
//	if ( 0 )
//	{
		// convert channel's output buffer and MIX(!) it to caller's buffer.
//		ConvertData(&convRate_, numStereoSamples, pOutBuffer_, pMixBuff);
//	}
	// Otherwise data is already at the correct sample rate
	// so just need to add it to the mix buffer
//	else
//	{
		S16	*pChanData = pOutBuffer_;
		float chanOutputSample;
		S32	sum;
//		bMoreData_ = 1;
		for (U32 i = 0; i < numStereoSamples; i++)
		{
			// Be sure the total sum stays within range
			chanOutputSample = (float)*pChanData++;
			chanOutputSample *= volume_; 			// Apply channel gain
			sum = *pMixBuff + (S16)chanOutputSample;			

			if (sum > kS16Max) sum = kS16Max;
			else if (sum < kS16Min) sum = kS16Min;
			
			*pMixBuff++ = (S16)sum;
		}
//	}
		
	return playerFramesRendered;
}

#if 0
//==============================================================================
//==============================================================================
static void InitConversionRate(tAudioConversion *pRate, U16 inRateInHz, U16 outRateInHz)
{
	U32 incr;

	pRate->iPos = 0;
	pRate->iLastLeft = 0;
	pRate->iLastRight = 0;
	pRate->oPosFrac = 0;
	pRate->oPos = 0;

	// Compute increment
	incr = (U32)((double)inRateInHz / (double)outRateInHz * 
					(double) ((U32) 1 << kFracBits));

	pRate->oPosIncFrac = incr & (((U32) 1 << kFracBits)-1);
	pRate->oPosInc = incr >> kFracBits;
}

//==============================================================================
//==============================================================================
static void ResetConversionRate(tAudioConversion *pRate)
{
	pRate->iPos = 0;
	pRate->iLastLeft = 0;
	pRate->iLastRight = 0;
	pRate->oPosFrac = 0;
	pRate->oPos = 0;
}

//==============================================================================
//==============================================================================
static void ConvertData(tAudioConversion *pRate, U32 numSamples, S16 *pInBuf, S16 *pOutBuf)
{
	S16		*iStart,*oStart;
	S16		iLastLeft, iLastRight;
	U32		tmp;
	S32		sum;
	double	t;

	iStart = (S16 *)&pInBuf[pRate->iPos << 1];
	oStart = (S16 *)pOutBuf;
	//NU_printf("conv:%d, x%x\n",pRate->iPos, iStart);					 

	// Loop through the data, computing 2 samples at a time (since stereo data)
	for (U16 i=0; i<numSamples; i+=2) 
	{
		// read as many input samples so that ipos > opos
		while (pRate->iPos <= pRate->oPos) 
		{
			iLastLeft = *iStart++;
			iLastRight = *iStart++;
			pRate->iPos++;
		}

		// interpolate both samples of the stereo data with the same factor
		// making sure the total sum stays within range
       t = (double) pRate->oPosFrac / ((U32) 1 << kFracBits);
       sum = (S32) (*oStart + (double) iLastLeft * (1.0 - t) + (double) (*iStart) * t);
		if (sum > kS16Max) sum = kS16Max;
		else if (sum < kS16Min) sum = kS16Min;
		*oStart++ = sum;
//		printf("converted sum left: %d ", sum);

       sum = (S32) (*oStart + (double) iLastRight * (1.0 - t) + (double) (*(iStart+1)) * t);
		if (sum > kS16Max) sum = kS16Max;
		else if (sum < kS16Min) sum = kS16Min;
		*oStart++ = sum;
//		printf("converted sum right: %d ", sum);

		//if (i < 5)
		//	NU_printf("%d, %d, %d, %d\n", iLastLeft, iLastRight, *(oStart-2),*(oStart-1));
       // increment position
		 tmp = pRate->oPosFrac + pRate->oPosIncFrac;
		 pRate->oPos = pRate->oPos + pRate->oPosInc + (tmp >> kFracBits);
		 pRate->oPosFrac = tmp & (((U32) 1 << kFracBits)-1);
	}
	pRate->iLastLeft = iLastLeft;
	pRate->iLastRight = iLastRight;
}
#endif
// EOF	
