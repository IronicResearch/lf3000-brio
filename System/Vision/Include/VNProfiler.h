
#ifndef __LFPROFILER_H__
#define __LFPROFILER_H__

#if VN_PROFILE

#define PROF_BLOCK_START(block_name) _profile_block_start(block_name)
#define PROF_BLOCK_END() _profile_block_end()
#define PROF_RESET() _profile_reset()
#define PROF_PRINT_REPORT() _profile_print()
#define PROF_PRINT_REPORT_AFTER_COUNT(C) _profile_print_after_count( C )
#define PROF_FRAMES_PER_SECOND(name) _profile_frames_per_second(name)

#else // VN_PROFILE

#define PROF_BLOCK_START(block_name) ((void)0)
#define PROF_BLOCK_END() ((void)0)
#define PROF_PRINT_REPORT() ((void)0)
#define PROF_RESET() ((void)0)
#define PROF_FRAMES_PER_SECOND(name) ((void)0)
#define PROF_PRINT_REPORT_AFTER_COUNT(C) ((void)0)

#endif // VN_PROFILE

void _profile_block_start(const char* func_name);
void _profile_block_end();
void _profile_print( );
void _profile_print_after_count( int c );
void _profile_reset();
void _profile_frames_per_second(const char* name);


#endif // __LFPROFILER_H__
