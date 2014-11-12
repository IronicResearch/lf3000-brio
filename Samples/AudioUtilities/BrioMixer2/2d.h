#ifndef _2D_H_
#define _2D_H_

#include <SystemErrors.h>
#include <StringTypes.h>
#include <EventMessage.h>

#include <BrioOpenGLConfig.h>
#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <SystemTypes.h>

#include <DisplayMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

#define k2D_WIDTH	320
#define k2D_HEIGHT	240
#define k2D_LEFT	0
#define k2D_TOP 	0
#define k2D_ALPHA	50

// COLORS RR:GG:BB
#define LF_COLOR_RGB_WHITE     0xFFFFFF
#define LF_COLOR_RGB_BLACK     0x000000
#define LF_COLOR_RGB_GRAY      0x777777
#define LF_COLOR_RGB_GRAYBLUE  0x878793
#define LF_COLOR_RGB_STEELBLUE 0xD3C3D0
#define LF_COLOR_RGB_WHITEBLUE 0xF0F0FF

#define LF_COLOR_RGB_BLUE    0x0000FF
#define LF_COLOR_RGB_GREEN   0x00FF00
#define LF_COLOR_RGB_RED     0xFF0000

#define LF_COLOR_RGB_YELLOW  0xFFFF00
#define LF_COLOR_RGB_AQUA    0x00FFFF
#define LF_COLOR_RGB_MAGENTA 0xFF00FF

// COLORS AA:RR:GG:BB
#define LF_COLOR_WHITE     (0xFF000000 | LF_COLOR_RGB_WHITE)
#define LF_COLOR_BLACK     (0xFF000000 | LF_COLOR_RGB_BLACK)
#define LF_COLOR_GRAY      (0xFF000000 | LF_COLOR_RGB_GRAY)
#define LF_COLOR_GRAYBLUE  (0xFF000000 | LF_COLOR_RGB_GRAYBLUE)
#define LF_COLOR_STEELBLUE (0xFF000000 | LF_COLOR_RGB_STEELBLUE)
#define LF_COLOR_WHITEBLUE (0xFF000000 | LF_COLOR_RGB_WHITEBLUE)

#define LF_COLOR_BLUE   0xFF0000FF
#define LF_COLOR_GREEN  0xFF00FF00
#define LF_COLOR_RED    0xFFFF0000

#define LF_COLOR_YELLOW  0xFFFFFF00
#define LF_COLOR_AQUA    0xFF00FFFF
#define LF_COLOR_MAGENTA 0xFFFF00FF

#define k2D_GradientDirection_LeftToRight 0
#define k2D_GradientDirection_RightToLeft 1
#define k2D_GradientDirection_TopToBottom 2
#define k2D_GradientDirection_BottomToTop 3

    tDisplayHandle
SetUp2D(CDisplayMPI *displayMPI, int left, int top, int width, int height);

//void Set2D_String(CDisplayMPI *displayMPI, char *s);
//void Set2D_String_Index(CDisplayMPI *displayMPI, char *s, long lineIndex);

LF_END_BRIO_NAMESPACE()

#endif // _2D_H_



