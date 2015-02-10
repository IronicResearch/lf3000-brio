//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioOutput.c
//
// Description:
//		Implements audio output on top of PortAudio.
//
//	  03/06/07	rdg	 PortAudio version for Lightning "Emulation"
//	  06/26/06	ytu	 Initial version
//==============================================================================
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <CoreTypes.h>

#include <AudioConfig.h>
#include <AudioOutput.h>

#include <DebugMPI.h>
#include <DebugTypes.h>
#include <GroupEnumeration.h>
#include <KernelMPI.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <linux/soundcard.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Global variables
//==============================================================================
 
//==============================================================================
// InitAudioOutput	
//==============================================================================
int InitAudioOutput( BrioAudioRenderCallback* callback, void* pUserData )
{	
	return InitAudioOutputAlsa(callback, pUserData);
}

// ==============================================================================
// StartAudioOutput	 
// ==============================================================================
int StartAudioOutput( void )
{
	return StartAudioOutputAlsa();
}

// ==============================================================================
// StopAudioOutput	
// ==============================================================================
int StopAudioOutput( void )
{
	return StopAudioOutputAlsa();
}

//==============================================================================
// DeInitAudioOutput  
//==============================================================================
int DeInitAudioOutput( void )
{
	return DeInitAudioOutputAlsa();
}

LF_END_BRIO_NAMESPACE()
// EOF
