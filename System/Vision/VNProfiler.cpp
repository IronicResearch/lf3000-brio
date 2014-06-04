
#include "VNProfiler.h"
#include <map>
#include <stack>
#include <string>
#include <iostream>
#include <time.h>
#include <iomanip>
//#include <error.h>
#include <assert.h>
#include <float.h>
#include <sys/time.h>
#include <stdint.h>

#define VN_USE_KERNEL_TIMER 0
#if VN_USE_KERNEL_TIMER
#include <KernelMPI.h>
#endif


static const int spacing = 20;


struct ProfileEntry {
  uint32_t startTime;
  float totalTime;
  int iterations;
  float mn, mx;

  ProfileEntry() :
    startTime(0),
    totalTime(0),
    iterations(0),
    mn(FLT_MAX),
    mx(FLT_MIN)
  {}

  void start() {
#if VN_USE_KERNEL_TIMER
    LeapFrog::Brio::CKernelMPI kernel;
    kernel.GetHRTAsUsec(startTime);
#else
    timeval tv;
    gettimeofday( &tv, NULL );
    startTime = (tv.tv_sec * 1000000 + tv.tv_usec);
#endif
  }

  void end() {
    uint32_t now;
#if VN_USE_KERNEL_TIMER
    LeapFrog::Brio::CKernelMPI kernel;
    kernel.GetHRTAsUsec(now);
#else
    timeval tv;
    gettimeofday( &tv, NULL );
    now = (tv.tv_sec * 1000000 + tv.tv_usec);

#endif
    float time = float(now - startTime)/1000000.0f; // convert to seconds
    if( time < mn )
      mn = time;
    if( time > mx )
      mx = time;
    totalTime += time;
    iterations++;
  }


  void print( const std::string& name ) {

    std::cout	<< std::setw(spacing) << name
        << std::setw(spacing) << iterations
        << std::setw(spacing) << totalTime
        << std::setw(spacing) <<  (1000.0f * mn)
        << std::setw(spacing) <<  (1000.0f * mx)
        << std::setw(spacing) <<  (1000.0f * (totalTime / iterations))
        << std::endl;

  }
};

static std::map<std::string, ProfileEntry> entries;
static std::stack<ProfileEntry*> stack;

struct FPSEntry {
  uint32_t lastTime;
  float accumulatedTime;
  int frameCnt;
  float fps;
  float mn;
  float mx;

  FPSEntry()
    : lastTime(0)
    , accumulatedTime(0)
    , frameCnt(0)
    , fps(0)
    , mn(FLT_MAX)
    , mx(FLT_MIN)
  {}

  void tick() {
    uint32_t now = 0;
#if VN_USE_KERNEL_TIMER
    LeapFrog::Brio::CKernelMPI kernel;
    kernel.GetHRTAsUsec(now);
#else
    timeval tv;
    gettimeofday( &tv, NULL );
    now = (tv.tv_sec * 1000000 + tv.tv_usec);

#endif
    if( frameCnt != 0 ) {
      accumulatedTime += (float(now) - lastTime);
    }
    frameCnt++;
    lastTime = now;

    averageFPS();
  }

  float averageFPS() {
    float avg = accumulatedTime / float(frameCnt);
    fps = 1.0f / (avg/1000000.0f);
    if( fps > FLT_MAX )
      return fps;
    if( fps < mn )
      mn = fps;
    if( fps > mx )
      mx = fps;

    return fps;
  }

  void reset() {
    mn = 0;
    mx = 0;
    frameCnt = 0;
    accumulatedTime = 0;
  }

  void print( const std::string& name ) {

    std::cout	<< std::setw(spacing) << name
          << std::setw(spacing) << frameCnt
          << std::setw(spacing) << "-fps-"
          << std::setw(spacing) << mn
          << std::setw(spacing) << mx
          << std::setw(spacing) << fps
          << std::endl;
  }
};

std::map<std::string, FPSEntry> fpsEntries;

void _profile_block_start(const char* func_name) {
  std::map<std::string, ProfileEntry>::iterator it = entries.find(std::string(func_name));
  if( it == entries.end() ) {
    ProfileEntry entry;
    entries[std::string(func_name)] = entry;
    it = entries.find(std::string(func_name));
  }

  it->second.start();

  stack.push( &it->second );
}

void _profile_block_end() {
  ProfileEntry* entry = stack.top();
  entry->end();
  stack.pop();
}

void _profile_print( void ) {

  if( entries.size() > 0 ) {
    std::cout 	<< std::setw(spacing) << "name"
        << std::setw(spacing) << "iterations"
        << std::setw(spacing) << "total (s)"
        << std::setw(spacing) << "min (ms)"
        << std::setw(spacing) << "max (ms)"
        << std::setw(spacing) << "avg (ms)"
        << std::endl;

    std::map<std::string, ProfileEntry>::iterator it = entries.begin();
    for(; it != entries.end(); it++ ) {
      it->second.print(it->first);
    }

  }

  if( fpsEntries.size() > 0 ) {
    std::cout 	<< std::setw(spacing) << "name"
        << std::setw(spacing) << "iterations"
        << std::setw(spacing) << "---"
        << std::setw(spacing) << "min (fps)"
        << std::setw(spacing) << "max (fps)"
        << std::setw(spacing) << "avg (fps)"
        << std::endl;
    std::map<std::string, FPSEntry>::iterator fpsit = fpsEntries.begin();
    for(; fpsit != fpsEntries.end(); fpsit++ ) {
      fpsit->second.print(fpsit->first);
    }

  }

}

static int frameCnt = 0;
void _profile_print_after_count( int c ) {
  frameCnt++;
  if( frameCnt % c == 0 ) {
    _profile_print();
  }
}


void _profile_reset() {
  entries.clear();
}

void _profile_frames_per_second(const char* name) {
  std::map<std::string, FPSEntry>::iterator it = fpsEntries.find(std::string(name));
  if( it == fpsEntries.end() ) {
    FPSEntry fpsEntry;
    fpsEntries[std::string(name)] = fpsEntry;
    it = fpsEntries.find(std::string(name));

    assert( it != fpsEntries.end() && "could not create fps entry");
  }

  it->second.tick();
}
