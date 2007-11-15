#ifndef _ME_DEMO_H
#define _ME_DEMO_H

/**
 * Initialize everything needed.
 */
int MobileerDemo_Init( long sampleRate );

/**
 * Synthesizes the next buffer of audio samples.
 * A stereo frame is two samples. Recommend multiple of 256
 * If stereo then it will be interleaved.
 * Number of samples in sampleBuffer is numFrames*numChannels.
 * This will play from an internal series of MIDI files in a loop.
 *
 * @param inputBuffer array of mixed signed 16 bit audio data
 * @param outputBuffer array of mixed signed 16 bit audio data
 * @param numFrames is number of audio frames in buffer.
 * @param numChannels is 1 for mono, 2 for stereo
 */
int MobileerDemo_Synthesize( short *inputBuffer,
                             short *mixedBuffer,
                             int numFrames, int numChannels );


/**
 * Cleanup.
 */
void MobileerDemo_Term( void );


#endif /* _ME_DEMO_H */

