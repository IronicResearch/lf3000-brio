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
#include <dirent.h>
#include <sys/ioctl.h>

#undef __STRICT_ANSI__
#include <linux/input.h>

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

#define KEYBOARD_NAME	"LF1000 Keyboard"

bool	scanned = false;
char	dev[20];

//----------------------------------------------------------------------------
// Returns the file name for the LF1000 Keyboard, if one is found
//----------------------------------------------------------------------------
char *GetKeyboardName(void)
{
	struct dirent *dp;
	DIR *dir;
	int fd;
	char name[32];

	if(scanned)
		return dev;

	dir = opendir("/dev/input/");
	if (!dir) {
		perror("can't open /dev/input directory\n");
		return NULL;
	}

	/* scan all /dev/input/eventN files until a match is found, in which
	 * case we return the path to the first event file which matched. */
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_name && !strncmp(dp->d_name, "event", 5)) {
			sprintf(dev, "/dev/input/%s", dp->d_name);
			fd = open(dev, O_RDONLY);
			if(fd == -1) {
				perror("can't open device\n");
				continue;
			}

			if(ioctl(fd, EVIOCGNAME(32), name) < 0) {
				perror("can't get keyboard name\n");
				close(fd);
				continue;
			}

			if(!strcmp(name, KEYBOARD_NAME)) { // found
				close(fd);
				closedir(dir);
				scanned = true;
				return dev;
			}
		}
	}

	closedir(dir);
	return NULL;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
