#ifndef _MIDIFILE_PLAYER_H
#define _MIDIFILE_PLAYER_H

/* $Id: midifile_player.h,v 1.18 2007/10/02 16:20:00 philjmsl Exp $ */
/**
 *
 * @file midifile_player.h
 * @brief MIDI File Parser and Player
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */
#include "include/spmidi.h"
#include "include/midifile_parser.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/** Opaque data type representing an internal MIDI File player structure. */
	typedef void * MIDIFilePlayer;

	/** Define function to receive text MetaEvents such as lyrics, etc. */
	typedef void (MIDIFilePlayer_TextCallback)( int trackIndex, int metaEventType,
	        const char *addr, int numChars, void *userData );

	/** Define function to receive regular MIDI bytes from SMF. */
	typedef void (MIDIFilePlayer_WriteCallback)( void *userContext, int byte );

	/** Define function to reset MIDI Synth context. */
	typedef void (MIDIFilePlayer_ResetCallback)( void *userContext );

	/**
	 * Print the contents of a MIDI File image in memory.
	 */
	int MIDIFile_Print( unsigned char *image, int numBytes );

	/**
	 * Setup a player for a MIDI file image.
	 * The parser will make an initial pass over the file to find the start of each track.
	 * It then assigns a track player to each track.
	 * Note that as of V1.80 you must call SPMIDI_Initialize() before calling this function.
	 */
	SPMIDI_Error MIDIFilePlayer_Create( MIDIFilePlayer **playerPtr, int sampleRate,
	                                    const unsigned char *image, int numBytes );

	SPMIDI_Error MIDIFilePlayer_Rewind( MIDIFilePlayer *player );

	/**
	 * Enable or disable NoteOn events on a track.
	 * NoteOff events will still be played to prevent stuck notes.
	 * All other events will be played as well to maintain the state of track.
	 *
	 * @param trackIndex Index of the track starting with zero.
	 *        In a format 1 MIDI file, track zero is typically the tempo track.
	 * @param onOrOff Zero to disable a track, non-zero to enable a track.
	 */
	SPMIDI_Error MIDIFilePlayer_SetTrackEnable( MIDIFilePlayer *player, int trackIndex, int onOrOff );

	/**
	 * Return enable status for a track.
	 * @param trackIndex Index of the track starting with zero.
	 *        In a format 1 MIDI file, track zero is typically the tempo track.
	 * @return onOrOff Zero if track is disabled, 1 if enabled, or negative on error.
	 */
	int MIDIFilePlayer_GetTrackEnable( MIDIFilePlayer *player, int trackIndex );

	/**
	 * @return Number of tracks in the MIDI file.
	 */
	int MIDIFilePlayer_GetTrackCount( MIDIFilePlayer *player );

	/**
	 * @return Length of the song in milliseconds.
	 */
	int MIDIFilePlayer_GetDurationInMilliseconds( MIDIFilePlayer *playerExt );

	/**
	 * @return Length of the song in frames.
	 */
	int MIDIFilePlayer_GetDurationInFrames( MIDIFilePlayer *playerExt );

	/**
	 * Set function to be called when a text MetaEvent is encountered while playing.
	 */
	void MIDIFilePlayer_SetTextCallback( MIDIFilePlayer *player,
	                                     MIDIFilePlayer_TextCallback *textCallback,
	                                     void *userData );

	/**
	 * Set function to be called when MIDI bytes are to be played.
	 */
	void MIDIFilePlayer_SetSynthCallbacks( MIDIFilePlayer *playerExt,
                                     MIDIFilePlayer_WriteCallback *writeCallback,
                                     MIDIFilePlayer_ResetCallback *resetCallback );

	/**
	 * Advance the track players forward in the file by a time corresponding
	 * to the given number of audio frames.
	 * @return 0 if more events are available to play. 1 if finished.
	 */
	int MIDIFilePlayer_PlayFrames( MIDIFilePlayer *player, SPMIDI_Context *spmidiContext, int numFrames );

	/**
	 * Reposition cursor in MIDI file to given frame.
	 * All MIDI events except noteOns will be processed so
	 * the MIDI state should be correct.
	 * If the desiredFrame is before the current frame then
	 * the song will be rewound to the beginning and played forward.
	 */
	int MIDIFilePlayer_GoToFrame( MIDIFilePlayer *playerExt, SPMIDI_Context *spmidiContext, int desiredFrame );

	/**
	 * @return Current playback time in MIDI file ticks.
	 */
	int MIDIFilePlayer_GetTickTime( MIDIFilePlayer *playerExt );

	/**
	 * @return Current playback time in audio frames.
	 */
	int MIDIFilePlayer_GetFrameTime( MIDIFilePlayer *playerExt );

	/**
	 * Delete the MIDI file player data.
	 */
	void MIDIFilePlayer_Delete( MIDIFilePlayer *player );

#ifdef __cplusplus
}
#endif

#endif

