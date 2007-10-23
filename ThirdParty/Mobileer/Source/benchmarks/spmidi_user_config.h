#ifndef _SPMIDI_USER_CONFIG_H
#define _SPMIDI_USER_CONFIG_H

/* $Id: spmidi_user_config.h,v 1.7 2007/10/02 16:12:57 philjmsl Exp $ */
/**
 *
 * @file spmidi_conig.h
 * @brief Configuration file to select compile time options.
 * @author Phil Burk, Copyright 2002 Mobileer PROPRIETARY and CONFIDENTIAL
 */

/** By defining SPMIDI_USER_CONFIG you can set your compile time configuration variables
 * in file called "spmidi_user_config.h" instead of passing them on the command line.
 */
#define SPMIDI_ME3000   (0)
#define SPMIDI_ME2000   (1)


#define SPMIDI_PRODUCTION          (1)

/** Absolute maximum number of voices allowed.
 * Internal structures will be allocated based on this value.
 * The actual maximum number of voices can be lowered dynamically by passing a value to SPMIDI_SetMaxVoices().
 */
#define SPMIDI_MAX_VOICES          (32)

/** Maximum rate that can be specified in call to SPMIDI_CreateContext().
 * This compile time maximum will affect the size of buffers such
 * as the compressor delay line. 
 * RAM can be saved by setting this to the maximum that you expect to use.
 */
#define SPMIDI_MAX_SAMPLE_RATE   (22050)

/** User can select mono or stereo synthesis when calling SPMIDI_ReadFrames().
 * One can save some memory by setting this to (1) at compile time but that
 * will prevent calling SPMIDI_ReadFrames() with samplesPerFrame greater than one.
 */
#define SPMIDI_MAX_SAMPLES_PER_FRAME   (1)

#ifndef SPMIDI_FRAMES_PER_BLOCK_LOG2
#define SPMIDI_FRAMES_PER_BLOCK_LOG2    (3)
#endif

/** Define this as zero to disable the dynamic range compressor. */
#define SPMIDI_USE_COMPRESSOR        (1)

/** Define this as one to enable the reverberation effect. */
#define SPMIDI_USE_REVERB            (0)

/** Define this as one to enable dynamic memory allocation. */
#define SPMIDI_SUPPORT_MALLOC        (0)

#define SPMIDI_USE_INTERNAL_MEMHEAP (1)
#define SPMIDI_MEMHEAP_SIZE  (128*1024)

#define SPMIDI_RELOCATABLE          (0)

/** Define the maximum number of SPMIDI_Contexts that can be created.
 * This is only used when SPMIDI_SUPPORT_MALLOC is zero.
 * It determines the number of context data structures that
 * are statically allocated at compile time.
 */
#define SPMIDI_MAX_NUM_CONTEXTS    (1)

/** Define the maximum number of MIDIFilePlayer's that can be created.
 * This is only used when SPMIDI_SUPPORT_MALLOC is zero.
 * It determines the number of MIDIFile player data structures that
 * are statically allocated at compile time.
 */
#define SPMIDI_MAX_NUM_PLAYERS     (1)


#define SPMIDI_USE_STDLIB  (0)

#endif /* _SPMIDI_USER_CONFIG_H */

