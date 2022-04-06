#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// To compile for testing:
// arm-linux-g++ -DTESTING -ISystem/Include -IThirdParty/boost/ \
// System/Utility/FlatProfiler.cpp -o FlatProfiler
#define ENABLE_PROFILING
#include <FlatProfiler.h>

static unsigned int *timeStamps;
static unsigned int *tsIndex;
static int numTimeStamps;
static int numTags;
static struct timeval *tvCurrent;

#define NUM_TIMESTAMP_BINS 20

//Initialize the flat profiler.  If numTags (nt) or numTimeStamps (nts) is
//non-zero, it overrides the FLATPROF_NUM_TAGS and FLATPROF_NUM_TIMESTAMPS
//respectively.
int FlatProfilerInit(int nt, int nts)
{
	int ret = 0;

	numTags = nt;
	if(nt == 0)
		numTags = FLATPROF_NUM_TAGS;
	
	numTimeStamps = nts;
	if(numTimeStamps == 0)
		numTimeStamps = FLATPROF_NUM_TIMESTAMPS;

	timeStamps = tsIndex = 0;
	tvCurrent = NULL;

	timeStamps = (unsigned int *)malloc(numTags * numTimeStamps * sizeof(int));
	if ( !timeStamps )
		return -1;

	tsIndex = (unsigned int *)malloc(numTags * sizeof(int));
	if ( !tsIndex ) {
		ret = -1;
		goto error;
	}
	memset(tsIndex, 0, numTags*sizeof(int));
	
	tvCurrent = (struct timeval *)malloc(numTags*sizeof(struct timeval));
	if ( !tvCurrent ) {
		ret = -1;
		goto error;
	}
	
	return ret;

 error:
	free(timeStamps);
	free(tsIndex);
	free(tvCurrent);
}

void FlatProfilerDone(void)
{
	unsigned int tag, ts, t, mean, max, min, *myTimeStamps;
	unsigned int bins[NUM_TIMESTAMP_BINS], b;
	float binSize, acc;
	printf("Compiling profile results...\n");
	for(tag=0; tag<numTags; tag++) {

		if(tsIndex[tag] == 0)
			continue;

		/* Calculate the results */
		myTimeStamps = timeStamps + numTimeStamps*tag;
		max = 0;
		min = 0xffffffff;
		acc = 0.0;

		for(ts=0; ts<tsIndex[tag]; ts++) {
			/* calculate min, mean, and max */
			t =	myTimeStamps[ts];
			if (t > max)
				max = t;
			if (t < min)
				min = t;
			acc += (float)t;
		}
		mean = (unsigned int)(acc/tsIndex[tag]);

		/* Print the results */
		printf("tag %d: max=%dms min=%dms mean=%dms\n", tag, max, min, mean);

		/* generate histogram */
		if(max == 0)
			continue;
		memset(bins, 0, NUM_TIMESTAMP_BINS*sizeof(int));
		binSize = (float)max/(NUM_TIMESTAMP_BINS - 1);
		for(ts=0; ts<tsIndex[tag]; ts++) {
			t =	myTimeStamps[ts];
			bins[(int)((float)t/binSize)] += 1;
		}
		printf("Range: number of instances\n");
		for(b=0; b<NUM_TIMESTAMP_BINS; b++) {
			printf("%dms-%dms:\t\t%d\n", (int)(binSize*b), (int)(binSize*(b+1)),
				   bins[b]);
		}
		printf("\n");
	}
	
	free(timeStamps);
	free(tsIndex);
	free(tvCurrent);
}

//Take a time stamp
void TimeStampOn(int tag)
{
	if(tag >= numTags)
		return;

	gettimeofday(&tvCurrent[tag], NULL);
	//if this fails, we have no recourse
}

void TimeStampOff(int tag)
{
	struct timeval tv, *base_tv;
	unsigned int ts, *myTimeStamps = timeStamps + numTimeStamps*tag;

	if(tag >= numTags)
		return;

	if(gettimeofday(&tv, NULL) == -1) {
		/* If this fails, we have no recourse */
		return;
	}
	
	base_tv = &tvCurrent[tag];
	/* start by calculating seconds */
	if(tv.tv_usec >= base_tv->tv_usec) {
		ts = (tv.tv_sec - base_tv->tv_sec) * 1000;
		ts += (tv.tv_usec - base_tv->tv_usec) / 1000;
	} else {
		ts = (tv.tv_sec - base_tv->tv_sec - 1) * 1000;
		ts += 1000 - (base_tv->tv_usec - tv.tv_usec) / 1000;
	}
	
	if(tsIndex[tag] < numTimeStamps) {
		myTimeStamps[tsIndex[tag]++] = ts;
	}
}

#ifdef TESTING

int main(void)
{
	int i, r1, r2, s;

	if(FlatProfilerInit(0, 0) != 0)
		printf("Failed to init profiler\n");

    for(i=0; i<10000; i++) {
		
		TimeStampOn(0); // Outer loop profile is tag 0

		r1 = rand();
		r2 = rand();
		
		s = (int)((float)r1/RAND_MAX*10000.0);
		TimeStampOn(1);
		usleep(s);
		TimeStampOff(1);

		s = (int)((float)r2/RAND_MAX*10000.0);
		TimeStampOn(2);
		usleep(s);
		TimeStampOff(2); // Outer loop profile is tag 0

		TimeStampOff(0); // Outer loop profile is tag 0		
    }

	FlatProfilerDone();

    return 0;
}

#endif
