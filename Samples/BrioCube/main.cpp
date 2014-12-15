#include <AudioMPI.h>
#include <AudioTypes.h>
#include <BrioOpenGLConfig.h>
#include <ButtonMPI.h>
#include <DebugMPI.h>
#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <FontMPI.h>
#include <KernelMPI.h>
#include <SystemTypes.h>
#include <math.h>
#include <string.h>

LF_USING_BRIO_NAMESPACE()
#include "render.h"



//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static CPath emurootPath = CPath(getenv("HOME"))+CPath("/emuroot");
	static bool gInitialized = EmulationConfig::Instance().Initialize(emurootPath.c_str());
#endif

//============================================================================
// Application Resource folder setup
//============================================================================
CPath GetAppRsrcFolder( )
{
#ifdef EMULATION
	return EmulationConfig::Instance().GetCartResourceSearchPath();
#else	// EMULATION
	return "/LF/Bulk/ProgramFiles/BrioCube/rsrc/";
#endif	// EMULATION
}
	
//Some useful global handles
// FIXME/tp: From original code, cleanup!
GLboolean drawInOrtho = GL_TRUE; //This variable will change with every click in the touch screen of the pda
GLboolean draw3dText = GL_FALSE; //Supported only if font can be loaded
EGLDisplay glesDisplay;  // EGL display
EGLSurface glesSurface;	 // EGL rendering surface
EGLContext glesContext;	 // EGL rendering context
const tDebugSignature kMyApp = kFirstCartridge1DebugSig;


//============================================================================
// Button Listener
//============================================================================
const tEventType kMyControllerTypes[] = { kAllButtonEvents, kAllAudioEvents };
const U32		 kDelayTime = 5000;

//----------------------------------------------------------------------------
class TransitionController : public IEventListener
{
public:
	TransitionController( )
		: IEventListener(kMyControllerTypes, ArrayCount(kMyControllerTypes)),
		dbgMPI_(kMyApp)
	{
		isDone_ = false;
		isMusicOn_ = false;
		isSineOn_ = false;
	
		startTime_ = kernel_.GetElapsedTimeAsMSecs();
		debugLevel_ = dbgMPI_.GetDebugLevel();
		
		//audioMPI_.SetAudioResourcePath(GetAppRsrcFolder());

	}
	
	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{
		if( msgIn.GetEventType() == kAudioCompletedEvent )
		{	
			const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
			const tAudioMsgAudioCompleted& data = msg.audioMsgData.audioCompleted;
			dbgMPI_.DebugOut(kDbgLvlCritical, "Audio done: id=%d, payload=%d, count=%d\n", 
					static_cast<int>(data.audioID), static_cast<int>(data.payload), static_cast<int>(data.count));
	
			// Change the flag for completing audio resources.
			if (data.audioID == sineID_) {
				isSineOn_ = false;
			} else if (data.audioID == musicID_) {
				isMusicOn_ = false;
			} else {
				dbgMPI_.DebugOut(kDbgLvlCritical, "BrioCube -- Mystery audioID completed, going unhandled!\n");
			}
		} 
		else if( msgIn.GetEventType() == kButtonStateChanged )
		{	
			const CButtonMessage& msg = dynamic_cast<const CButtonMessage&>(msgIn);
			tButtonData data = msg.GetButtonState();
			char ch = (data.buttonState & data.buttonTransition) ? '+' : '-';
			switch( data.buttonTransition )
			{
				case kButtonPause:
					if (ch == '+')
						isDone_ = true;
					break;
				case kButtonUp:
					if( ch == '+' && debugLevel_ <= kMaxDebugLevel )
						debugLevel_ = static_cast<tDebugLevel>(debugLevel_+1);
					dbgMPI_.SetDebugLevel(debugLevel_);
					dbgMPI_.DebugOut(kDbgLvlCritical, "%cKeyDown, Debug level=%d\n", ch, debugLevel_);
					break;
				case kButtonDown:
					if( ch == '+' && debugLevel_ >= 0 )
						debugLevel_ = static_cast<tDebugLevel>(debugLevel_-1);
					dbgMPI_.SetDebugLevel(debugLevel_);
					dbgMPI_.DebugOut(kDbgLvlCritical, "%cKeyUP, Debug level=%d\n", ch, debugLevel_);
					break;
				case kButtonRight:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyRight, forcing transition\n", ch);
					ForceTransition();
					break;
				case kButtonLeft:
					dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyLeft, forcing transition\n", ch);
					ForceTransition();
					break;
				case kButtonA:
					if( ch == '+' ) {
						if (!isMusicOn_) {
							//musicID_ = audioMPI_.StartAudio( "BlueNile44m.ogg", 100, 1, 0, this, 0, kAudioOptionsDoneMsgAfterComplete );
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio audioID = 0x%x\n", 
												ch, static_cast<unsigned int>(musicID_) );
							isMusicOn_ = true;
						} else {
							audioMPI_.StopAudio( musicID_, false );
							isMusicOn_ = false;
						}
					}	
					break;
				case kButtonB:
					break;
				case kButtonHint:
					if( ch == '+' ) {
						if (!isSineOn_) {
							//sineID_ = audioMPI_.StartAudio( "SINE44.ogg", 100, 1, 0, this, 0, kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped );
							dbgMPI_.DebugOut(kDbgLvlVerbose, "%cKeyA, StartAudio audioID = 0x%x\n", ch, 
									static_cast<unsigned int>(sineID_) );
							isSineOn_ = true;
						} else {
							audioMPI_.StopAudio( sineID_, false );
							isSineOn_ = false;
						}
					}	
					break;
				case kButtonRightShoulder:
				{
					dbgMPI_.DebugOut(kDbgLvlCritical, "%cVolumeUP\n", ch);
					break;
				}
				case kButtonLeftShoulder:
				{
					dbgMPI_.DebugOut(kDbgLvlCritical, "%cVolumeDOWN\n", ch);
					break;
				}
			}
		}
			return kEventStatusOKConsumed;
	}
	
	bool IsDone()
	{
		return isDone_;
	}

	bool ShouldTransition()
	{
		U32 now = kernel_.GetElapsedTimeAsMSecs();
		if( (now - startTime_) > kDelayTime )
		{
			startTime_ = now;
			return true;
		}
		return false;
	}

	void ForceTransition()
	{
		startTime_ = 0;		//reset to force transition
	}

private:
	
	CDebugMPI		dbgMPI_;
	CAudioMPI		audioMPI_;

	bool			isDone_;
	bool			isMusicOn_;
	bool			isSineOn_;
	tAudioID		musicID_;
	tAudioID		sineID_;

	mutable CKernelMPI	kernel_;	// FIXME/tp: Fix const correctness of KernelMPI
	U32			startTime_;
	tDebugLevel	debugLevel_;
};

//============================================================================
// Render functions
//============================================================================
//----------------------------------------------------------------------------
void Render()
{
  static int rotation = 0;
    
  // We are going to draw a cube centered at origin, and with an edge of 10 units
  /*
           7                      6
            +--------------------+
          / |                   /|
        /   |                 /  |
   3  /     |             2 /    |
    +---------------------+      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |4            |      |5
    |       +-------------|------+
    |     /               |     /
    |   /                 |   /
    | /                   | /
    +---------------------+
   0                      1
  
  */
  

  static GLubyte front[]  = {2,1,3,0}; //front face
  static GLubyte back[]   = {5,6,4,7}; //back face
  static GLubyte top[]    = {6,2,7,3}; //top face
  static GLubyte bottom[] = {1,5,0,4}; //bottom face    
  static GLubyte left[]   = {3,0,7,4}; //left face
  static GLubyte right[]  = {6,5,2,1}; //right face

  
  static GLshort vertices[] = {-5,-5,-5,  5,-5,-5,  5,5,-5, -5,5,-5,  
                               -5,-5,5,   5,-5,5,   5,5,5,  -5,5,5};

  // Texture map coordinates
  static GLshort texmap[] = { 0,1,  1,1,  1,0,  0,0,   
  							  0,1,  1,1,  1,0,  0,0 };
  							  
#if defined(EMULATION) || defined(LF1000)
  static GLshort quad[] = { -5,-5,-20,  5,-5,-20,  5,5,-20,  -5,5,-20 }; 
#endif

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

  glLoadIdentity();
  glPushMatrix();
  if(drawInOrtho)    
    glTranslatex(0, 0, FixedFromInt(-30));

  glRotatex(FixedFromInt(45), ONE, 0, 0);
  glRotatex(FixedFromInt(rotation++), 0, ONE,0); 
  

  //Enable the vertices array  
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_SHORT, 0, vertices);
  //3 = XYZ coordinates, GL_SHORT = data type, 0 = 0 stride bytes
 
  if (draw3dText) 
  {
    // Enable the texture map array for selected sides = 1st two
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_SHORT, 0, texmap); //2 = UV coordinates per vertex
	glEnable(GL_TEXTURE_2D);
  }
  
  /*We are going to draw the cube, face by face, with different colors for 
  each face, using indexed vertex arrays and triangle strips*/
  glColor4x(ONE,0,0,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, front);

  glColor4x(0,ONE,0,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, back);
  glDisable(GL_TEXTURE_2D);

  glColor4x(0,0,ONE,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, top);

  glColor4x(ONE,ONE,0,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, bottom);

  glColor4x(0,ONE,ONE,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, left);

  glColor4x(ONE,0,ONE,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, right);

  // Render textured quad in normalized modelview coordinates
  glPopMatrix();

#if defined(EMULATION) || defined(LF1000)
  // FIXME/dm: We really need normalized orthograophic matrix
  //		   to render flat texture tile at 1:1 scale.
  if (draw3dText) 
  {
	  glVertexPointer(3, GL_SHORT, 0, quad);
	  glTexCoordPointer(2, GL_SHORT, 0, texmap);
	  glEnable(GL_TEXTURE_2D);
	  glEnable(GL_BLEND);
	  glBlendFunc(GL_ONE, GL_ONE);
	  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	  }
#endif
		
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);		
  glDisableClientState(GL_VERTEX_ARRAY);
  glFlush();   	
  eglSwapBuffers(glesDisplay, glesSurface);
}
//----------------------------------------------------------------------------
void Perspective (GLfloat fovy, GLfloat aspect, GLfloat zNear,  GLfloat zFar)
{
  GLfixed xmin, xmax, ymin, ymax, aspectFixed, znearFixed;     
  
  aspectFixed = FixedFromFloat(aspect);
  znearFixed = FixedFromFloat(zNear);

  ymax = MultiplyFixed(znearFixed, FixedFromFloat((GLfloat)tan(fovy * 3.1415962f / 360.0f)));  
  ymin = -ymax;

  xmin = MultiplyFixed(ymin, aspectFixed);
  xmax = MultiplyFixed(ymax, aspectFixed);  
   
  glFrustumx(xmin, xmax, ymin, ymax, znearFixed, FixedFromFloat(zFar));

}
//----------------------------------------------------------------------------
void SetOrtho()
{
  /*Setup of the orthographic matrix. We use an ortho cube centered 
  at (0,0,0) with 40 units of edge*/
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();    
  glOrthox(FixedFromInt(-20), FixedFromInt(20), 
           FixedFromInt(-20), FixedFromInt(20), 
           FixedFromInt(-20) , FixedFromInt(20));
  glMatrixMode(GL_MODELVIEW);
}
//----------------------------------------------------------------------------
void SetPerspective()
{
  int width, height;
  // FIXME/tp: Use DisplayMPI
  //OS_GetWindowSize(hNativeWnd, &width, &height);
  width = 240;
  height= 320;
  float ratio = (float)width/height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();        
  Perspective(45.0f,ratio, 1.0f, 40.0f);
  glMatrixMode(GL_MODELVIEW);
}


//============================================================================
// main()
//============================================================================
int main() 
{
	// Initialize OpenGL state (FIXME: get rid of glesXXX globals)
	CKernelMPI	kernel;
	BrioOpenGLConfig	config(kBrioOpenGL11);
	glesDisplay = config.eglDisplay;
	glesSurface = config.eglSurface;
	glesContext = config.eglContext;

	// Setup button handler
	// FIXME: Order of events here is important on embedded target
	CDebugMPI	dbg(kMyApp);
	CButtonMPI	button;
	TransitionController	controller;
	
	if (button.RegisterEventListener(&controller) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return -1;
	}
	
	// Setup font via app resource path
	CFontMPI	myFont;
	CString		myFontName = CString("DidjPropBold.ttf");

	//myFont.SetFontResourcePath(GetAppRsrcFolder());
	//draw3dText = (myFont.LoadFont(myFontName, 18) != kInvalidFontHndl);
		
	if (draw3dText) 
	{
		// Set font drawing attributes 
		myFont.SetFontColor(0xFFFFFFFF);
		
		// Create buffer for font rendering which will be used as texture
		tFontSurf	fontSurf;
		fontSurf.width = 128;
		fontSurf.height = 128;
		fontSurf.pitch = fontSurf.width * 4;
		fontSurf.buffer = static_cast<U8*>(malloc(fontSurf.pitch * fontSurf.height));
		fontSurf.format = kPixelFormatARGB8888;
		memset(fontSurf.buffer, 0, fontSurf.pitch * fontSurf.height);		
				
		// Setup texture the OpenGL ES way, using fixed-point params as below  
		GLuint 		myTex;
		glGenTextures(1, &myTex);
		glBindTexture(GL_TEXTURE_2D, myTex);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glEnable(GL_TEXTURE_2D);
		
		// Render font text string and download to texture
		//myFont.DrawString(&myFontName, 0, fontSurf.height/2, &fontSurf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fontSurf.width, fontSurf.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontSurf.buffer);
	}
	/*Remember: because we are programming for a mobile device, we cant 
	use any of the OpenGL ES functions that finish in 'f', we must use 
	the fixed point version (they finish in 'x'*/
	glClearColorx(0, 0, 0, 0);
	//Do not want to see smoothed colors, only a plain color for face
	glShadeModel(GL_FLAT);	
	//Enable the depth test in order to see the cube correctly
	glEnable(GL_DEPTH_TEST);
	/*Taking care of specifying correctly the winding order of the 
	vertices (counter clock wise order), we can cull up all back faces.
	This is probably one of the better optimizations we could work,
	because we are avoiding a lot of computations that wont be reflected
	in the screen, using	glEnable(GL_CULL_FACE) to do the work*/
	glEnable(GL_CULL_FACE);

	//SetOrtho();
	SetPerspective();
	
	//Message Loop
	bool done = false;
	while(!done)
	{
		Render();

		bool transition = controller.ShouldTransition();

		if( transition )
		{
			if( drawInOrtho )
				SetOrtho();
			else
				SetPerspective();
			drawInOrtho = !drawInOrtho;
		}

		// Yield to other task threads, like audio
		kernel.TaskSleep(10); 

		done = controller.IsDone();
	}
 
	button.UnregisterEventListener(&controller);
	return 0;
}
