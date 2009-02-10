#ifndef LF_BRIO_POWERPRIV_H
#define LF_BRIO_POWERPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PowerPriv.h
//
// Description:
//		Defines the interface for the private underlying Power manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <EventTypes.h>
#include <PowerTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Constants
//==============================================================================
const CString			kPowerModuleName	= "Power";
const CURI				kModuleURI			= "/LF/System/Power";
const tVersion			kPowerModuleVersion	= 3;
const tEventPriority	kPowerEventPriority	= 0;	//TBD/tp: make replacable?

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_POWERPRIV_H

// eof
