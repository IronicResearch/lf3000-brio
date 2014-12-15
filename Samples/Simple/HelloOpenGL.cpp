/******************************************************************************

 @File         OGLESHelloTriangle_LinuxX11.cpp

 @Title        OpenGL ES 1.x Hello Triangle Tutorial

 @Copyright    Copyright (C) 2000 - 2007 by Imagination Technologies Limited.

 @Platform     Brio framework.

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenGL ES
               1.x, use it for drawing a triangle and terminate it.

******************************************************************************/
#include <BrioOpenGLConfig.h>	
#include <ButtonMPI.h>
#include <DisplayTypes.h>
#include <DebugMPI.h>	
#include <EmulationConfig.h>
#include <EventListener.h>
#include <EventMPI.h>
#include <FontMPI.h>
#include <SystemTypes.h>
#include <string.h>
LF_USING_BRIO_NAMESPACE()


/******************************************************************************
 Defines
******************************************************************************/

// Defines to abstract float/fixed data for Common/CommonLite profiles
#ifdef OGLESLITE
#define VERTTYPE		GLfixed
#define VERTTYPEENUM	GL_FIXED
#define f2vt(x)			((int)((x)*65536))
#define myglLoadMatrix	glLoadMatrixx
#define myglClearColor	glClearColorx
#else
#define VERTTYPE		GLfloat
#define VERTTYPEENUM	GL_FLOAT
#define f2vt(x)			(x)
#define myglLoadMatrix	glLoadMatrixf
#define myglClearColor	glClearColor
#endif


//============================================================================
// Button Listener
//============================================================================

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;
const tEventType kMyHandledTypes[] = { kAllButtonEvents };

//----------------------------------------------------------------------------
class MyBtnEventListener : public IEventListener
{
 public:
	MyBtnEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)),
		dbg_(kMyApp) 
		{
			numPresses_ = 0;
		}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
		{
			type_ = msg.GetEventType();
			const CButtonMessage& btnmsg = reinterpret_cast<const CButtonMessage&>(msg);
			data_ = btnmsg.GetButtonState();
			if (data_.buttonState & data_.buttonTransition) 
				numPresses_++;
			return kEventStatusOKConsumed;
		}

	bool IsDone()
		{
			return (numPresses_ > 3) ? true : false;
		}

	bool IsToggled()
		{
			return (numPresses_ % 2) ? true : false;
		}

private:
	tEventType	type_;
	tButtonData	data_;
	int			numPresses_;
	CDebugMPI	dbg_;
};


/*!****************************************************************************
 @Function		TestEGLError
 @Input			pszLocation		location in the program where the error took
								place. ie: function name
 @Return		bool			true if no EGL error was detected
 @Description	Tests for an EGL error and prints it
******************************************************************************/
bool TestEGLError(const char* pszLocation)
{
	/*
		eglGetError returns the last error that has happened using egl,
		not the status of the last called function. The user has to
		check after every single egl call or at least once every frame.
	*/
	EGLint iErr = eglGetError();
	if (iErr != EGL_SUCCESS)
	{
		CDebugMPI	dbg(kMyApp);
		dbg.Assert(false, "%s failed (%d).\n", pszLocation, iErr);
		return false;
	}

	return true;
}

//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		/* LeapFrog::Brio:: */ EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif

//============================================================================
// Application Resource folder setup
//============================================================================
CPath GetAppRsrcFolder( )
{
#ifdef EMULATION
	return EmulationConfig::Instance().GetCartResourceSearchPath();
#else	// EMULATION
	return "/LF/Bulk/ProgramFiles/Simple/rsrc/";
#endif	// EMULATION
}
		

/*!****************************************************************************
 @Function		main
 @Input			argc		Number of arguments
 @Input			argv		Command line arguments
 @Return		int			result code to OS
 @Description	Main function of the program
******************************************************************************/
int main()
{
	// Setup OpenGL context
	BrioOpenGLConfig	config;
	
	// Setup button handler
	// NOTE: Order of events is significant on embedded target!
	CDebugMPI	dbg(kMyApp);
	CButtonMPI	button;				// 2
	MyBtnEventListener	handler;	// 3

	if (button.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return -1;
	}
	// Setup font
	CFontMPI			myFont;
//	tFontProp			myProp;
//	tFontAttr			myAttr;
	tFontSurf			mySurf;
	CString				myFontName = CString("DidjPropBold.ttf");			// font name

	// Tell FontMPI where to look for fonts.
	myFont.SetFontResourcePath(GetAppRsrcFolder());

	// Set font size properties explicitly
	myFont.LoadFont(myFontName, 18);

	// Set font drawing attributes explicitly
	myFont.SetFontColor(0xFF44CCFF);

	// Create pixmap buffer for font rendering which will be used as texture
	mySurf.width = 128;
	mySurf.height = 128;
	mySurf.pitch = 128 * 4;
	mySurf.format = kPixelFormatARGB8888;
	mySurf.buffer = static_cast<U8*>(malloc(mySurf.pitch * mySurf.height));
	memset(mySurf.buffer, 0, mySurf.pitch * mySurf.height);		
			
	// Setup texture the OpenGL ES way  
	GLuint 		myTex;
	glGenTextures(1, &myTex);
	glBindTexture(GL_TEXTURE_2D, myTex);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_2D);

	// Color shade effect with texture 	
	bool				bColorEffect = true; //false;

	// Variable set in the message handler to finish the demo
	bool				bDemoDone	= false;

	/*
		Step 8 - Draw something with OpenGL ES.
		At this point everything is initialized and we're ready to use
		OpenGL ES to draw something on the screen. 
	*/

	// Sets the clear color.
	// The colours are passed per channel (red,green,blue,alpha) as float values from 0.0 to 1.0
	myglClearColor(f2vt(0.6f), f2vt(0.8f), f2vt(1.0f), f2vt(1.0f)); // clear blue

	// Draws a triangle for 800 frames
	dbg.SetDebugLevel(kDbgLvlImportant);
	for(int i = 0; i < 800; ++i)
	{
		// Query button handler for state changes
		if (handler.IsDone())
			bDemoDone = true;

		// Check if the message handler finished the demo
		if (bDemoDone) break;

		/*
			Clears the color buffer.
			glClear() can also be used to clear the depth or stencil buffer
			(GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
		*/
		glClear(GL_COLOR_BUFFER_BIT);
		if (!TestEGLError("glClear"))
			return -1;

		/*
			Draw a quad as 2 triangles
			The vertex data is situated here for clarity but, for optimal performance, it must be located outside this loop. 
		*/

		// Vertex position (x,y,z)
		VERTTYPE pfVertices[] = {	f2vt(-0.5f),f2vt(-0.5f),f2vt(0.0f), 
									f2vt(-0.5f),f2vt(0.5f),f2vt(0.0f), 
									f2vt(0.5f),f2vt(0.5f),f2vt(0.0f),
									f2vt(0.5f),f2vt(-0.5f),f2vt(0.0f)
		};
		
		// Enable vertex arrays
		glEnableClientState(GL_VERTEX_ARRAY);
		
		/*
		   Set the vertex pointer. 
		   param 1: Number of coordinates per vertex; must be 2, 3, or 4.
		   param 2: GL_FIXED for CommonLite and GL_FLOAT for Common profile.
		   param 3: Specifies the byte offset between consecutive vertexes. Set to 0 for not interleaved data.
		   param 4: Pointer to the allocated data
		 */
		glVertexPointer(3,VERTTYPEENUM,0,pfVertices);

		// Set color data in the same way (red, green, blue, alpha)
		// Let's see shading by setting different colors per vertex. (DM)
		VERTTYPE pfColors[] = {	f2vt(0.0f), f2vt(0.0f), f2vt(0.66f), f2vt(1.0f), 
								f2vt(0.0f), f2vt(1.0f), f2vt(0.66f), f2vt(1.0f), 
								f2vt(1.0f), f2vt(1.0f), f2vt(0.66f), f2vt(1.0f), 
								f2vt(1.0f), f2vt(0.0f), f2vt(0.66f), f2vt(1.0f)};

		(bColorEffect) ? 
			glEnableClientState(GL_COLOR_ARRAY) : 
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glColorPointer(4,VERTTYPEENUM,0,pfColors);

		/*
			Draws a non-indexed triangle array from the pointers previously given.
			param 1: Primitive type: GL_POINTS, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, etc.
			param 2: The starting index in the array. 
			param 3: The nu	mber of indices to be rendered. 
		*/
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		if (!TestEGLError("glDrawArrays"))
			return -1;
		
		// Wait for rendering to finish and copy into pixmap buffer
		eglWaitGL();	
		glReadPixels(0, 0, mySurf.width, mySurf.height, GL_RGBA, GL_UNSIGNED_BYTE, mySurf.buffer);
		if (!TestEGLError("glReadPixels"))
			return -1;
		
		// Draw font string into pixmap buffer and download as texture
		myFont.DrawString(&myFontName, 0, i % (mySurf.height), &mySurf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mySurf.width, mySurf.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mySurf.buffer);
		
		// Setup texture map to quad
		VERTTYPE pfTexMap[] = {	f2vt(0.0f), f2vt(1.0f),  
								f2vt(0.0f), f2vt(0.0f),  
								f2vt(1.0f), f2vt(0.0f), 
								f2vt(1.0f), f2vt(1.0f)};
		
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		//glDisableClientState(GL_COLOR_ARRAY);
		glTexCoordPointer(2,VERTTYPEENUM,0,pfTexMap);
		
		// Draw quad again with texturing enabled and color disabled
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		if (!TestEGLError("glDrawArrays"))
			return -1;

		/*
			Swap Buffers.
			Brings to the native display the current render surface.
		*/
		eglWaitGL();
		eglSwapBuffers(config.eglDisplay, config.eglSurface);
		if (!TestEGLError("eglSwapBuffers"))
			return -1;

	}
	button.UnregisterEventListener(&handler);
	return 0;
}

/******************************************************************************
 End of file (OGLESHelloTriangle.cpp)
******************************************************************************/
