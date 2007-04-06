#ifndef LF_BRIO_AUDIOMPI_H
#define LF_BRIO_AUDIOMPI_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Audio module. 
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <ResourceTypes.h>
//#include <AudioTypes.h>
#include <CoreMPI.h>
//#include <EventListener.h>

class IEventListener;


//==============================================================================
// Class:
//		CAudioMgrMPI
//
// Description:
//		Module Public Interface (MPI) class for the Audio Manager module. 
//==============================================================================
class CAudioMPI : public ICoreMPI {
public:
	// MPI core functions
	virtual	Boolean		IsValid() const;	
	
	virtual	tErrType	GetMPIVersion(tVersion &version) const;		   
	virtual	tErrType	GetMPIName(ConstPtrCString &pName) const;		

	virtual	tErrType	GetModuleVersion(tVersion &version) const;
	virtual	tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual	tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;


	// class-specific functionality
	CAudioMPI( const IEventListener* pDefaultListener = NULL );
	virtual ~CAudioMPI();

	//********************************
	// Overall audio control 
	//********************************
	tErrType	StartAudio( void );
	tErrType	StopAudio( void );
	tErrType	PauseAudio( void );
	tErrType	ResumeAudio( void );
	
private:
	class CAudioModule*	mpModule;
};

#endif /* LF_BRIO_AUDIOMPI_H */

// EOF
