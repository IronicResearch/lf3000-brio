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
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class TestSuiteBase
{
protected:
	TestSuiteBase(const char* name = NULL)
	{
#ifdef EMULATION
		const char* kPath = "Build/Lightning_emulation/Module";
		EmulationConfig::Instance().Initialize("");
		EmulationConfig::Instance().SetModuleSearchPath(kPath);
		if (name != NULL)
		{
			CPath path = EmulationConfig::Instance().GetCartResourceSearchPath();
			size_t idx = path.find("/Build/");
			path = path.substr(0, idx) + "/UnitTestData/";
			path += name;
			path += "/";
			EmulationConfig::Instance().SetCartResourceSearchPath(path.c_str());
		}
#endif
		CDebugMPI	dbg(kGroupUnitTests);
		dbg.EnableThrowOnAssert();
		dbg.SetDebugLevel(kDbgLvlSilent);
	}
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_UNITTESTUTILS_H

// eof
