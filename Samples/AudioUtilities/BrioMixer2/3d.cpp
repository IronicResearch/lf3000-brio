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

LF_USING_BRIO_NAMESPACE()
#include "3d.h"
#include "font.h"
#include "cube.h"
	
GLboolean drawInOrtho = GL_TRUE; //This variable will change with every click in the touch screen of the pda
EGLDisplay glesDisplay;  // EGL display
EGLSurface glesSurface;	 // EGL rendering surface
EGLContext glesContext;	 // EGL rendering context

// ============================================================================
// SetUp3D
// ============================================================================
void SetUp3D()
{
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
}   // ---- end SetUp3D() ----

//============================================================================
// Render3D functions
//============================================================================
void Render3D()
{
RenderCube();
}   // ---- end Render3D() ----

// ============================================================================
// Perspective
// ============================================================================
void Perspective (GLfloat fovy, GLfloat aspect, GLfloat zNear,  GLfloat zFar)
{
  GLfixed xmin, xmax, ymin, ymax, aspectFixed, znearFixed;     
  
  aspectFixed = FixedFromFloat(aspect);
  znearFixed  = FixedFromFloat(zNear);

  ymax = MultiplyFixed(znearFixed, FixedFromFloat((GLfloat)tan(fovy * 3.1415962f / 360.0f)));  
  ymin = -ymax;

  xmin = MultiplyFixed(ymin, aspectFixed);
  xmax = MultiplyFixed(ymax, aspectFixed);  
   
  glFrustumx(xmin, xmax, ymin, ymax, znearFixed, FixedFromFloat(zFar));

}   // ---- end Perspective() ----

// ============================================================================
// SetOrtho
// ============================================================================
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
}   // ---- end SetOrtho() ----

// ============================================================================
// SetPerspective
// ============================================================================
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
}   // ---- end SetPerspective() ----



