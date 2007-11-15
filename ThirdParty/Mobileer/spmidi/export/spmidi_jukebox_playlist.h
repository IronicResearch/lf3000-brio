#ifndef _SPMIDI_JUKEBOX_PLAYLIST_H
#define _SPMIDI_JUKEBOX_PLAYLIST_H
/**
 * Song playlist for JukeBox.
 * Generated automatically by Mobileer Editor
 * Do NOT edit by hand!
 * (C) Mobileer, Inc. CONFIDENTIAL and PROPRIETARY
 */

#include "songs/song_EchoEcho_rt.h"
#include "songs/song_BonyparteCall_rt.h"
#include "songs/song_Paco_de_Lucia_Guajiras_Lucia.h"
#include "songs/song_024AcGuitar.h"

typedef struct JukeBoxEntry_s
{
	const unsigned char *image;
	int size;
} JukeBoxEntry_t;


    JukeBoxEntry_t jukeBoxSongs[] =
{
    { song_EchoEcho_rt, sizeof(song_EchoEcho_rt) },
    { song_BonyparteCall_rt, sizeof(song_BonyparteCall_rt) },
    { song_Paco_de_Lucia_Guajiras_Lucia, sizeof(song_Paco_de_Lucia_Guajiras_Lucia) },
    { song_024AcGuitar, sizeof(song_024AcGuitar) },
};

#define JUKEBOX_NUM_SONGS  (sizeof(jukeBoxSongs)/sizeof(JukeBoxEntry_t))


#endif /* _SPMIDI_JUKEBOX_PLAYLIST_H */
