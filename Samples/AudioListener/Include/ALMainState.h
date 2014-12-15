#ifndef AUDIO_TRIGGEREDAUDIO_INCLUDE_ALMAINSTATE_H_
#define AUDIO_TRIGGEREDAUDIO_INCLUDE_ALMAINSTATE_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file ALMainState.h
*
* The ALMainState class is responsible for handling all button and touch events,
* creating the touch button and playing the audio file.  ALMainState is the
* only state in which the AudioListenerApp can take on.
*
**
* author: 	leapfrog
*  			alindblad - 10/22/12 - Created copy from STUB app
*
******************************************************************************/

#include <GameStateBase.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <AudioMPI.h>  // API to play audio files
#include "AudioListenerApp.h"
#include "Common/LFTouchButton.h" // helper class for handling touch buttons
#include "Common/LFDisplayRenderer.h"

class ALAudioListener;
/*
 *  ALMainState class.
 *
 * The only state of the AudioListener app.  This class is responsible
 * for creating the touch button, handling touch and button events and
 * playing the audio file in response to the trigger of a touch event.
 */
class ALMainState : public CGameStateBase, public LFTouchButtonDelegate {
 public:
	/*
	 *  ALMainState default constructor
	 *
	 *  Creates an app instance.
	 */
    ALMainState(LeapFrog::Brio::tDisplayHandle handle);

	/*
	 *  ALMainState destructor
	 *
	 *  performs class cleanup, i.e.frees all memory allocated
	 */
    ~ALMainState();

	/*
	 * TODO: research Suspend()
	 */
    virtual void Suspend();

	/*
	 * TODO: research Resume()
	 */
    virtual void Resume();

	/*
	 * TODO: research Enter()
	 */
    virtual void Enter(CStateData* userData);

	/*
	 * TODO: research Update()
	 */
    virtual void Update(CGameStateHandler* sh);

	/*
	 * TODO: research Exit()
	 */
    virtual void Exit();

    /*
     * HandleAudioDonePlaying
     *
     * This method is gets called from the audio listener when an audio
     * file is done playing.  This is where the second audio file is played.
     */
    void HandleAudioDonePlaying(tAudioID audio_id);

 protected:
	/*
	 * DoTouchEventLoop
	 *
	 * Checks for any touch events and then handles an event appropriately
	 */
    int DoTouchEventLoop(void);

	/*
	 * DoKeyEventLoop
	 *
	 * Checks for any physial button events and then handles an event appropriately
	 * Physical buttons are the native buttons to the device, i.e. the home button
	 */
    int DoKeyEventLoop(void);

    LeapFrog::Brio::CTouchEventQueue    touch_event_queue_;	   ///< TODO: research the touch event queue
    LeapFrog::Brio::CButtonEventQueue   button_event_queue_;   ///< TODO: research the button event queue

    CAudioMPI                           audio_mpi_;            ///< used as main interface to audio class
    std::vector<LeapFrog::Brio::CPath>  sample_audio_;         ///< paths to the sample audio files
    LFTouchButton                       touch_button_;         ///< the touch button used to trigger audio
    tAudioID                            audio_id_;             ///< the id of the last created audio
    ALAudioListener*                    audio_listener_;       ///< fires callback when an audio file finishes playing
    bool                                first_audio_playing_;  ///< indicates if the first sample audio is playing

    LFDisplayRenderer                   display_renderer_;     ///< handles display buffers

 private:
	/*
	 * init
	 *
	 * A helper method that performs all initialization required by
	 * the touch button and audio used in this example.
	 */
    void Init(void);

	/*
	 * fillSampleAudioVector
	 *
	 * A helper method that creates a vector of paths to
	 * sample audio files that can be played
	 */
    void FillSampleAudioVector(void);

	/*
	 * addTouchButton
	 *
	 * A helper method that creates the touch button for
	 * rendering on the screen
	 */
    void AddTouchButton(void);

    /*
     * CreateAudioListener
     *
     * This method is the listener that gives ALMainState the notification that
     * an audio file is done playing
     */
    void CreateAudioListener(void);

	/*
	 * registerEventListeners
	 *
	 * A helper method that registers listeners for screen touch and
	 * physical button events.
	 */
    void RegisterEventListeners(void);

	/*
	 * unregisterEventListeners
	 *
	 * A helper method that unregisters the listeners that were
	 * registered via a call to registerEventListeners
	 */
    void UnregisterEventListeners(void);

    /*
     * HandleTouchEvent
     *
     * This method is the delegate callback for touch button events.
     * In this app this is where the audio is played.
     */
    void HandleTouchEvent(unsigned int id);

    /*
     * PlayRandomAudio
     *
     * This method plays a single random audio file from the sample files.
     */
    void PlayRandomAudio(void);
};

#endif  // AUDIO_TRIGGEREDAUDIO_INCLUDE_ALMAINSTATE_H_

