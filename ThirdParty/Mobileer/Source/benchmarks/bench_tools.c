/*
 * Long benchmark for physical CPUs.
 * Copyright 2002 Mobileer
 *
 */
#include "midi.h"
#include "spmidi.h"
#include "spmidi_util.h"
#include "spmidi_print.h"
#include "bench_tools.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#ifdef WIN32
	#include <windows.h>

#ifdef _X86_
static __forceinline LONGLONG __cdecl getCpuCycleCount()
{
	__asm
	{
	    rdtsc
	}
}
#endif

#define READ_TIMER  ((long) getCpuCycleCount())
	#define TICK_RATE   (900000000) /* TODO - adjust for CPU clock rate */
#else
	#define READ_TIMER  Bench_GetTicks()
	#define TICK_RATE   Bench_GetTicksPerSecond()
#endif

/*******************************************************************/
#if defined(WIN32) || defined(BENCH_USE_MAIN)
int main(void);
int main(void)
#else
int Benchmark_Perform(void)
#endif
{
	int  err;
	long numFrames;
	long  frameRate;
	long startTime;
	long elapsedTimeInTicks;
	long checksum = 0;
	long elapsedTimeInFrames;
	long realMilliseconds;
	long audioMilliseconds;

	PRTMSG("Benchmark Tool\n");

	err = Benchmark_Init();
	if( err < 0 )
	{
		PRTMSGNUMH( "ERROR: Benchmark_Init returned ", err );
		return err;
	}

	PRTMSG("START\n");

	/* Measure the execution time of the critical code. */
	Bench_Boost();
	startTime = READ_TIMER;
	err = Benchmark_Run( &checksum, &numFrames, &frameRate );
	elapsedTimeInTicks = READ_TIMER - startTime;
	Bench_Unboost();
	if( err < 0 )
	{
		PRTMSGNUMH( "ERROR: Benchmark_Run returned ", err );
		return err;
	}
	
	PRTMSG("STOP\n");
	PRTMSGNUMD( "checksum = ", checksum );

	PRTMSGNUMD( "elapsed ticks = ", elapsedTimeInTicks );
	PRTMSGNUMD( "numFrames = ", numFrames );
	{
		int audioTicks = (numFrames * TICK_RATE) / frameRate;
		PRTMSGNUMD( "audio time in ticks = ", audioTicks );
	}
	realMilliseconds = (1000 * elapsedTimeInTicks) / TICK_RATE;
	PRTMSGNUMD( "real time in milliseconds = ", realMilliseconds );
	audioMilliseconds = (1000 * numFrames) / frameRate;
	PRTMSGNUMD( "audio time in milliseconds = ", audioMilliseconds );

#ifdef WIN32
	elapsedTimeInFrames = (long) ((double) elapsedTimeInTicks * frameRate / TICK_RATE);
#else
	elapsedTimeInFrames = (elapsedTimeInTicks * frameRate) / (long)TICK_RATE;
#endif

	PRTMSGNUMD( "elapsed time in frames = ", elapsedTimeInFrames );
	PRTMSGNUMD( "%% realtime = ", elapsedTimeInFrames * 100 / numFrames );

	err = Benchmark_Term();

	return err;
}
