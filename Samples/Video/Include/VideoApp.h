#ifndef VIDEO_VIDEO_INCLUDE_VIDEOAPP_H_
#define VIDEO_VIDEO_INCLUDE_VIDEOAPP_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file VideoApp.h
*
* The Video Sample App demonstrates how to:
* 	* Load a video based on screen size
* 	* Play a video with interleaved audio
**
* author: 	leapfrog
*  			alindblad - 10/22/12 - created copy from STUB app
*
******************************************************************************/

#include <AppInterface.h>
#include <GameStateHandler.h>
#include <DisplayMPI.h>

/*
 *  VideoApp class.
 *
 *  The VideoApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class VideoApp : public CAppInterface {
 public:
    /*
     *  VideoApp constructor
     *
     *  Creates an app instance.
     *
     *  @param userData
     */
    explicit VideoApp(void *userData);

    /*
     *  VideoApp destructor
     *
     *  Standard destructor, deallocates memory and cleanup of class object and members when the object is destroyed.
     *
     */
    ~VideoApp();

    void Enter();

    void Update();

    void Exit();

    void Suspend();

    void Resume();

    /*
     * @param state
     */
    static void RequestNewState(CGameStateBase* state);

 protected:
    CGameStateHandler *          state_handler_;        ///< manages the state of the app

    static bool                  new_state_;            ///< tracks whether there has been a state change
    static CGameStateBase*       new_state_requested_;  ///< pointer to the up-coming state object
    static CGameStateBase*       current_state_;        ///< pointer to the current state object

    LeapFrog::Brio::CDisplayMPI  display_mpi_;          ///< used to create the display handle
    LeapFrog::Brio::tDisplayHandle display_handle_;     ///< used for displaying Video
 private:
	/*
	 *  VideoApp copy constructor
	 *
	 *	Private and unimplemented to prevent copying
	 */
    VideoApp(const VideoApp&);

    /*
	 *  operator =
	 *
	 *	Private and unimplemented to prevent copying
	 */
    VideoApp& operator =(const VideoApp&);
};


#endif  // VIDEO_VIDEO_INCLUDE_VIDEOAPP_H_

