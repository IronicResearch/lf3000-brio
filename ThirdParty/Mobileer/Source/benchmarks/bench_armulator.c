/******************************************************
* Benchmark functions for ARMULATOR.
* (C) 2005 Mobileer, 
******************************************************/
#include 	<stddef.h>
#include 	<string.h>
#include    <stdio.h>

#include    "bench_tools.h"

#define ARMULATOR_MSEC_CLOCK ((volatile unsigned long *) 0x0bfffff0)

/* These are host specific functions. */
int Bench_GetTicks( void )
{
	return (int) *ARMULATOR_MSEC_CLOCK;
}

int Bench_GetTicksPerSecond( void )
{
	return 1000;
}

/** Lock out other tasks to get more accurate benchmark. */
void Bench_Boost( void )
{}
void Bench_Unboost( void )
{}

#if 1
int main( void )
{
	printf( "Benchmark ARMULATOR.\n" );
	return Benchmark_Perform();
}
#endif
