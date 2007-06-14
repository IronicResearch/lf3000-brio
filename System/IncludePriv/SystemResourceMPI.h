#ifndef LF_BRIO_SYSTEMRESOURCEMPI_H
#define LF_BRIO_SYSTEMRESOURCEMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		SystemResourceMPI.h
//
// Description:
//		Extends the ResourceMPI to give Brio system modules special powers!
//		(Namely, access to all opened devices, packages and resources).
//
//==============================================================================

#include <ResourceMPI.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// CSystemResourceMPI
//==============================================================================
class CSystemResourceMPI : public CResourceMPI {
public:	
	// ICoreMPI functionality
	virtual const CString*	GetMPIName() const;		

	// class-specific functionality
	CSystemResourceMPI(eSynchState block = kBlocking,
				const IEventListener *pListener = NULL);
};


#endif // LF_BRIO_SYSTEMRESOURCEMPI_H

// eof
