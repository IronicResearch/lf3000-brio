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
		const char* kPath = "Build/LightningGCC_emulation/Module";
		EmulationConfig::Instance().SetModuleSearchPath(kPath);
	}
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_UNITTESTUTILS_H

// eof
