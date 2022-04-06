#ifndef LF_BRIO_UNITTESTVISIONUTILS_H
#define LF_BRIO_UNITTESTVISIONUTILS_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		UnitTestVisionUtils.h
//
// Description:
//		Vision utilities for unit tests
//
//==============================================================================


LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class UnitTestVisionUtils
{

public:

	UnitTestVisionUtils()
		: isVGA(false) {
		init();
	}

	int GetScreenHeight() {
		return (IsQVGA() ? 240 : 480); // VGA if not QVGA
	}

	int GetScreenWidth() {
		return (IsQVGA() ? 320 : 640); // VGA if not QVGA
	}

	bool IsQVGA()
	{
		return isVGA;
	}
	
private:
	bool isVGA;

	void init() {
		struct stat s;
		isVGA =  (0 == stat("/flags/qvga_vision_mode", &s));
	}
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_UNITTESTVISIONUTILS_H

// eof
