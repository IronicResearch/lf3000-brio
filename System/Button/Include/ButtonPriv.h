#ifndef LF_BRIO_BUTTONPRIV_H
#define LF_BRIO_BUTTONPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ButtonPriv.h
//
// Description:
//		Defines the interface for the private underlying Button manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <StringTypes.h>
#include <EventTypes.h>
#include <ButtonTypes.h>
#include <DebugMPI.h>
#include <EventMPI.h>
#include <KernelMPI.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Constants
//==============================================================================
const CString			kButtonModuleName		= "Button";
const CURI				kModuleURI				= "/LF/System/Button";
const tVersion			kButtonModuleVersion	= 3;
const tEventPriority	kButtonEventPriority	= 0;	//TBD/tp: make replacable?

class CButtonModule
{
public:
	CButtonModule(void);
	~CButtonModule(void);
	void InitModule(void);
	void DeinitModule(void);
	tButtonData GetButtonState() const;
private:
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
public:
	CEventMPI			eventmgr_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONPRIV_H

// eof
