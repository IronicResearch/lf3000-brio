
#ifndef __LFPROFILER_H__
#define __LFPROFILER_H__

#ifdef LF_PROFILE

#define PROF_BLOCK_START(block_name) _profile_block_start(block_name)
#define PROF_BLOCK_END() _profile_block_end()
#define PROF_RESET() _profile_reset()
#define PROF_PRINT_REPORT() _profile_print()

#else // LF_PROFILE

#define PROF_BLOCK_START(block_name) ((void)0)
#define PROF_BLOCK_END() ((void)0)
#define PROF_PRINT_REPORT() ((void)0)

#endif // LF_PROFILE

void _profile_block_start(const char* func_name);
void _profile_block_end();
void _profile_print( void );
void _profile_reset();
void foobar();

#endif // __LFPR
