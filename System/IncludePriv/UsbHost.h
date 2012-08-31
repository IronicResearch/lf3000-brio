#ifndef LF_BRIO_USBHOST_H
#define LF_BRIO_USBHOST_H

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		UsbHost.h
//
// Description:
//		This class exists as a way to coordinate the powering on and
//		off of the usbhost.
//
//==============================================================================

#include <SystemTypes.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
LF_BEGIN_BRIO_NAMESPACE()

class CUsbHost {
public:
	static boost::shared_ptr<CUsbHost> Instance();
	~CUsbHost();

private:
	CUsbHost();
	static boost::weak_ptr<CUsbHost> mSingleton;
};

LF_END_BRIO_NAMESPACE()

#endif
