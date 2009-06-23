//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Keyboard.cpp
//
// Description:
//		Implements Utility functions for Brio.
//
//============================================================================
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <asm-generic/int-ll64.h>
#include <linux/input.h>

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

#define KEYBOARD_NAME	"LF1000 Keyboard"
#define MAX_DEVNODES	8

bool	scanned = false;
char	dev[20];

//----------------------------------------------------------------------------
// Returns the file name for the LF1000 Keyboard, if one is found
//----------------------------------------------------------------------------
char *GetKeyboardName(void)
{
	int fd;
	char name[32];

	if(scanned)
		return dev;

	for(int i = 0; i < MAX_DEVNODES; i++) {
		sprintf(dev, "/dev/input/event%d", i);
		fd = open(dev, O_RDONLY);
		if(fd < 0) {
			perror("can't open device\n");
			return NULL;
		}

		if(ioctl(fd, EVIOCGNAME(32), name) < 0) {
			perror("can't get keyboard name\n");
			close(fd);
			return NULL;
		}

		if(!strcmp(name, KEYBOARD_NAME)) { // found
			close(fd);
			scanned = true;
			return dev;
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
