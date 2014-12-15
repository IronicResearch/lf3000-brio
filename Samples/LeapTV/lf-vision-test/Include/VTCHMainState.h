#ifndef __VTCHMAINSTATE_H__
#define __VTCHMAINSTATE_H__

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file VTCHMainState.h
*
* The VTCHMainState class is responsible for creating, setting up and
* handling all wand related tracking.  This includes the algorithm, the
* hot spots and the hot spot listener.
*
* author: leapfrog
*          alindblad - 1/3/14 - created class
*
******************************************************************************/

#include "VirtualTouchApp.h"
#include <GameStateBase.h>
#include <DisplayMPI.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <Vision/VNVisionMPI.h>
#include <GLES/gl.h>


namespace LeapFrog {
namespace Brio {
  class BrioOpenGLConfig;
}
}

namespace LF {
  namespace Vision {
    class VNRectHotSpot;
    class VNCircleHotSpot;
  }
}

class VTCHMainState : public CGameStateBase {
public:
  VTCHMainState(void);
  ~VTCHMainState(void);
  
  /*!
   * Enter is called when the state first loads.
   * All initial setup should take place here
   * Enter is only called once per instance
   */
  virtual void Enter(CStateData* userData);
  
  /*!
   * Update is called on every tick
   * This is the heartbeat of the program
   */
  virtual void Update(CGameStateHandler* sh);
  
  /*!
   * Suspend is called when AppManager needs to pause the game
   * This could be triggered by a system pause from the pause button,
   * a low battery warning, or by pushing a new app onto the AppManager
   *  stack (ie tutorial)
   */
  virtual void Suspend(void);
  
  /*!
   * Resume is called upon returning from the suspended state
   * This could be triggered by a system un-pause from the pause button,
   * returning from the low battery warning, or returning from another 
   * app that was previously launched from the AppManager stack
   */
  virtual void Resume(void);
  
  /*!
   * Exit is called when the state is destroyed
   * All cleanup takes place here
   * Exit is only called once per instance
   * This could be triggered by quitting the game, plugging in the USB, 
   * turning the power off, etc  Exit would not be called in the case 
   * of a catastrophic failure, such as sudden loss of power
   */
  virtual void Exit(void);
  
 protected:
  int DoKeyEventLoop(void);  
  void ButtonSelected(U16 item);
  void InitSample(void);
  void DrawSample(void);
  void InitDisplay(void);
  void CleanDisplay(void);
  void RegisterEventListeners(void);
  void UnRegisterEventListeners(void);
  void UpdateVisionAlgorithm(void);
  void SetupDisplaySurface(void);
  void SetupOGL(void);
  void CreateCameraTexture(void);
  void SetupVisionAlgorithm(void);
  void DrawCameraBuffer(void);
  void DrawHotSpots(void);
  void RenderRectHotSpot(const LF::Vision::VNRectHotSpot *rectHS);
  void RenderCircleHotSpot(const LF::Vision::VNCircleHotSpot *cHS);

  LeapFrog::Brio::CButtonEventQueue buttonEventQueue_; 
  LeapFrog::Brio::CDisplayMPI displayManager_;
  int screenWidth_, screenHeight_;
  int targetWidth_, targetHeight_;

  LeapFrog::Brio::BrioOpenGLConfig* brioOGLConfig_;
 
  LeapFrog::Brio::tDisplayHandle displayHandle_;
  unsigned char* rgbBuffer_;
  
  class HotSpotListener : public IEventListener {
  public:
    HotSpotListener(void);
    tEventStatus Notify(const IEventMessage& msg);
  };

  HotSpotListener hotspotListener_;
  LF::Vision::VNVisionMPI  visionMPI_;
  LF::Vision::VNAlgorithm *visionAlgorithm_;
  std::vector<const LF::Vision::VNHotSpot*> hotSpots_;
  LeapFrog::Brio::tRect displayFrame_;
  
  tVideoSurf videoDisplaySurface_;
  GLuint oglCameraTexture_;
  float  texcoord_[8];  ///< Corresponding texture coordinates
  float  vertices_[8];  ///< on screen coordinates for texture rendering
};



#endif // __VTCHMAINSTATE_H__

