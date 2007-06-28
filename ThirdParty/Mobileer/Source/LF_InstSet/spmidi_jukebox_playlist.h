#ifndef _SPMIDI_JUKEBOX_PLAYLIST_H
#define _SPMIDI_JUKEBOX_PLAYLIST_H
/**
 * Song playlist for JukeBox.
 * Generated automatically by Mobileer Editor
 * Do NOT edit by hand!
 * (C) Mobileer, Inc. CONFIDENTIAL and PROPRIETARY
 */


typedef struct JukeBoxEntry_s
{
	const unsigned char *image;
	int size;
} JukeBoxEntry_t;


JukeBoxEntry_t *jukeBoxSongs = NULL;

#define JUKEBOX_NUM_SONGS  (0)


#endif /* _SPMIDI_JUKEBOX_PLAYLIST_H */
