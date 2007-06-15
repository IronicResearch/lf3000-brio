#ifndef LF_BRIO_UNITTESTUTILS_H
#define LF_BRIO_UNITTESTUTILS_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		UnitTestUtils.h
//
// Description:
//		Base class for unit tests to do common initialization
//
//==============================================================================

#include <SystemTypes.h>
#include <StringTypes.h>
#include <EmulationConfig.h>
#include <DebugMPI.h>
#include <sys/stat.h>

LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class TestSuiteBase
{
protected:
	TestSuiteBase()
	{
		// 1) Do a dummy initialize to get the CartResourcePath set relative to
		//		the running EXE.
		// 2) The module search path will always be ../Lightning_emulation/Module from the
		//		Cart resource path, so set that.
		// 3) "emulation" unit test EXEs are in /Brio2/Build/Lightning_emulation/MPI
		//	  "publish" ones are in /Brio2/Publish_XXX/Libs/Lightning_emulation/MPI
		//	  NOTE: These two paths have different depths!
		//	  We need to reset the CartResourceSearchPath to /Brio2/UnitTestData,
		//    so search for a literal string names to find the Brio2 root
		// 4) Allow unit test to test Assert conditions without halting the testing.
		// 5) Minimize expected debug out "chatter" for critical and assert warnings
		//	  by setting the debug level to "silent".
		//
#ifdef EMULATION
		EmulationConfig::Instance().Initialize("");						//*1						
		CPath path = EmulationConfig::Instance().GetCartResourceSearchPath();
		
		path = path.substr(0, path.rfind('/'));							//*2
		path = path.substr(0, path.rfind('/'));
		CPath modSearch = path + "/Lightning_emulation/Module";
		EmulationConfig::Instance().SetModuleSearchPath(modSearch.c_str());
		
		size_t idx = path.find("/Brio2/");								//*3
		path = path.substr(0, idx + 7) + "UnitTestData/";
		EmulationConfig::Instance().SetCartResourceSearchPath(path.c_str());
#endif
		CDebugMPI	dbg(kGroupUnitTests);
		dbg.EnableThrowOnAssert();										//*4
		dbg.SetDebugLevel(kDbgLvlSilent);								//*5
	}
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_UNITTESTUTILS_H

// eof
