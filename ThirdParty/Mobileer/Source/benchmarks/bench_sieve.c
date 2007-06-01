/*
 * Benchmark some instruments.
 * Copyright 2002 Mobileer
 *
 */
#include "stdio.h"
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



/*******************************************************************/
int Benchmark_Init( void )
{
	return 0;
}

/*******************************************************************/
int Benchmark_Run( long *checkSumPtr, long *numFramesPtr, long *frameRatePtr )
{
	int i;
#define SIEVE_NUM_LOOPS  (10)
#define SIEVE_DISABLE_IRQ  (0)

#if SIEVE_DISABLE_IRQ

	int_disable_irq();
	int_disable_fiq();
#endif

	for( i=0; i<SIEVE_NUM_LOOPS; i++ )
	{
		*checkSumPtr = sieve();
	}

#if SIEVE_DISABLE_IRQ
	int_enable_fiq();
	int_enable_irq();
#endif

	*numFramesPtr = 44100;
	*frameRatePtr = 44100;

	return 0;
}

/*******************************************************************/
int Benchmark_Term( void )
{
	return 0;
}
