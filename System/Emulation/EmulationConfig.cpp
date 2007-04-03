//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EmulationConfig.cpp
//
// Description:
//		Configure emulation for the environment
//
//==============================================================================

#include <SystemErrors.h>
#include <EmulationConfig.h>

//==============================================================================
namespace
{
	CPath	gModuleSearchPath;
}


//==============================================================================
namespace LeapFrog
{
	namespace Brio
	{
		//----------------------------------------------------------------------
		EmulationConfig::EmulationConfig( )
		{
		}

		//----------------------------------------------------------------------
		EmulationConfig& EmulationConfig::Instance( )
		{
			static EmulationConfig minst;
			return minst;
		}

		//----------------------------------------------------------------------
		void EmulationConfig::SetModuleSearchPath( const CPath& path )
		{
			gModuleSearchPath = path;
		}

		//----------------------------------------------------------------------
		CPath EmulationConfig::GetModuleSearchPath( ) const
		{
			return gModuleSearchPath;
		}
		
	}	// end of Brio namespace
}		// end of LeapFrog namespace

// eof
