#ifndef LF_BRIO_CAMERATYPES_H
#define LF_BRIO_CAMERATYPES_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CameraTypes.h
//
// Description:
//		Type definitions for the Brio Camera subsystem.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <GroupEnumeration.h>
#include <EventMessage.h>
#include <SystemEvents.h>
#include <DisplayTypes.h>

#include <vector>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Typedefs
//==============================================================================

// Video capture handle
typedef tHndl 		tVidCapHndl;
typedef tHndl 		tAudCapHndl;

const tVidCapHndl	kInvalidVidCapHndl = static_cast<tVidCapHndl>(0);
const tVidCapHndl	kInvalidAudCapHndl = static_cast<tAudCapHndl>(0);


enum tControlType {
	kControlTypeError = 0,
	kControlTypeBrightness,
	kControlTypeContrast,
	kControlTypeSaturation,
	kControlTypeHue,
	kControlTypeGamma,
	kControlPowerLineFreq,
	kControlTypeSharpness,
	kControlTypeBacklightComp,
};

// Controls info
struct tControlInfo {
	tControlType	type;
	S32				min;
	S32				max;
	S32				preset;
	S32				current;
};

typedef std::vector<tControlInfo*> tCameraControls;

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_CAMERATYPES_H

// EOF
