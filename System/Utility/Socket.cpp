//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Socket.cpp
//
// Description:
//		Implements Utility functions for Brio.
//
//============================================================================
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
// Returns a connected socket
//----------------------------------------------------------------------------
int CreateReportSocket(const char *path)
{
	struct sockaddr_un sa;
	int s, ret;

	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if(s == -1)
		return s;

	memset(&sa, 0, sizeof(struct sockaddr_un));
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, path, strlen(path));

	ret = connect(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_un));
	if(ret < 0) {
		close(s);
		return ret;
	}

	return s;
}

//----------------------------------------------------------------------------
// Returns a listening socket
//----------------------------------------------------------------------------
int CreateListeningSocket(const char *path)
{
	struct sockaddr_un sa;
	int ls, ret;
	struct stat st;

	ls = socket(AF_UNIX, SOCK_STREAM, 0);
	if(ls == -1) {
		return -1;
	}
	fcntl(ls, F_SETFL, O_NONBLOCK);

	if(stat(path, &st) == 0) /* file exists */
		remove(path);

	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, path, strlen(path));
	ret = bind(ls, (struct sockaddr *)&sa, sizeof(struct sockaddr_un));
	if(ret < 0) {
		close(ls);
		return ret;
	}

	ret = listen(ls, 20);
	if(ret < 0) {
		close(ls);
		return ret;
	}
	
	return ls;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
