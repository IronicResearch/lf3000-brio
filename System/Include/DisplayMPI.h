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
	
	// Returns number of display output screens
	U16							GetNumberOfScreens() const;
	
	// Returns screen info for the selected output device (width, height, pixel format, description)
	const tDisplayScreenStats*	GetScreenStats(tDisplayScreen screen) const;
	
	// Updates the display output, either selected rectangular region or entire screen
	tErrType					Invalidate(tDisplayScreen screen, tRect *pDirtyRect = NULL);

	// Graphics contexts (through tDisplayHandle)
	//===========================================
	
	// Creates a display surface context for access by the application
	tDisplayHandle      CreateHandle(U16 height, U16 width, tPixelFormat colorDepth, 
									U8 *pBuffer = NULL);

	// Returns the buffer address in user memory space of the display surface
	U8*                 GetBuffer(tDisplayHandle hndl) const;

	// Returns the pixel format enum of the display surface
	tPixelFormat		GetPixelFormat(tDisplayHandle hndl) const;
	
	// Returns the bytes per scanline pitch (stride) of the display surface
	U16                 GetPitch(tDisplayHandle hndl) const;
	
	// Returns the bits per pixel depth of the display surface
	U16                 GetDepth(tDisplayHandle hndl) const;

	// Returns the height of the display surface
	U16                 GetHeight(tDisplayHandle hndl) const;
	
	// Returns the width of the display surface
	U16                 GetWidth(tDisplayHandle hndl) const;
	
	// Positions the display surface on the screen	
	tErrType            Register(tDisplayHandle hndl, S16 xPos = 0, S16 yPos = 0, 
								 tDisplayHandle insertAfter = 0, 
								 tDisplayScreen screen = kDisplayScreenAllScreens);

	// Positions the display surface on the screen	
	tErrType            Register(tDisplayHandle hndl, S16 xPos = 0, S16 yPos = 0, 
								 tDisplayZOrder initialZOrder = kDisplayOnTop, 
	                             tDisplayScreen screen = kDisplayScreenAllScreens);

	// Removes the display surface from the screen
	tErrType            UnRegister(tDisplayHandle hndl, tDisplayScreen screen);
	
	// Destroys the display surface context created by CreateHandle()
	tErrType            DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer);
	
	// Locks the display surface framebuffer memory for exclusive access by the application
	tErrType            LockBuffer(tDisplayHandle hndl);
	
	// Unlocks the display surface framebuffer memory locked by LockBuffer()
	tErrType            UnlockBuffer(tDisplayHandle hndl, tRect *pDirtyRect = NULL);
	
	// Sets the alpha transparency value for the display surface layer
	tErrType			SetAlpha(tDisplayHandle hndl, U8 level, 
								Boolean enable=true);

	// Returns the alpha transparency value of the display surface layer
	U8                  GetAlpha(tDisplayHandle hndl) const;

	// OpenGL context interface
	//=========================

	// Initializes the display manager for OpenGL rendering contexts
	void				InitOpenGL(void* pCtx);
	
	// Shutdown display manager OpenGL context setup by InitIOpenGL()
	void				DeinitOpenGL();
	
	// Enables OpenGL hardware 3D engine rendering output to display surface layer
	void				EnableOpenGL(void* pCtx);
	
	// Disables OpenGL hardware 3D engine rendering output to display surface layer
	void				DisableOpenGL();

	// Updates OpenGL hardware 3D engine rendering output to display surface layer
	void				UpdateOpenGL();

#ifdef LF1000
	// (LF1000)
	void				WaitForDisplayAddressPatched(void);

	// (LF1000)
	void				SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress);
#endif

	// Brightness and contrast controls
	//=================================

	// Sets the brightness of the display output screen (0 = default setting)
	tErrType			SetBrightness(tDisplayScreen screen, S8 brightness);
	
	// Sets the contrast of the display output screen (0 = default setting)
	tErrType			SetContrast(tDisplayScreen screen, S8 contrast);
	
	// Gets the brightness setting of the display output screen
	S8					GetBrightness(tDisplayScreen screen);
	
	// Gets the contrast setting of the display output screen
	S8					GetContrast(tDisplayScreen screen);
	
private:
	class CDisplayModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYMPI_H

// eof
