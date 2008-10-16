//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		ButtonHandler.cpp
//
// Description:
//		Implements the underlying Button Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <ButtonPriv.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>
#include <Utility.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Platform-specific delegate functions
//============================================================================

//----------------------------------------------------------------------------

void CButtonModule::InitModule()
{
	// Button thread consolidated into Event manager
}

//----------------------------------------------------------------------------
void CButtonModule::DeinitModule()
{
}

//============================================================================
// Button state
//============================================================================

//----------------------------------------------------------------------------
tButtonData CButtonModule::GetButtonState() const
{
	return LeapFrog::Brio::GetButtonState();
}

LF_END_BRIO_NAMESPACE()
// EOF
