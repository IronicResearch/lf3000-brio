//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		LightningSetup.cpp
//
// Description:
//		Lightning-specific Resource Manager setup
//
//============================================================================
#include <ResourcePriv.h>

LF_BEGIN_BRIO_NAMESPACE()

void GetDeviceMountPoints(MountPoints& mp)
{
	mp.push_back(kBaseRsrcPath);
	mp.push_back(kCart1RsrcPath);
}

LF_END_BRIO_NAMESPACE()
// EOF
