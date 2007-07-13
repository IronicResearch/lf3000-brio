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
//		In the future, we may want to add an accessor that gets the underlying
//		path name, etc.
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


LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_SYSTEMRESOURCEMPI_H

// eof
