#include "stdafx.h"
/******************************************************
* Benchmark ADPCM decoder.
* (C) 2005 Mobileer, 
******************************************************/
#include 	<stddef.h>
#include 	<string.h>
#include    <stdio.h>

#include    "ucos_ii.h"

#include    "bench_tools.h"

/* These are host specific functions. */
int Bench_GetTicks( void )
{
	return (INT32S) OSTimeGet ();
}

int Bench_GetTicksPerSecond( void )
{
	return 100;
}

/** Lock out other tasks to get more accurate benchmark. */
void Bench_Boost( void )
{
	// FIXME cannot link!	OSSchedLock();
}
void Bench_Unboost( void )
{
	// FIXME cannot link!	OSSchedUnlock();
}

/********************************************************
void PrintCR1( void )
{
	register int cr1;
	
	// Check cache control register value.
	asm ("MRC p15,0,%0,CR1,CR0,0" : "=r" (cr1) );
	
	Debug_PrintString( "CR1 = ");
	Debug_PrintHexNumber( cr1 );
	Debug_PrintString("\n");
}
*/

int SharpMain( void )
{
	print( "Benchmark LH79524.\n" );
	return Benchmark_Perform();
}
