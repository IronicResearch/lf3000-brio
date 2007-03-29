#ifndef LF_BRIO_BUTTONMPI_H
#define LF_BRIO_BUTTONMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventMgrMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Button Manager module. 
//
//==============================================================================

#include <CoreMPI.h>
#include <EventMessage.h>

//==============================================================================	   
// Button types
//==============================================================================
//------------------------------------------------------------------------------
struct tButtonData {
	U32	buttonState;
	U32	buttonTransition;
};

//------------------------------------------------------------------------------
class CButtonMessage : public IEventMessage {
public:
	CButtonMessage( const tButtonData& data );
	virtual U16	GetSizeInBytes() const;
	tButtonData GetButtonState() const;
private:
	tButtonData	mData;
};


//==============================================================================
class CButtonMPI : public ICoreMPI {
public:	
	// core functionality
	virtual	Boolean		IsValid() const;
	virtual tErrType	GetMPIVersion(tVersion &version) const;		   
	virtual tErrType	GetMPIName(ConstPtrCString &pName) const;		
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CButtonMPI();
	virtual ~CButtonMPI();

	// Get button state
	tErrType	GetButtonState(tButtonData& data);

private:
	class CButtonModule*	mpModule;
};


#endif // LF_BRIO_BUTTONMPI_H

// eof
