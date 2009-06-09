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
static int				refcnt = 0;

//============================================================================
// Ctor & dtor
//============================================================================
CButtonModule::CButtonModule(void)
{
	if (sinst == NULL) {
		sinst = this;
		InitModule();
	}
	refcnt++;
}

CButtonModule::~CButtonModule(void)
{
	refcnt--;
	if (refcnt == 0) {
		DeinitModule();
	}
}

LF_END_BRIO_NAMESPACE()

// EOF
