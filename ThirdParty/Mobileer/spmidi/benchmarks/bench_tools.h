#ifndef _BENCH_TOOLS_H
#define _BENCH_TOOLS_H
/*
 * Benchmark tools
 * Copyright 2002 Mobileer
 *
 */

int Benchmark_Perform(void);

/* These functions are supplied by the program being benchmarked. */

int Benchmark_Init( void );
int Benchmark_Run( long *checkSumPtr, long *numFramesPtr, long *frameRatePtr );
int Benchmark_Term( void );

/* These are host specific functions. */
int Bench_GetTicks( void );
int Bench_GetTicksPerSecond( void );

/** Lock out other tasks to get more accurate benchmark. */
void Bench_Boost( void );
void Bench_Unboost( void );

#endif /* _BENCH_TOOLS_H */
