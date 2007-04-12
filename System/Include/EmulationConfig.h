#ifndef LF_BRIO_EMULATIONCONFIG_H
#define LF_BRIO_EMULATIONCONFIG_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EmulationConfig.h
//
// Description:
//		Configure emulation for the environment. 
//
//==============================================================================

#include <SystemTypes.h>
#include <StringTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// EmulationConfig
//==============================================================================
class EmulationConfig
{
public:
	//--------------------------------------------------------------
	// Basic singleton interface
	//--------------------------------------------------------------
	static EmulationConfig& Instance( );

	//--------------------------------------------------------------
	// Emulation project setup
	//--------------------------------------------------------------
	bool		Initialize( const char* );
	void		SetModuleSearchPath( const char* );
	const char*	GetModuleSearchPath( ) const;

	//--------------------------------------------------------------
	// Frame buffer characteristics
	// SetLcdInitialSize(2.0) will display an LCD buffer size of 
	//		160x160 in a 320x320 window on your workstation's screen.
	//--------------------------------------------------------------
	void		SetLcdDisplayWindow( U32 hwnd );
	U32			GetLcdDisplayWindow( ) const;
	void		GetLcdFrameBufferSize( U16& width, U16& height ) const;
	void		SetLcdFrameBufferSize( U16 width, U16 height );

	//--------------------------------------------------------------
	// FlashPart setup
	//--------------------------------------------------------------
//			void		GetFlashPartConfig( U32 idFlash, U32& numSectors, U32& sectorSize );
//			void		SetFlashPartConfig( U32 idFlash, U32 numSectors, U32 sectorSize );

	//--------------------------------------------------------------
	// Memory allocation behaviour
	//--------------------------------------------------------------
//			bool		ContineOnMallocFails() const;
//			void		SetContineOnMallocFails( bool bContinue );

private:
	//--------------------------------------------------------------
	// Singleton implementation
	//--------------------------------------------------------------
	EmulationConfig( );
	EmulationConfig( const EmulationConfig& );
	EmulationConfig& operator=( const EmulationConfig& );

	//--------------------------------------------------------------
	// Implementation member vars
	//--------------------------------------------------------------
};
		

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_EMULATIONCONFIG_H

// eof
