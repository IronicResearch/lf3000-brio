/*
 * $Id: pa_trace.c 1097 2006-08-26 08:27:53Z rossb $
 * Portable Audio I/O Library Trace Facility
 * Store trace information in real-time for later printing.
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2000 Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

/** @file
 @ingroup common_src

 @brief Event trace mechanism for debugging.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "rt_trace.h"

#if RT_TRACE_REALTIME_EVENTS

static char *traceTextArray[RT_MAX_TRACE_RECORDS];
static int traceIntArray[RT_MAX_TRACE_RECORDS];
static unsigned long traceTimeArray[RT_MAX_TRACE_RECORDS];
static int traceIndex = 0;
static int traceBlock = 0;

static struct timeval rt_time;
 
unsigned long gettime_usecs( void ) {

   gettimeofday(&rt_time, NULL);
   return rt_time.tv_usec;
}
   
/*********************************************************************/
void RT_ResetTraceMessages()
{
    traceIndex = 0;
}

/*********************************************************************/
void RT_DumpTraceMessages()
{
    int i;
    int messageCount = (traceIndex < RT_MAX_TRACE_RECORDS) ? traceIndex : RT_MAX_TRACE_RECORDS;

    printf("DumpTraceMessages: traceIndex = %d\n", traceIndex );
    for( i=0; i<messageCount; i++ )
    {
        printf("ev# %d: %s, cb count = %u, time = %u\n",
               i, traceTextArray[i], traceIntArray[i], traceTimeArray[i] );
     }

	fflush(stdout);
    RT_ResetTraceMessages();
 }

/*********************************************************************/
void RT_AddTraceMessage( const char *msg, int data )
{
    if( (traceIndex == RT_MAX_TRACE_RECORDS) && (traceBlock == 0) )
    {
        traceBlock = 1;
        /*  PaUtil_DumpTraceMessages(); */
    }
    else if( traceIndex < RT_MAX_TRACE_RECORDS )
    {
        traceTextArray[traceIndex] = msg;
        traceIntArray[traceIndex] = data;
        traceTimeArray[traceIndex] = gettime_usecs();
        traceIndex++;
    }
}

#endif /* TRACE_REALTIME_EVENTS */
