#ifndef LF_BRIO_CARTRIDGEPRIV_H
#define LF_BRIO_CARTRIDGEPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CartridgePriv.h
//
// Description:
//		Defines the interface for the private underlying Cartridge manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <EventTypes.h>
#include <CartridgeTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Constants
//==============================================================================
const CString			kCartridgeModuleName	= "Cartridge";
const CURI			kModuleURI		= "/LF/System/Cartridge";
const tVersion			kCartridgeModuleVersion	= 1;
const tEventPriority		kCartridgeEventPriority	= 0;	//TBD/tp: make replacable?

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_CARTRIDGEPRIV_H

// eof
