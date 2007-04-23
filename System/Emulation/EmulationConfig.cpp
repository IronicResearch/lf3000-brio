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

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <EmulationConfig.h>

LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
namespace
{
	CPath	gModuleSearchPath;
	U32		gWindow;
}


//==============================================================================
//----------------------------------------------------------------------
EmulationConfig::EmulationConfig( )
{
	gWindow = 0;
}

//----------------------------------------------------------------------
EmulationConfig& EmulationConfig::Instance( )
{
	static EmulationConfig minst;
	return minst;
}

//----------------------------------------------------------------------
bool EmulationConfig::Initialize( const char* pathIn )
{
	CPath path = pathIn;
	path += "/Libs/LightningGCC_emulation/Module";
	SetModuleSearchPath(path.c_str());
	return true;
}

//----------------------------------------------------------------------
void EmulationConfig::SetModuleSearchPath( const char* path )
{
	gModuleSearchPath = path;
}

//----------------------------------------------------------------------
const char* EmulationConfig::GetModuleSearchPath( ) const
{
	return gModuleSearchPath.c_str();
}

//----------------------------------------------------------------------
void EmulationConfig::SetLcdDisplayWindow( U32 hwnd )
{
	gWindow = hwnd;
}

//----------------------------------------------------------------------
U32 EmulationConfig::GetLcdDisplayWindow( ) const
{
	return gWindow;
}

LF_END_BRIO_NAMESPACE()

// eof
