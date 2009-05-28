//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Uptime.cpp
//
// Description:
//		Utility function to print uptime to console in uniform manner.
//
//============================================================================
#include <stdio.h>
#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

void PrintUptime(const char *tag)
{
#define LEN 32
        char buf[LEN];
        buf[0] = 0;
        FILE *fin = fopen ("/proc/uptime", "r");
        if (fin)
        {
                int x = fread (buf, 1, LEN, fin);
                int len = std::min(x, LEN-1);
                buf[len] = 0;
                fclose (fin);
        }
        printf ("+++ AppManager: %s: %s", tag, buf); 
#undef LEN 
}

LF_END_BRIO_NAMESPACE()

// EOF
