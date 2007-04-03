//==============================================================================
//
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		DebugMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the System Debug 
//		module.
//
//==============================================================================

//==============================================================================

// std includes

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>		// for vprintfs

// system includes
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <StringTypes.h>

#include <Module.h>

#include <DebugMPI.h>

// Module includes
#include <DebugPriv.h>

const CString kMPIName = "DebugMPI";
const tVersion  kMPIVersion = MakeVersion(0,1);


//==============================================================================
//----------------------------------------------------------------------------
CDebugMPI::CDebugMPI(): mpModule(NULL)
{
	ICoreModule*	pModule;
	Module::Connect( pModule, kDebugModuleName, kDebugModuleVersion );
	mpModule = reinterpret_cast<CDebugModule*>(pModule);
}

//----------------------------------------------------------------------------
CDebugMPI::~CDebugMPI( void )
{
	Module::Disconnect( mpModule );
}

//==============================================================================
//----------------------------------------------------------------------------
Boolean CDebugMPI::IsValid() const
{
	return (mpModule != NULL) ? true : false; 
}

//----------------------------------------------------------------------------
tErrType	CDebugMPI::GetMPIVersion( tVersion &version )  const
{
	version = kMPIVersion;
	return kNoErr; 
}

//----------------------------------------------------------------------------
tErrType	CDebugMPI::GetMPIName( ConstPtrCString &pName ) const
{
	pName = &kMPIName;
	return kNoErr; 
}	

//----------------------------------------------------------------------------
tErrType	CDebugMPI::GetModuleVersion( tVersion &version ) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;

	return mpModule->GetModuleVersion( version );
}
	
//----------------------------------------------------------------------------
tErrType	CDebugMPI::GetModuleName( ConstPtrCString &pName ) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;

	return mpModule->GetModuleName( pName );
}
 
//----------------------------------------------------------------------------
tErrType	CDebugMPI::GetModuleOrigin( ConstPtrCURI &pURI ) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleOrigin( pURI );
}	

//==============================================================================
//----------------------------------------------------------------------------
void 	CDebugMPI::DebugOut( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, ... )
{
	va_list arguments;

	if (mpModule)
	{
		va_start( arguments, formatString );
		mpModule->VDebugOut( sig, lvl, formatString, arguments );
		va_end( arguments );
	}
}

//----------------------------------------------------------------------------
void 	CDebugMPI::VDebugOut( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments )
{
	if (mpModule)
	{
		mpModule->VDebugOut( sig, lvl, formatString, arguments );
	}
}

//----------------------------------------------------------------------------
void 	CDebugMPI::DebugOutLiteral( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, ... )
{
	va_list arguments;

	if (mpModule)
	{
		va_start( arguments, formatString );
		mpModule->VDebugOut( sig, lvl, formatString, arguments );
		va_end( arguments );
	}
}

//----------------------------------------------------------------------------
void 	CDebugMPI::VDebugOutLiteral( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments )
{
	if (mpModule)
	{
		mpModule->VDebugOut( sig, lvl, formatString, arguments );
	}
}

//----------------------------------------------------------------------------
void 	CDebugMPI::Warn( tDebugSignature sig, const char * formatString, ... )
{
	va_list arguments;

	if (mpModule)
	{
		va_start( arguments, formatString );
		mpModule->Warn( sig, formatString, arguments );
		va_end( arguments );
	}
}
						
//----------------------------------------------------------------------------
void 	CDebugMPI::Assert( int testResult, const char * formatString, ... )
{
	if (!testResult && mpModule)
	{
		va_list arguments;
		va_start( arguments, formatString );
		mpModule->Assert( formatString, arguments );
		va_end( arguments );
	}
}
			
//==============================================================================
// Function:
//		DisableDebugOut
//
// Parameters:
//		tDebugSignature sig -- The target signature for which debug output
//			is to be disabled.
//
// Returns:
//		N/A
//
// Description:
//		Allows code to disable debug output for a specific signature so that
//		a user is not necessarily required to get the desired configuration
//		through a series of serial commands each time they power the system.   
//==============================================================================
void CDebugMPI::DisableDebugOut( tDebugSignature sig )
{
	if (mpModule)
		mpModule->DisableDebugOut( sig );
}

//==============================================================================
// Function:
//		DisableDebugOut
//
// Parameters:
//		tDebugSignature sig -- The target signature for which debug output
//			is to be re-enabled.
//
// Returns:
//		N/A
//
// Description:
//		Allows code to re-enable debug output for a specific signature so that
//		a user is not necessarily required to get the desired configuration
//		through a series of serial commands each time they power the system.   
//==============================================================================
void CDebugMPI::EnableDebugOut( tDebugSignature sig )
{
	if (mpModule)
		mpModule->EnableDebugOut( sig );
}

//==============================================================================
// Function:
//		DebugOutIsEnabled
//
// Parameters:
//		tDebugSignature sig -- The target signature
//		tDebugLevel	level	
//
// Returns:
//		Boolean 
//
// Description:
//		Checks if a DebugOut command with this level or higher is issued,
//		whether it will go out the serial port.   
//==============================================================================
Boolean CDebugMPI::DebugOutIsEnabled( tDebugSignature sig, tDebugLevel level )
{
	if (mpModule)
		return mpModule->DebugOutIsEnabled( sig, level );
}

//==============================================================================
// Function:
//		SetDebugLevel
//		GetDebugLevel
//
// Parameters:
//		various
//
// Returns:
//		various
//
// Description:
//		Gets & sets the master debug level for all DebugOut.   
//==============================================================================

void CDebugMPI::SetDebugLevel( tDebugLevel newLevel )
{
	if (mpModule)
		mpModule->SetDebugLevel( newLevel );
}

tDebugLevel CDebugMPI::GetDebugLevel()
{
	if (mpModule)
		return mpModule->GetDebugLevel();
}

//==============================================================================
// Function:
//		EnableDebugOutTimestamp
//		DisableDebugOutTimestamp
//
// Parameters:
//		none
//
// Returns:
//		none
//
// Description:
//		Enable/disable adding timestamp to all debug output.   
//==============================================================================

void CDebugMPI::EnableDebugOutTimestamp()
{
	if (mpModule)
		mpModule->EnableDebugOutTimestamp();
}

void CDebugMPI::DisableDebugOutTimestamp()
{
	if (mpModule)
		mpModule->DisableDebugOutTimestamp();
}

// EOF
