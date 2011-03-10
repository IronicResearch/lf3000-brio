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
#include <unistd.h>
#include <boost/scoped_array.hpp>
#include <string.h>
#include <DebugMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

const size_t	kMaxPath = 2048;
const CPath kPackedRsrcFolder("/rsrc");


//==============================================================================
namespace
{
	CPath	gModuleSearchPath;
	CPath	gBaseResourceSearchPath;
	CPath	gCartResourceSearchPath;
	U32		gWindow = 0;
	U32		gLcdFrameBufferWidth = 320;
	U32		gLcdFrameBufferHeight = 240;
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
	// For normal (non-unit test) development, the client app needs
	// to call Initialize() with the root of the installed SDK
	// C++ development kit.
	// The module search path and BaseROM assets path are based off
	// of the C++ development kit root folder.
	//
	// If SDK path not specified, use default emulation path
	if (strstr(pathIn,"emuroot")) {
		CPath emuroot = CPath(pathIn);
		CPath modpath = emuroot + "/LF/Base/Brio/Module";
		CPath basepath = emuroot + "/LF/Base";
		CPath cartpath = emuroot + "/LF/Cart";
		SetModuleSearchPath(modpath.c_str());
		SetBaseResourceSearchPath(basepath.c_str());
		SetCartResourceSearchPath(cartpath.c_str());
		return true;
	}
	
	CPath path = AppendPathSeparator(pathIn);
	path += "Libs/Lightning_emulation/Module";
	SetModuleSearchPath(path.c_str());
	
	path = AppendPathSeparator(pathIn);
	path += "BaseROM/rsrc";
	SetBaseResourceSearchPath(path.c_str());
	
	boost::scoped_array<char> buf(new char[kMaxPath]);
	memset(buf.get(), 0, kMaxPath);
    if (readlink("/proc/self/exe", buf.get(), kMaxPath) != -1)
	{
		CPath temp(buf.get());
		temp = temp.substr(0, temp.rfind('/'));
		temp = temp.substr(0, temp.rfind('/'));
		temp = temp + kPackedRsrcFolder;
		SetCartResourceSearchPath(temp.c_str());
	}
	return true;
}

//----------------------------------------------------------------------
void EmulationConfig::SetModuleSearchPath( const char* path )
{
	gModuleSearchPath = AppendPathSeparator(path);
}

//----------------------------------------------------------------------
const char* EmulationConfig::GetModuleSearchPath( ) const
{
	return gModuleSearchPath.c_str();
}

//----------------------------------------------------------------------
void EmulationConfig::SetBaseResourceSearchPath( const char* path )
{
	gBaseResourceSearchPath = AppendPathSeparator(path);
}

//----------------------------------------------------------------------
const char* EmulationConfig::GetBaseResourceSearchPath( ) const
{
	return gBaseResourceSearchPath.c_str();
}

//----------------------------------------------------------------------
void EmulationConfig::SetCartResourceSearchPath( const char* path )
{
	gCartResourceSearchPath = AppendPathSeparator(path);
}

//----------------------------------------------------------------------
const char* EmulationConfig::GetCartResourceSearchPath( ) const
{
	return gCartResourceSearchPath.c_str();
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
//----------------------------------------------------------------------
void EmulationConfig::GetLcdFrameBufferSize( U16& width, U16& height ) const
{
	width = gLcdFrameBufferWidth;
	height = gLcdFrameBufferHeight;
}
//----------------------------------------------------------------------
void EmulationConfig::SetLcdFrameBufferSize( U16 width, U16 height )
{
	CDebugMPI debug_mpi(kGroupHardware);
	debug_mpi.DebugOut(kDbgLvlCritical, "EmulationConfig::SetLcdFrameBufferSize width=%d, height=%d\n", width, height);
	gLcdFrameBufferWidth = width;
	gLcdFrameBufferHeight = height;
}
LF_END_BRIO_NAMESPACE()

// eof
