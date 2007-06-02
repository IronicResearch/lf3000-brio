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
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class TestSuiteBase
{
protected:
	TestSuiteBase()
	{
#ifdef EMULATION
		const char* kPath = "Build/Lightning_emulation/Module";
		EmulationConfig::Instance().SetModuleSearchPath(kPath);
#endif
	}
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_UNITTESTUTILS_H

// eof
