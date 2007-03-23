#ifndef LF_BRIO_MODULEMGR_H
#define LF_BRIO_MODULEMGR_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ModuleMgr.h
//
// Description:
//		The boot system's ModuleMgr class is responsible for the following:
//
//		* Finding all system modules on devices
//		* Maintaining a table of found modules and their versions
//		* Managing module lifetimes using a lazy-loading policy
//		* Maintaining a policy of one instanciated module per subsystem
//		* Connecting module interface objects (ICoreMPI-derived classes)
//			to modules through Connect()/Disconnect() member functions
//		* Enforce versioning constraints between interface objects
//			and modules
//
//		One key distinction between system module objects (ICoreModule-derived)
//		and lightweight module interface objects (ICoreMPI-derived) is that
//		the former need to be replacable, so they are dynamically linked,
//		whereas the latter are statically linked.  Interface objects provide
//		the subsystem API to the application code that uses them, so it doesn't
//		make sense to update that interface for an existing application.
//
//==============================================================================
// NOTE: There are many subtle issues with module versioning that are not yet worked out.

#include <CoreModule.h>
#include <StringTypes.h>

class CModuleMgr {
public:
	static CModuleMgr* Instance();
	tErrType	FindModules();
	tErrType	Connect(ICoreModule*& ptr, const CString& name, 
						tVersion version);
	tErrType	Disconnect(const CString& name);

private:
	static CModuleMgr*	mpinst;
	CModuleMgr();
	~CModuleMgr();
};

#endif // LF_BRIO_MODULEMGR_H

// eof
