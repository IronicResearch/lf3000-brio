//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		EmulationSetup.cpp
//
// Description:
//		Emulation-specific Resource Manager setup
//
//============================================================================
#include <ResourcePriv.h>
#include <EmulationConfig.h>

LF_BEGIN_BRIO_NAMESPACE()

void GetDeviceMountPoints(MountPoints& mp)
{
	mp.push_back(EmulationConfig::Instance().GetBaseResourceSearchPath());
	mp.push_back(EmulationConfig::Instance().GetCartResourceSearchPath());
}

LF_END_BRIO_NAMESPACE()
// EOF
