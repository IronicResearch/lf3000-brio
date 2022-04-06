#ifndef LF_BRIO_FLAT_PROFILER
#define LF_BRIO_FLAT_PROFILER

#include <SystemTypes.h>
#include <sys/time.h>
#include <time.h>

//----------------------------------------------------------------------------
// Simple Flat Profiler
//----------------------------------------------------------------------------

// This is a simple flat profiler.  All it does is take a timestamp before and
// after a given code block and report the difference at destructor time.  It is
// meant to profile tight loops to understand where various delays are coming
// from.  To use it, call FlatProfileOn before the code you wish to profile, and
// FlatProfileOff at the end.  It's the caller's responsibility to pass the
// proper tag to these functions.  Tags start at 0 and go to
// FLATPROF_NUM_TAGS-1.

// Set some defaults.  These can be overriden by the user at Init time by
// passing non-zero arguments to the constructor.

// This is the max number of different things you want to profile.  Again, it
// can be overridden by the user at constructor time
#define FLATPROF_NUM_TAGS 10

// This is the max number of timestamps that you will save for each for each
// tag.  Note that memory consumption scales with
// FLATPROF_NUM_TAGS*FLATPROF_NUM_TIMESTAMPS.  By default, memory usage is about
// 100kB.
#define FLATPROF_NUM_TIMESTAMPS 10000

#ifdef ENABLE_PROFILING
//Initialize the flat profiler.  If numTags or numTimeStamps is non-zero, it
//overrides the FLATPROF_NUM_TAGS and FLATPROF_NUM_TIMESTAMPS respectively.
int FlatProfilerInit(int numTags_, int numTimeStamps_);

//Call this once after the profiling is done.  It will dump profiling data
//to stdout
void FlatProfilerDone(void);

//Take a time stamp
void TimeStampOn(int tag);
void TimeStampOff(int tag);
#else
#define FlatProfilerInit(numTags_, numTimeStamps_) 0
#define FlatProfilerDone() {}
#define TimeStampOn(t) {}
#define TimeStampOff(t) {}
#endif

#endif // LF_BRIO_FLAT_PROFILER
