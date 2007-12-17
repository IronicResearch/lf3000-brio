#ifndef LF_BRIO_DEBUGMPI_H
#define LF_BRIO_DEBUGMPI_H
//==============================================================================
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DebugMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Debug module. 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <DebugTypes.h>
#include <CoreMPI.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Class:
//		CDebugMPI
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Debug module.
//==============================================================================
class CDebugMPI : public ICoreMPI {
public:
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;
	
	explicit CDebugMPI(tDebugSignature sig);  
	virtual ~CDebugMPI();
	
	// printf-like serial output (suppressible based on tDebugSignature). The signature
	// value is prepended in the output to allow easy debug identification.  These are
	// the primary calls to use for output.
	// Example: "This is my text output" -> [3] This is my text output 
	void 	DebugOut(tDebugLevel lvl, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));
	void	VDebugOut(tDebugLevel lvl, const char * formatString, va_list arguments) const
				__attribute__ ((format (printf, 3, 0)));
	void 	DebugOutErr(tDebugLevel lvl, tErrType err, const char * formatString, ...) const
				__attribute__ ((format (printf, 4, 5)));

	// same as above, but without anything prepended or postfixed in the output.  This is useful
	// when printing a sequence of output that needs to be cleanly formatted (tables, etc).
	// Example:  "This is my text output" -> This is my text output
	void	DebugOutLiteral(tDebugLevel lvl, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));
	void	VDebugOutLiteral(tDebugLevel lvl, const char * formatString, va_list arguments) const
				__attribute__ ((format (printf, 3, 0)));

	// issue a warning that a critical, but recoverable, error occurred.  This
	// is just one step below Asserting.  The warning message is printed
	// on the screen, &/or out the serial port, and could be accompanied by
	// an audio alert to indicate a critical error occurred.
	// (suppressible based on tDebugSignature)
	void	Warn(const char * formatString, ...) const
				__attribute__ ((format (printf, 2, 3)));

	// Assert
	void 	Assert(int testResult, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));
	void 	AssertNoErr(tErrType err, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));
							
	// turning on & off debug-out on a debug-signature basis
	void 	DisableDebugOut(tDebugSignature sig);
 	void 	EnableDebugOut(tDebugSignature sig);
 
	// checks if debug out is enabled for a specific signature & level.  Helpful for
	// checking in advance if anything will go out the serial port before issuing a
	// series of DebugOuts.  'atLeastLevel' indicates output enabled for levels
	// >= that specified--default checks if any output enabled for the 'sig'.
	Boolean		DebugOutIsEnabled(tDebugSignature sig, tDebugLevel atLeastLevel=kDbgLvlCritical) const;

	// accessing the master DebugOut level
// BSK /121107
//	void		SetDebugLevel(tDebugLevel newLevel);
//	tDebugLevel	GetDebugLevel() const;

	//------------------------------------------------------------------------------
	// Function:	 	EnableDebugOutTimestamp
	//					DisableDebugOutTimestamp
	// Description:		if enabled, adds a timestamp to all debug output.  Time
	//					shown is elapsed time since system boot.
	//------------------------------------------------------------------------------
	void 	EnableDebugOutTimestamp();
	void 	DisableDebugOutTimestamp();

	//------------------------------------------------------------------------------
	// Function:	 	EnableThrowOnAssert
	//				 	DisableThrowOnAssert
	// Description:		if enabled, Assert() calls throw a UnitTestAssertException
	//					object instead of performing an assert() and halting the 
	//					system.  Useful for unit testing.
	//------------------------------------------------------------------------------
	void 	EnableThrowOnAssert();
	void 	DisableThrowOnAssert();

	//------------------------------------------------------------------------------
	// Function:	 	SetDebugLevel
	//				 	GetDebugLevel
	//                  
	// Description:		setter and getter for 
	//					masterDebugLevel_ 
	//					They are the friend functions of DebugPriv.h class
	//------------------------------------------------------------------------------
	
	void SetDebugLevel( tDebugLevel newLevel );
	tDebugLevel GetDebugLevel() const;
	Boolean DebugOutIsEnabled( tDebugLevel lvl ) const;
	
private:
	class CDebugModule* pModule_;
	tDebugSignature		sig_;
	tDebugLevel			masterDebugLevel_;
	// Disable copy semantics
	CDebugMPI(const CDebugMPI&);
	CDebugMPI& operator=(const CDebugMPI&);
};
		   
LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DEBUGMPI_H

// EOF
