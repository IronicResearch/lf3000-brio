/******************************************************
* Benchmark functions for BlackFin.
* (C) 2005 Mobileer, 
******************************************************/
#include 	<stddef.h>
#include 	<string.h>
#include    <stdio.h>
#include    <time.h> 

#include    "bench_tools.h"

/* These are host specific functions. */
int Bench_GetTicks( void )
{
	volatile clock_t clock_value = clock();
	return (int) (clock_value >> 8);
}

int Bench_GetTicksPerSecond( void )
{
	return (100 * 1000000) >> 8;
}

/** Lock out other tasks to get more accurate benchmark. */
void Bench_Boost( void )
{}
void Bench_Unboost( void )
{}

#if 1
int main( void )
{
	printf( "Benchmark Blackfin.\n" );
	return Benchmark_Perform();
}
#endif
