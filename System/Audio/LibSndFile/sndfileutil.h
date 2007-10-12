#ifndef SNDFILEUTIL_H
#define SNDFILEUTIL_H

#include "sfconfig.h"
#include "sndfile.h"

#ifdef __cplusplus
extern "C" {
#endif

void Print_SF_INFO(SF_INFO *d);

SNDFILE	*OpenSoundFile( char *path, SF_INFO *sfi, long rwType);
int CloseSoundFile( SNDFILE **soundFile );
// int RewindSoundFile( SNDFILE **soundFile, SF_INFO *afi, char *path )

/* Add whatever here */
#ifdef __cplusplus
}
#endif



#endif  // SNDFILEUTIL_H



