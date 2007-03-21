#ifndef FLASHLITE2_0MPI_H
#define FLASHLITE2_0MPI_H

//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		FlashLite20MPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System FlashLite2.0 module. 
//
//==============================================================================

#include "CoreMPI.h"

class CFlashLite20MPI : public ICoreMPI
{
public:
	// core functionality
	virtual tErrType	Init();			   
	virtual tErrType	DeInit(); 			
	virtual Boolean		IsInited();

	virtual tErrType	GetMPIVersion(tVersion *pVersion);		   
	virtual tErrType	GetMPIName(const CString **ppName);		

	virtual tErrType	GetModuleVersion(tVersion *pVersion);
	virtual tErrType	GetModuleName(const CString **ppName);	
	virtual tErrType	GetModuleOrigin(const CURI **ppURI);

	// class-specific functionality
	CFlashLite20MPI();
	virtual ~CFlashLite20MPI();


	tErrType			CreatePlayer(tFlashCreationFlags *pFlags);
	tErrType			DestroyPlayer();
	tErrType 			PlayMovie(tRsrcHndl nMovie);
	tErrType			GetState(tFlashMovieState *pFlashState);
	tErrType			PauseMovie();
	tErrType			SetFrameRate(U16 nFrameRate);
	U16					GetFrameRate(void);
	tErrType			SetFrameNumber(U16 nFrameNumber);
	U16					GetFrameNumber(void);
	tErrType			SetQuality(U16 nQuality);
	U16					GetQuality(void);
	tErrType			SetLooping(Boolean bLooping);
	Boolean 			GetLooping(void);

private:
	class CFlashLite20MPIImpl *mpImpl;
};


#endif // FLASHLITE2_0MPI_H

// eof
