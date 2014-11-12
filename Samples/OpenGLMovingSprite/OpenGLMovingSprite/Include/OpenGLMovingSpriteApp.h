#ifndef GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_OPENGLMOVINGSPRITEAPP_H_
#define GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_OPENGLMOVINGSPRITEAPP_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file OpenGLMovingSpriteApp.h
*
* The OpenGLMovingSprite sample app demonstrates how to:
*	* load images via CImageIO
*	* Render the images as a simple texture (sprite) in OpenGL
*	* Render animations as a series of sprites
*	* Create device independent frame rate
* Note:
* 	If this sample app does not function properly you may have to install the
* 	CImageIO patch lib to your device.  The patch lib was included in the zip
* 	file for this project and is named CImageIO_PatchLib.zip.  Unzip that file
* 	and move the .lf2 file on to your device using OmegaTerm.
**
* author: 	leapfrog
*  			alindblad - 10/22/12 - created copy from STUB app
*
******************************************************************************/

#include <AppInterface.h>
#include <GameStateHandler.h>

/*
 *  OpenGLMovingSpriteApp class.
 *
 *  The OpenGLMovingSpriteApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class OpenGLMovingSpriteApp : public CAppInterface {
 public:
    /*
     *  OpenGLMovingSpriteApp constructor
     *
     *  Creates an app instance.
     *
     *  @param userData
     */
    explicit OpenGLMovingSpriteApp(void *userData);

    /*
     *  OpenGLMovingSpriteApp destructor
     *
     *  Standard destructor, deallocates memory and cleanup of class object and members when the object is destroyed.
     *
     */
    ~OpenGLMovingSpriteApp();

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
    CGameStateHandler *                 state_handler_;        ///< manages the state of the app

    static bool                         new_state_;            ///< tracks whether there has been a state change
    static CGameStateBase*              new_state_requested_;  ///< pointer to the up-coming state object
    static CGameStateBase*              current_state_;        ///< pointer to the current state object

 private:
	/*
	 *  OpenGLMovingSpriteApp copy constructor
	 *
	 *	Private and unimplemented to prevent copying
	 */
    OpenGLMovingSpriteApp(const OpenGLMovingSpriteApp&);

    /*
	 *  operator =
	 *
	 *	Private and unimplemented to prevent copying
	 */
    OpenGLMovingSpriteApp& operator =(const OpenGLMovingSpriteApp&);
};


#endif  // GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_OPENGLMOVINGSPRITEAPP_H_

