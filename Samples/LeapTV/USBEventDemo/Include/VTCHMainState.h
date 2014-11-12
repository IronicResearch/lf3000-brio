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

#include <GLES/gl.h>
#include "VirtualTouchApp.h"
#include <GameStateBase.h>
#include <DisplayMPI.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <Vision/VNVisionMPI.h>
#include <Hardware/HWControllerMPI.h>
#include <USBDeviceMPI.h>
#include <ImageIO.h>

#define REZ 1/8
#define GRID_SIZE_X (int)1280*REZ
#define GRID_SIZE_Y (int)720*REZ
#define MAX_RAD 20
#define FADE_STEPS 100

#define MAX_CONTROLLERS 1

namespace LeapFrog
{
	namespace Brio
	{
		class BrioOpenGLConfig;
	}
}

namespace LF
{
	namespace Vision
	{
		class VNRectHotSpot;
		class VNCircleHotSpot;
	}
}

struct MSPoint
{
	float x_;
	float y_;
	MSPoint(float x, float y) : x_(x), y_(y) { }
	MSPoint(const MSPoint& point) : x_(point.x_), y_(point.y_) { }
	MSPoint(void) : x_(0), y_(0) { }
};

typedef MSPoint MSVector;

struct MSSize
{
	float width_;
	float height_;
	MSSize(float width, float height) : width_(width), height_(height) { }
	MSSize(const MSSize& size) : width_(size.width_), height_(size.height_) { }
	MSSize(void) : width_(0), height_(0) { }
};

struct MSRect
{
	MSPoint origin_;
	MSSize size_;
	MSRect(MSPoint origin, MSSize size) : origin_(origin), size_(size) { }
	MSRect(const MSRect& rect) : origin_(rect.origin_), size_(rect.size_) { }
	MSRect(void) { }
};



class VTCHMainState : public CGameStateBase, public LeapFrog::Brio::IEventListener
{
	public:
		VTCHMainState(void);
		~VTCHMainState(void);
		virtual void Enter(CStateData* userData);
		virtual void Update(CGameStateHandler* sh);
		virtual void Suspend(void);
		virtual void Resume(void);
		virtual void Exit(void);

	protected:

		enum DrawState
		{
			kRects,
			kCircles,
			kNorm,
			kExp
		};

		virtual LeapFrog::Brio::tEventStatus Notify( const LeapFrog::Brio::IEventMessage &msgIn );

		void ButtonSelected(U16 item);
		void InitSample(void);
		void DrawSample(void);
		void InitDisplay(void);
		void CleanDisplay(void);
		void RegisterEventListeners(void);
		void UnRegisterEventListeners(void);
		void GetControllers(void);
		bool UpdateVisionAlgorithm(void);
		void SetupDisplaySurface(void);
		void SetupOGL(void);
		void SetupVisionAlgorithm(void);
		void DrawHotSpots(void);
		void SwitchDrawState();

		void RenderCircleHotSpot(const LF::Vision::VNRectHotSpot *rectHS);
		void RenderRectHotSpot(const LF::Vision::VNRectHotSpot *rectHS);
		void RenderExpHotSpot(const LF::Vision::VNRectHotSpot *rectHS);
		void RenderNormHotSpot(const LF::Vision::VNRectHotSpot *rectHS);

		void DrawRects(const LF::Vision::VNRectHotSpot *rectHS);

		void SetWindowPosition(void);
		void CleanUpHotSpots(void);
		void CleanUpAlgorithm(void);
		void GetErrorArt(void);
		MSSize ImageSize(void);
		void UpdateVertices(void);
		void Render(void);
		void SetFrame(const MSRect& rect);
		void SetupError(void);
		void SetTranslation(const MSVector& vec);
		void Init(void);
		void SetupErrorOGL(void);

		LeapFrog::Brio::CUSBDeviceMPI	usbDeviceMPI_;
		LF::Hardware::HWControllerMPI controller_mpi_;

		CImageIO        image_io_;     ///< loads image data

		LeapFrog::Brio::CPath  path_;         ///< path for image to display
		tVideoSurf             image_surf_;   ///< keeps meta data of the image
		GLuint                 texture_;      ///< handle for opengl texture
		MSRect                 frame_;
		float                  etexcoord_[8];  ///< Corresponding texture coordinates
		float                  evertices_[8];
		MSVector     translation_;
		MSSize size;

		struct ControllerData
		{
			float x_, y_, lx_, ly_;  ///< analog stick data
			float x_pos_, y_pos_, z_pos_; ///< position for pyramid indicator object
			float dx_, dy_, dz_; ///< incremental updates to the position
			int   rotationY_, rotationX_, rotationZ_; ///< rotation of the pyramid indicator object
			LeapFrog::Brio::S32 ax_,ay_,az_;
			LeapFrog::Brio::S32 lax_, lay_, laz_;
			bool analog_stick_on_; ///< flag indicating if we are using the analog stick
		};

//		LeapFrog::Brio::CButtonEventQueue buttonEventQueue_;
		LeapFrog::Brio::CDisplayMPI displayManager_;
		int screenWidth_, screenHeight_;
		int targetWidth_, targetHeight_;

		LeapFrog::Brio::BrioOpenGLConfig* brioOGLConfig_;

		LeapFrog::Brio::tDisplayHandle displayHandle_;
		unsigned char* rgbBuffer_;

		class HotSpotListener : public IEventListener
		{
			public:
				HotSpotListener(void);
				tEventStatus Notify(const IEventMessage& msg);
		};

		HotSpotListener hotspotListener_;
		LF::Vision::VNVisionMPI  *visionMPI_;
		LF::Vision::VNAlgorithm *visionAlgorithm_;
		std::vector<const LF::Vision::VNHotSpot*> hotSpots_;
		LeapFrog::Brio::tRect displayFrame_;

		tVideoSurf videoDisplaySurface_;
		float  texcoord_[8];  ///< Corresponding texture coordinates
		float  vertices_[8];  ///< on screen coordinates for texture rendering

		ControllerData controllerData[MAX_CONTROLLERS];
		int numControllers;

		bool isCameraConnected;
		bool errorDisplayed;

		DrawState drawState;

		struct PointData
		{
			float rad;
			float dx, dy;
		};

		PointData pointData[GRID_SIZE_X*GRID_SIZE_Y];

		int switchStateWait;
};



#endif // __VTCHMAINSTATE_H__

