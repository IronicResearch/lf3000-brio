//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		MfgDataWrapper.cpp
//
// Description:
//		MfgData I/O routines.
//
//==============================================================================

#include <libMfgData.h>

//----------------------------------------------------------------------------
bool MFG_Is50Hz(void)
{
#if defined(LF3000) && !defined(EMULATION)
	int hz = 0;

	CMfgData mfgdata;
	if (mfgdata.Init() == 0) {
		mfgdata.GetLineHz(hz);
		mfgdata.Exit();
		return (hz == 50) ? true : false;
	}
#endif
	return false;
}
//----------------------------------------------------------------------------
// EOF
