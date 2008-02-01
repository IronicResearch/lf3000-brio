#ifndef _FONT_H_
#define _FONT_H_

//OpenGL ES Includes
#include <GLES/gl.h>

/*EGL is the "machine" that glues OpenGL ES with the underlying
windowing system. We need it to create a suitable context and a drawable window*/
#include <GLES/egl.h>
#include <BrioOpenGLConfig.h>
#include <SystemTypes.h>

#include <FontMPI.h>

#define FONT_POINT_SIZE   12

extern long draw3dText;
extern char *fontDisplayString;

void SetFont_DisplayString(CFontMPI *pFont, char *s);

void SetUpFont(CFontMPI *pFont);

#endif // _FONT_H_
