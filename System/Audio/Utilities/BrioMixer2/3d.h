#ifndef _3D_H_
#define _3D_H_


//OpenGL ES Includes
#include <GLES/gl.h>

/*EGL is the "machine" that glues OpenGL ES with the underlying
windowing system. We need it to create a suitable context and a drawable window*/
#include <GLES/egl.h>

/*Because we are building software device dependent (the PDA), we have care about 
its limitations. PDA's doesn't have a FPU unit, so all floating point operations 
are emulated in the CPU. To have real data type, PDA's uses reals with a fixed point
format. For a fixed point number we only need an integer, with the same size (in bytes)
that a float, that is, a normal int number. The first 16 bits of the int will be the 
"integer" part, and the last 16 bits will be the "real" part. This will cause a lack 
of precision, but it is better than emulate all FPU in the CPU. To convert an integer 
number to a fixed point number we need to displace its bits to the left, as the FixedFromInt 
function does. In this chapter we only will need the conversion int->fixed point.
Other conversions will be showed when needed, in later chapters. A complete description of 
the fixed point maths is beyond the purpose of this set of tutorials, but the topic will
be widely covered through the chapters. 
OpenGL ES offers us a set of functions that works with fixed point (Glfixed). These 
functions are available through the OpenGL ES OES_fixed_point extension. 
A little word about the OpenGL ES extensions: They are divided into two categories: 
those that are fully integrated into the profile definition (core additions); and those
that remain extensions (profile extensions). Core additions do not use extension suffixes
and does not requires initialization, whereas profile extensions retain their extension suffixes.
OES_fixed_point is a core addition. The other extensions are listed and explained in the 
OpenGL ES 1.1 specification.*/

#include <BrioOpenGLConfig.h>
#include <SystemTypes.h>

/*changed defines to constd because we do not want these operations occur in run time*/

const unsigned int PRECISION = 16;
const GLfixed ONE  = 1 << PRECISION;
const GLfixed ZERO = 0;

inline GLfixed FixedFromInt(int value) {return value << PRECISION;}
inline GLfixed FixedFromFloat(float value) {return static_cast<GLfixed>(value * static_cast<float>(ONE));}
inline GLfixed MultiplyFixed(GLfixed op1, GLfixed op2) {return static_cast<GLfixed>(((S64)op1 * (S64)op2) >> PRECISION);}

GLboolean InitOGLES();// Our GL initialization function
void SetOrtho();
void SetPerspective();

//Our own gluPerspective-like function but modified to work with GLfixed
void Perspective (GLfloat fovy, GLfloat aspect, GLfloat zNear,  GLfloat zFar); 
void Clean();   //Our clean function. It will clean all used resources

// Global OpenGL variables
extern GLboolean drawInOrtho; //This variable will change with every click in the touch screen of the pda
extern EGLDisplay glesDisplay;  // EGL display
extern EGLSurface glesSurface;	 // EGL rendering surface
extern EGLContext glesContext;	 // EGL rendering context

void SetUp3D();
void Render3D();  // Our Render function

#endif // _3D_H_
