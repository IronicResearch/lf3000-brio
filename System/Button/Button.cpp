//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Button.cpp
//
// Description:
//		Implements Button module for emulation.
//
//============================================================================
#include <ButtonPriv.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Instance singleton for Button Module
//============================================================================

static CButtonModule*	sinst = NULL;

//============================================================================
// Ctor & dtor
//============================================================================
CButtonModule::CButtonModule(void)
{
	if (sinst == NULL) {
		sinst = this;
		InitModule();
	}
}

CButtonModule::~CButtonModule(void)
{
	if (sinst == this) {
		DeinitModule();
		sinst = NULL;
	}
}

LF_END_BRIO_NAMESPACE()

// EOF
