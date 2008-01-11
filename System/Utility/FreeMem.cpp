//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		FreeMem.cpp
//
// Description:
//		Implements Utility functions for Brio.
//
//============================================================================
#include <stdio.h>
#include <vector>	
#include <string>	
#include <fstream> 
#include <fcntl.h>
#include <SystemTypes.h>
#include <CoreTypes.h>	
#include <boost/shared_array.hpp>	

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
// Returns free memory snapshot in user space
//----------------------------------------------------------------------------
int GetFreeMem(void)
{
	int 	freemem = 0;
	int	size = 26;
	char	buf[80];
	char	stat[80];
	string	str = "";
	FILE*	f = NULL;

	f = fopen("/proc/meminfo", "r");
	if (f == NULL)
		return 0;
	while (!feof(f)) {
		fread(buf, 1, size, f);
		str += buf;
		if (strncmp("MemFree:", buf, 8) == 0) {
			sscanf(buf, "%s%d", &stat[0], &freemem); 
//			printf("%s%d\n", stat, freemem);
			break;
		}
	}
	fclose(f);

	return freemem;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
