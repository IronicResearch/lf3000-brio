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

#include <StringTypes.h>
#include <EmulationConfig.h>

//==============================================================================
class TestSuiteBase
{
protected:
	TestSuiteBase()
	{
		const char* kPath = "Build/LightningGCC_emulation/Module";
		LeapFrog::Brio::EmulationConfig::Instance().SetModuleSearchPath(kPath);
	}
};

#endif // LF_BRIO_UNITTESTUTILS_H

// eof
