#ifndef LF_BRIO_AUDIOOUTPUT_H
#define LF_BRIO_AUDIOOUTPUT_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.

//==============================================================================
//
// File:
//		AudioOutput.h
//
// Description:
//		Defines the audio output.
//
//==============================================================================
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

int InitAudioOutput( void );
int DeInitAudioOutput( void );
int StartAudioOutput( void );
int StopAudioOutput( void );

#ifdef __cplusplus
}
#endif /* __cplusplus */

LF_END_BRIO_NAMESPACE()	
#endif /*LF_BRIO_AUDIOOUTPUT_H*/
