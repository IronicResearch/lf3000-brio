#ifndef LF_BRIO_AUDIOMPI_H
#define LF_BRIO_AUDIOMPI_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
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
#include <SystemTypes.h>
#include <ResourceTypes.h>
//#include <AudioTypes.h>
#include <CoreMPI.h>
//#include <EventListener.h>
LF_BEGIN_BRIO_NAMESPACE()

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
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

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
	class CAudioModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif /* LF_BRIO_AUDIOMPI_H */

// EOF
