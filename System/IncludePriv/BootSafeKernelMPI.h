#ifndef LF_BRIO_BOOTSAFEKERNELMPI_H
#define LF_BRIO_BOOTSAFEKERNELMPI_H
//==============================================================================//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		BootSafeKernelMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Kernel module. 
//
//==============================================================================

#include <vector>
#include <SystemTypes.h>
#include <CoreMPI.h>
#include <KernelTypes.h>
#include <StringTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


class CBootSafeKernelMPI : public ICoreMPI
{
public:
	// ICoreMPI functionality
	virtual	Boolean		IsValid() const;
	virtual const CString*	GetMPIName() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
	virtual const CURI*	GetModuleOrigin() const;
    
	// class-specific functionality
	CBootSafeKernelMPI();

	//==============================================================================
	// Tasks
	//==============================================================================
	
	//==============================================================================
	// Memory Allocation
	//==============================================================================
	tPtr		Malloc( U32 size );
	void		Free( tPtr pAllocatedMemory );


	//==============================================================================
	// Message Queues
	//==============================================================================
				
	//==============================================================================
	// Time & TimerstTimerGenHndl
	//==============================================================================

	//==============================================================================
	// Mutexes
	//==============================================================================
    
	//==============================================================================
	// Conditions
	//==============================================================================

	//==============================================================================
	// I/O Functions
	//==============================================================================
	// Standard printf syntax to output a string through the serial port
	void	Printf( const char * formatString, ... ) const
						__attribute__ ((format (printf, 2, 3)));

	// Standard vprintf syntax to output a string through the serial port
	void	VPrintf( const char * formatString, va_list arguments ) const
						__attribute__ ((format (printf, 2, 0)));
   
	// Logging functions
	void	Logging( const char * ident, int priority, const char * formatString, ... ) const
						__attribute__ ((format (printf, 4, 5)));

	void	VLogging( const char * ident, int priority, const char * formatString, va_list arguments ) const
						__attribute__ ((format (printf, 4, 0)));

	//==============================================================================
	// Power Control Functions
	//==============================================================================
	void	PowerDown() const;
    
	//==============================================================================
	// File System Functions
	//==============================================================================
	std::vector<CPath>		GetFilesInDirectory( const CPath& dir ) const;
    
	Boolean				IsDirectory( const CPath& dir ) const;
    
	//==============================================================================
	// Code Module Functions
	//==============================================================================
	tHndl	LoadModule( const CPath& dir ) const;
	void*	RetrieveSymbolFromModule( tHndl obj, const CString& symbol ) const;
	void		UnloadModule( tHndl obj ) const;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BOOTSAFEKERNELMPI_H

// EOF

