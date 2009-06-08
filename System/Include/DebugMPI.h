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
//		Defines the Module Public Interface (MPI) for debugging and logging.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <DebugTypes.h>
#include <CoreMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

const char* const ErrToStr( tErrType error );

/// \class CDebugMPI
///
///   The debug interface is designed to provide unified interfaces for accessing console output and
///   "Flight Recorder". 
///
///   The debug interface will generate both console output and "Flight Recorder" logging when debug level
///   is set properly.
/// 
///   Any module or application should go through these interfaces for accessing console output and 
///   logging into "Flight Recorder"
///
///  "Console output" is a the result of printf statement.
///
///   The level of "console output" can be controlled through
///		SetDebugLevel( tDebugLevel newLevel );
///   The API set the console output level for the particular instance of the CDebugMPI object, other instances
///   of the CDebugMPI object remains unchanged.
///
///   The default level for "console output" is
///			 kDbgLvlValuable
///   Please note: console output level has no impact on "Flight Recorder"
///
///
///   "Flight Recorder" is a persistent storage facility for logging important events, warnings, and errors.
///   The following are examples that should log into "Flight Recorder"
///		* File open errors
///		* Profile creation and deletion
/// 		* Running out of memeory
/// 		* Low battery events.
///   The Flight Recorder logging level is controlled through:
///		SetMasterDebugLevel(tDebugLevel newLevel);
///   The API is meant for "system monitor deamon" to control the overall Flight Recorder level, any individual
///   module or game should NOT try to set the master debug level.
///
///   The default level for Flight Recorder is
///			 kDbgLvlValuable
///
///   Each module that wishes to use the debug interface should identify itself with an unique "signature"
///   The signature allows the module message to be easily distinguishable in "Flight Recorder"
///   The signature is passed in through constructor
///
///   It is not recommended to use the debug interface to print out high frequent messages.
///   If you have to log repeatitive messages, try to use the level of kDbgLvlVerbose.
///
///
//==============================================================================
class CDebugMPI  : public ICoreMPI{
public:
	// ICoreMPI functionality, retain only for binary compatible with Didj Game

	/// Description: This function is obsolete, it is provided for compatibility
	///  with legacy C++ Games
	virtual	Boolean			IsValid() const;
	/// Description: This function is obsolete, it is provided for compatibility
	///  with legacy C++ Games
	virtual const CString*	GetMPIName() const;
	/// Description: This function is obsolete, it is provided for compatibility
	///  with legacy C++ Games
	virtual tVersion		GetModuleVersion() const;
	/// Description: This function is obsolete, it is provided for compatibility
	///  with legacy C++ Games
	virtual const CString*	GetModuleName() const;
	/// Description: This function is obsolete, it is provided for compatibility
	///  with legacy C++ Games
	virtual const CURI*		GetModuleOrigin() const;

	explicit CDebugMPI(tDebugSignature sig);
	virtual ~CDebugMPI();

	/// Description: printf-like debug output. The signature value is prepended in the output to allow easy
	/// debug identification.
	/// \param level		the debug level associated with this output.
	/// \param formatString	format string used like printf
	/// \param  ... 	variable arguments
	/// \return: none
	void 	DebugOut(tDebugLevel level, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));
	/// Description: vprintf-like debug output. The signature value is prepended in the output to allow easy
	/// debug identification.
	/// \param level		the debug level associated with this output.
	/// \param formatString	format string used like printf
	/// \param  arguments 	variable arguments obtained through va_start(...)
	/// \return: none
	void		VDebugOut(tDebugLevel level, const char * formatString, va_list arguments) const
				__attribute__ ((format (printf, 3, 0)));
	/// Description: printf-like debug output. This is used to output error messages.
	/// \param level		the debug level associated with this output.
	/// \param formatString	format string used like printf
	/// \param  ...		 	variable arguments
	/// \return: none
	void 	DebugOutErr(tDebugLevel level, tErrType err, const char * formatString, ...) const
				__attribute__ ((format (printf, 4, 5)));

	/// Description: This interface is provided for backward compatibility only.
	/// Use DebugOut(...) interface instead.
	void	DebugOutLiteral(tDebugLevel lvl, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));
	/// Description: This interface is provided for backward compatibility only.
	/// Use VDebugOut(...) interface instead.
	void	VDebugOutLiteral(tDebugLevel lvl, const char * formatString, va_list arguments) const
				__attribute__ ((format (printf, 3, 0)));

	/// Description: issue a warning that a critical, but recoverable, error occurred.
	/// \param formatString	format string used like printf
	/// \param  ... 	variable arguments
	/// \return: none
	void		Warn(const char * formatString, ...) const
				__attribute__ ((format (printf, 2, 3)));

	/// Description: Assert is the most critical message.  In Emerald, this interface does NOT
	/// cause application to exit.
	/// \param formatString	format string used like printf
	/// \param  ... 	variable arguments
	/// \return: none
	void 	Assert(int testResult, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));
	/// Description: Assert is the most critical message.  In Emerald, this interface does NOT
	/// cause application to exit.
	/// \param err	system error which will be translated to a string during output
	/// \param formatString	format string used like printf
	/// \param  ... 	variable arguments
	/// \return: none
	void 	AssertNoErr(tErrType err, const char * formatString, ...) const
				__attribute__ ((format (printf, 3, 4)));

	/// Description: This function is obsolete and does nothing, it is provide for compatibility
	///  with legacy C++ Games. 
	void 	DisableDebugOut(tDebugSignature sig);
	/// Description: This function is obsolete and does nothing, it is provide for compatibility
	///  with legacy C++ Games. 
 	void 	EnableDebugOut(tDebugSignature sig);

	/// Description: checks if debug out is enabled for a specific signature & level.  Helpful for
	/// checking in advance if anything will go out to console or "Flight Recorder"
	/// \param sig	signature for the debug module
	/// \param atLeastLevel	debug level to check
	/// \return: Boolean true (enabled), false(disabled)
	Boolean	DebugOutIsEnabled(tDebugSignature sig, tDebugLevel atLeastLevel=kDbgLvlCritical) const;

	/// Description: set master debug level which impact all the debug interface instances.
	/// Individual module should not use this function, this function is used by some system tools.
	/// \param newLevel	the new debug level
	/// \return: none
	void		SetMasterDebugLevel(tDebugLevel newLevel);
	
	/// Description: set master debug level which impact all the debug interface instances.
	/// \param none
	/// \return: tDebugLevel
	tDebugLevel	GetMasterDebugLevel() const;

	/// Description: adds a timestamp to all debug output. time shown is elapsed time since system boot.
	/// \param none
	/// \return: none
	void 	EnableDebugOutTimestamp();
	/// Description: disable timestamp to all debug output. time shown is elapsed time since system boot.
	/// \param none
	/// \return: none
	void 	DisableDebugOutTimestamp();
	/// Description: check if timestamp is enabled in debug output
	/// \param none
	/// \return: none
	Boolean   TimestampIsEnabled() const;

	//------------------------------------------------------------------------------
	// Function:	 	EnableThrowOnAssert
	//				 	DisableThrowOnAssert
	// Description:		if enabled, Assert() calls throw a UnitTestAssertException
	//					object instead of performing an assert() and halting the
	//					system.  Useful for unit testing.
	//------------------------------------------------------------------------------
	void 	EnableThrowOnAssert();
	void 	DisableThrowOnAssert();
	Boolean 	ThrowOnAssertIsEnabled() const;

	/// Description: set debug level for the instance object.
	/// \param newLevel:	the new debug level
	/// \return: none
	void 	SetDebugLevel( tDebugLevel newLevel );

	/// Description: get debug level for the instance object.
	/// \param none
	/// \return: tDebugLevel
	tDebugLevel GetDebugLevel() const;

private:
	class CDebugModule* pModule_;
	tDebugSignature		sig_;
	tDebugLevel			localDebugLevel_;

	static  tDebugLevel		masterDebugLevel_ ;
	static  Boolean		timestampDebugOut_;
	static  Boolean		throwOnAssert_ ;

	// Disable copy semantics
	CDebugMPI(const CDebugMPI&);
	CDebugMPI& operator=(const CDebugMPI&);
};

#define EmeraldAssert__(dbg, cond, des) \
do{ \
	dbg.Assert(cond, "assert: %s @ line %d (" #cond ") %s \n", __FILE__, __LINE__, des); \
}while(0)


#define EmeraldAssert(SIG, cond, des ) \
do{ \
	CDebugMPI dbg(SIG); \
	EmeraldAssert__(dbg, cond, des); \
}while(0)

#define EmeraldFlashAssert( cond )  \
do { \
	LeapFrog::Brio::CDebugMPI dbg(kFlashLiteDebugSig); \
	dbg.Assert((int)cond, "assert: %s @ line %d (" #cond ") \n", __FILE__, __LINE__); \
 }while(0)

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_DEBUGMPI_H

// EOF
