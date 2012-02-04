#ifndef LF_BRIO_DISPLAYMPI_H
#define LF_BRIO_DISPLAYMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Display module. 
//
//==============================================================================
#include <CoreMPI.h>
#include <DisplayTypes.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

/// \class CDisplayMPI
///
/// The Display manager module is intended to provide services to acesss framebuffer
/// regions, select RGB, ARGB, or YUV display formats, and adjust display specific
/// features like LCD brightness, contrast, and backlight levels.
/// 
/// The Display MPI manages framebuffer regions as display contexts which are intended
/// for use by other functional modules such as OpenGL, Video, or Font renderers.
/// The original Display MPI abstraction model expected to create display contexts
/// in a particular size and pixel format via CreateHandle(), and then register them in
/// their Z order for updating the display. All registered display contexts would
/// then be composited to the display via Invalidate() calls. 
/// 
/// Although the LF1000 hardware has no 2D acclerator for blitting framebuffer regions, 
/// it does contain a multi-layer controller for compositing framebuffer regions in
/// layered RGB or YUV formats. The Register() function will attempt to assign the 
/// Z order of such onscreen contexts accordingly and make them visible on the display,
/// and UnRegister() will remove them from being displayed.
///
/// Additional Display MPI functions SwapBuffers(), IsBufferSwapped(), and 
/// GetCurrentDisplayHandle() are provided for page-flipping support. With page-flipping 
/// functionality, the Brio application has the option for rendering a 2D buffer offscreen 
/// and then have it displayed onscreen at the next VSync update without visible tearing or 
/// flickering.
///
/// On the LF1000 platform, up to 3 RGB buffers of the same fullscreen format may be 
/// allocated to support triple-buffered page-flipping. 2 RGB buffers may be allocated 
/// for double-buffered page-flipping if that is simpler to manage. When either 2 or 3 
/// RGB buffers are allocated via CreateHandle(), then framebuffer memory used for YUV 
/// video will be unavailable in that case. Once all available framebuffer regions are 
/// allocated, CreateHandle() will return an invalid handle (NULL) until these contexts 
/// are released via corresponding calls to DestroyHandle().
/// 
/// Whenever rendering to one of these buffers is completed, SwapBuffers() can be called 
/// to update it onscreen at the next VSync interval. SwapBuffers() can optionally wait 
/// until the VSync occurs before returning, which would be preferable for double-buffering 
/// case. IsBufferSwapped() can also be optionally called prior to the next update, which 
/// would be more typical of triple-buffering case.
/// 
/// The current visible onscreen display context can be queried via GetCurrentDisplayHandle(). 
/// This corresponds to the most recent context passed to SwapBuffers() when page-flipping 
/// is used, or the topmost onscreen context passed to Register() otherwise.
///
/// On Emerald LF1000 platform, framebuffer memory is now managed as a heap,
/// so RGB, YUV, and OGL buffers can be more flexibly allocated as needed.
/// Framebuffer used and free memory can be tracked via GetDisplayMem() API.
/// 
/// RGB and YUV buffers are allocated directly via CreateHandle() API.
/// OGL buffers are allocated internally via BrioOpenGLConfig() object, whose
/// constructor allows configuring 1D/2D heap sizes for the OpenGL ES library.
///
/// The LF1000 hardware has requirements on OGL and YUV buffers to be multiples
/// of 1Meg blocks with 4K pitch, so buffer allocations will be aligned accordingly.

//==============================================================================
class CDisplayMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CDisplayMPI();
	virtual ~CDisplayMPI();

	// Screen functionality
	//=====================
	
	/// Returns number of display output screens
	U16							GetNumberOfScreens() const;
	
	/// Returns screen info for the selected output device (width, height, pixel format, description)
	///
	/// \param screen 	The selected output screen
	///
	/// \return 		Returns pointer to display screen struct filled in.
	const tDisplayScreenStats*	GetScreenStats(tDisplayScreen screen) const;
	
	/// Updates the display output, either selected rectangular region or entire screen
	///
	/// \param screen 	The selected output screen
	///
	/// \param pDirtyRect 	The destination rectangle to be updated if non-NULL,
	/// or NULL for the entire screen.
	///
	/// \return 		Returns kNoErr on success. 
	tErrType					Invalidate(tDisplayScreen screen, tRect *pDirtyRect = NULL);

	// Graphics contexts (through tDisplayHandle)
	//===========================================
	
	/// Creates a display surface context for access by the application
	/// 
	/// \param height 	The height in pixels of the display context
	///
	/// \param width 	The width in pixels of the display context
	///
	/// \param colorDepth 	The pixel format enum of the display context
	///
	/// \param pBuffer 	The pointer to an offscreen buffer created by the caller,
	/// or NULL for an onscreen buffer to be created by the driver.
	///
	/// \return 		Returns valid display context handle on success, 
	/// or kInvalidDisplayHndl on failure.
	tDisplayHandle      CreateHandle(U16 height, U16 width, tPixelFormat colorDepth, 
									U8 *pBuffer = NULL);

	/// Returns the buffer address in user memory space of the display surface
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	U8*                 GetBuffer(tDisplayHandle hndl) const;

	/// Returns the pixel format enum of the display surface
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	tPixelFormat		GetPixelFormat(tDisplayHandle hndl) const;
	
	/// Returns the bytes per scanline pitch (stride) of the display surface
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	U16                 GetPitch(tDisplayHandle hndl) const;
	
	/// Returns the bits per pixel depth of the display surface
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	U16                 GetDepth(tDisplayHandle hndl) const;

	/// Returns the height of the display surface
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	U16                 GetHeight(tDisplayHandle hndl) const;
	
	/// Returns the width of the display surface
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	U16                 GetWidth(tDisplayHandle hndl) const;
	
	/// Positions the display surface on the screen
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \param xPos		The X destination position
	///
	/// \param yPos		The Y destination position
	///
	/// \param insertAfter	The display handle previously registered to insert after 
	/// in the linked list, or the end of the list if kNull.
	///
	/// \param screen 	The selected output screen
	///
	/// \return 		Returns kNoErr on success. 
	tErrType            Register(tDisplayHandle hndl, S16 xPos = 0, S16 yPos = 0, 
								 tDisplayHandle insertAfter = 0, 
								 tDisplayScreen screen = kDisplayScreenAllScreens);

	/// Positions the display surface on the screen	
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \param xPos		The X destination position
	///
	/// \param yPos		The Y destination position
	///
	/// \param initialZOrder	The Z order to insert in the linked list.
	///
	/// \param screen 	The selected output screen
	///
	/// \return 		Returns kNoErr on success. 
	tErrType            Register(tDisplayHandle hndl, S16 xPos = 0, S16 yPos = 0, 
								 tDisplayZOrder initialZOrder = kDisplayOnTop, 
	                             tDisplayScreen screen = kDisplayScreenAllScreens);

	/// Removes the display surface from the screen
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \param screen 	The selected output screen
	///
	/// \return 		Returns kNoErr on success. 
	tErrType            UnRegister(tDisplayHandle hndl, tDisplayScreen screen);
	
	/// Destroys the display surface context created by CreateHandle()
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \param destroyBuffer	(not used)
	///
	/// \return			Returns kNoErr on success.
	tErrType            DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer);
	
	/// Locks the display surface framebuffer memory for exclusive access by the application
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// (LF1000 platform does nothing since there is no 2D accelerator to synchronize access)
	tErrType            LockBuffer(tDisplayHandle hndl);
	
	/// Unlocks the display surface framebuffer memory locked by LockBuffer()
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \param pDirtyRect	(not used)
	///
	/// (LF1000 platform does nothing since there is no 2D accelerator to synchronize access)
	tErrType            UnlockBuffer(tDisplayHandle hndl, tRect *pDirtyRect = NULL);
	
	/// Swaps the display with the selected fullscreen context, optionally waiting for vertical sync
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \param waitVSync	Option to wait for VSync before returning
	///
	/// \return			Returns kNoErr on success.
	tErrType			SwapBuffers(tDisplayHandle hndl, Boolean waitVSync = false);

	/// Returns true when the previous context passed to SwapBuffers() has been updated to the display
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \return			Returns true if the buffer has been updated, false if still pending.
	Boolean				IsBufferSwapped(tDisplayHandle hndl);

	/// Returns the handle for the display context currently visible onscreen
	tDisplayHandle		GetCurrentDisplayHandle();
	
	/// Sets the alpha transparency value for the display surface layer
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \param level	The global alpha value for the display context (0 to 100)
	///
	/// \param enable	Enable alpha blending for the display context
	///
	/// \return			Returns kNoErr on success.
	///
	/// LF1000: Global alpha value effective for RGB, YUV pixel formats only, not ARGB.
	/// Per-pixel alpha values effective instead when enabled for ARGB display contexts.
	/// Has no effect on emulation.
	tErrType			SetAlpha(tDisplayHandle hndl, U8 level, 
								Boolean enable=true);

	/// Returns the alpha transparency value of the display surface layer
	/// 
	/// \param hndl		The display handle returned by CreateHandle()
	///
	/// \return			Returns the global alpha value set via SetAlpha().
	U8                  GetAlpha(tDisplayHandle hndl) const;

	// OpenGL context interface (internal Brio use only)
	//=========================

	// Internal Brio use only. Not for use by developers.
	// Initializes the display manager for OpenGL rendering contexts
	void				InitOpenGL(void* pCtx);
	
	// Internal Brio use only. Not for use by developers.
	// Shutdown display manager OpenGL context setup by InitIOpenGL()
	void				DeinitOpenGL();
	
	// Internal Brio use only. Not for use by developers.
	// Enables OpenGL hardware 3D engine rendering output to display surface layer
	void				EnableOpenGL(void* pCtx);
	
	// Internal Brio use only. Not for use by developers.
	// Disables OpenGL hardware 3D engine rendering output to display surface layer
	void				DisableOpenGL();

	// Internal Brio use only. Not for use by developers.
	// Updates OpenGL hardware 3D engine rendering output to display surface layer
	void				UpdateOpenGL();

	// (LF1000)
	// Internal Brio use only. Not for use by developers.
	void				WaitForDisplayAddressPatched(void);

	// (LF1000)
	// Internal Brio use only. Not for use by developers.
	void				SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress);

	// Brightness and contrast controls
	//=================================

	/// \deprecated		Brightness/Contrast support No longer implemented
	///
	/// \return			Returns kNoImplErr
	tErrType			SetBrightness(tDisplayScreen screen, S8 brightness);
	
	/// \deprecated		Brightness/Contrast support No longer implemented
	///
	/// \return			Returns kNoImplErr
	tErrType			SetContrast(tDisplayScreen screen, S8 contrast);
	
	/// Sets the backlight of the display output screen (-128..127 range setting)
	///
	/// \param screen 	The selected output screen
	///
	/// \param brightness	The backlight value in range -128 to 127
	///
	/// \return			Returns kNoErr on success.
	tErrType			SetBacklight(tDisplayScreen screen, S8 brightness);
	
	/// \deprecated		Brightness/Contrast support No longer implemented
	///
	/// \return			Returns 0
	S8					GetBrightness(tDisplayScreen screen);
	
	/// \deprecated		Brightness/Contrast support No longer implemented
	///
	/// \return			Returns 0
	S8					GetContrast(tDisplayScreen screen);
	
	/// Gets the backlight setting of the display output screen
	///
	/// \param screen 	The selected output screen
	///
	/// \return			Returns the backlight value set via SetBacklight().
	S8					GetBacklight(tDisplayScreen screen);

	/// Gets available display memory allocation info.
	///
	/// \param memtype	tDisplayMem type of memory allocation requested
	///
	/// \return			Returns the requested display memory allocation info.
	U32					GetDisplayMem(tDisplayMem memtype);

	/// Sets the window position and size for selected display handle.
	///
	/// \param hndl		Display handle for onscreen context
	/// \param x		Window position X
	/// \param y		Window position Y
	/// \param width	Window width relative to X
	/// \param height	Window height relative to Y
	///
	/// \return			Returns kNoErr on success, or kInvalidParamErr for invalid handle.
	///
	/// Overides default window size and position determined by CreateHandle() and Register().
	tErrType			SetWindowPosition(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height, Boolean visible);

	/// Gets the window position and size for selected display handle.
	///
	/// \param hndl		Display handle for onscreen context
	/// \param x		Window position X
	/// \param y		Window position Y 
	/// \param width	Window width relative to X
	/// \param height	Window height relative to Y
	///
	/// \return			Returns kNoErr on success, or kInvalidParamErr for invalid handle.
	tErrType			GetWindowPosition(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height, Boolean& visible);
	
	/// Returns a PixelFormat of an available layer
	///
	///  \returns		PixelFormat of an available layer (kPixelFormatYUV420 or kPixelFormatARGB8888).
	///					kPixelFormatError returned if no layers are free.
	tPixelFormat		GetAvailableFormat();

	/// Sets the source video scaler size for the selected YUV video context.
	///
	/// \param hndl		Display handle for YUV video context
	/// \param width	Source video width
	/// \param height	Source video height
	/// \param centered	Source video centered instead of scaled to window size
	///
	/// \return			Returns kNoErr on success, or kInvalidParamErr for invalid handle.
	///
	/// LF1000: Sets the YUV HW video source:destination scaler for the video context.
	/// By default, the source and destination width and height are set to the same values 
	/// as the video context width and height for 1:1 scaling. The video source size may be
	/// overidden to smaller values for scaling up to the destination window size, or
	/// larger values for scaling down.
	tErrType 			SetVideoScaler(tDisplayHandle hndl, U16 width, U16 height, Boolean centered);

	/// Gets the source video scaler size for the selected YUV video context.
	///
	/// \param hndl		Display handle for YUV video context
	/// \param width	Source video width
	/// \param height	Source video height
	/// \param centered	Source video centered instead of scaled to window size
	///
	/// \return			Returns kNoErr on success, or kInvalidParamErr for invalid handle.
	tErrType 			GetVideoScaler(tDisplayHandle hndl, U16& width, U16& height, Boolean& centered);

	// Internal Brio use only. Not for use by developers.
	// Sets/Gets active display/touch viewport via enum
	tErrType			SetViewport(tDisplayHandle hndl, tDisplayViewport viewport);
	tDisplayViewport	GetViewport(tDisplayHandle hndl);

	// Internal Brio use only. Not for use by developers.
	// Sets/Gets active display/touch viewport via x,y position and w,h size
	tErrType			SetViewport(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height);
	tErrType			GetViewport(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height);

	// Internal Brio use only. Not for use by developers.
	// Sets/Gets effective display/touch viewport orientation (for Flash player use)
	tErrType			SetOrientation(tDisplayHandle hndl, tDisplayOrientation orient);
	tDisplayOrientation	GetOrientation(tDisplayHandle hndl);

	// Internal Brio use only. Not for use by developers.
	// Sets/Gets effective auto-rotation control on orientation event changes (for Flash player use)
	tErrType			SetAutoRotation(Boolean enable);
	Boolean				GetAutoRotation();

	/// Returns the handle for the active display context matching the pixelformat
	tDisplayHandle		GetCurrentDisplayHandle(tPixelFormat pixelformat);

private:
	class CDisplayModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYMPI_H

// eof
