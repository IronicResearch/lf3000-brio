#ifndef VIDEO_VIDEO_INCLUDE_VIDMAINSTATE_H_
#define VIDEO_VIDEO_INCLUDE_VIDMAINSTATE_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file VIDMainState.h
*
* The VIDMainState class is responsible for handling all button and touch events,
* creating the touch button and playing the audio file.  VIDMainState is the
* only state in which the VideoApp can take on.
*
**
* author: 	leapfrog
*  			alindblad - 10/22/12 - Created copy from STUB app
*
******************************************************************************/

#include <GameStateBase.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <BlitBuffer.h>
#include <DisplayMPI.h>
#include <VideoMPI.h>
#include "LFDisplayRenderer.h"

// static values to indicate errors during initialization
typedef enum {
	kVIDError = -1,
	kVIDNoError
} VIDInitializationError;

class VIDVideoListener;

/*
 *  VIDMainState class.
 *
 * The only state of the Video app.  This class is responsible
 * for creating the touch button, handling touch and button events.
 */
class VIDMainState : public CGameStateBase {
 public:
	/*
	 *  VideoApp default constructor
	 *
	 *  Creates an app instance.
	 */
    VIDMainState(void);

	/*
	 *  VideoApp destructor
	 *
	 *  performs class cleanup, i.e.frees all memory allocated
	 */
    ~VIDMainState();

	/*
	 * Enter is called when the state first loads.
	 * All initial setup should take place here
	 * Enter is only called once per instance
	 */
    virtual void Enter(CStateData* userData);

	/*
	 * Update is called on every tick
	 * This is the heartbeat of the program
	 */
    virtual void Update(CGameStateHandler* sh);

	/*
	 * Suspend is called when AppManager needs to pause the game
	 * This could be triggered by a system pause from the pause button,
	 * a low battery warning, or by pushing a new app onto the AppManager stack (ie tutorial)
	 */
   virtual void Suspend();

	/*
	 * Resume is called upon returning from the suspended state
	 * This could be triggered by a system un-pause from the pause button,
	 * returning from the low battery warning, or returning from another app that
	 * was previously launched from the AppManager stack
	 */
    virtual void Resume();

	/*
	 * Exit is called when the state is destroyed
	 * All cleanup takes place here
	 * Exit is only called once per instance
	 * This could be triggered by quitting the game, plugging in the USB, turning the power off, etc
	 * Exit would not be called in the case of a catastrophic failure, such as sudden loss of power
	 */
    virtual void Exit();

    /*
     * HandleVideoDonePlaying
     *
     * This method is gets called from the video listener when a video
     * is done playing.
     */
    void HandleVideoDonePlaying(LeapFrog::Brio::tVideoHndl video_handle);

 protected:
	/*
	 * DoKeyEventLoop
	 *
	 * Checks for any physial button events and then handles an event appropriately
	 * Physical Video are the native Video to the device, i.e. the home button
	 */
    int DoKeyEventLoop(void);

    LeapFrog::Brio::CButtonEventQueue   button_event_queue_;   ///< a queue of all button events

    LeapFrog::Brio::CPath               video_path_;            ///< path to the video
    LeapFrog::Brio::tVideoHndl          video_handle_;          ///< handle to the video object
    LeapFrog::Brio::tVideoSurf          surface_;               ///< the surface the video is displayed to
    VIDVideoListener*                   video_listener_;        ///< video listener, responds to video completed events
    LeapFrog::Brio::tDisplayHandle 		video_display_handle_;	///< the display handle associated with the video

    LFDisplayRenderer                   renderer_;              ///< Handles screen buffering
    LeapFrog::Brio::CVideoMPI           video_mpi_;             ///< handles playing of video

 private:
    /*
	 * SetupVideo
	 *
	 * A helper method that performs all video playback related
	 * setup and initialization
	 */
    VIDInitializationError SetupVideo(void);

    /*
	 * PauseVideo
	 *
	 * A helper method that pauses the current video, if one exists
	 */
    void PauseVideo(void);

    /*
	 * ResumeVideo
	 *
	 * A helper method that resumes the current video, if one exists and it is paused
	 */
   void ResumeVideo(void);

   /*
	 * StopVideo
	 *
	 * A helper method that stops the current video, if one exists, and it is playing.
	 * Also sets the video_handle_ object to kInvalidVideoHndl
	 */
    void StopVideo(void);

    /*
 	 * KillVideo
 	 *
 	 * A helper method that cleans up the video related objects
 	 */
     void KillVideo(void);

    /*
	 * init
	 *
	 * A helper method that performs all initialization required by
	 * the touch button and audio used in this example.
	 */
    void Init(void);

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
	 * This method is called when the touch button event occurs and handles
	 * playing the audio file.
	 */
    void HandleTouchEvent(unsigned int id);
};

#endif  // VIDEO_VIDEO_INCLUDE_VIDMAINSTATE_H_

