#ifndef _BITSTRIP_H_
#define _BITSTRIP_H_

#include <SystemErrors.h>
#include <StringTypes.h>
#include <EventMessage.h>

#include <BrioOpenGLConfig.h>
#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <SystemTypes.h>

#include <DisplayMPI.h>
#include <DebugMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

#include "2d.h"

#define kBitStrip_Width	    320
#define kBitStrip_Height	10
#define kBitStrip_Left	    0
#define kBitStrip_Top    	0

#define kBitStrip_CellHeight 12

#define kBitStrip_Dirty   1
#define kBitStrip_Updated 0

#define kBitStrip_BitmapFormat_ARGB_8888 0 // 4 bytes: Alpha, Red, Green, Blue
#define kBitStrip_BitmapFormat_RGB_888   1 // 3 bytes:        Red, Green, Blue

//==============================================================================
// Class: BitStrip
//==============================================================================
class CBitStrip {
public:
    CBitStrip      (CDisplayMPI *displayMPI, tDisplayHandle handle,
                        int left, int top, int width, int height);
	~CBitStrip();

    void Redraw();
    void Update();
     
    void SetColor          (unsigned long colorBytes_AARRGGBB);
    void SetColor_Intensity(unsigned char x);

    void Clear(long update);
    void SetBitmap(void *p, long width, long height, long format, long update);

private:
	CDisplayMPI   *_pDisplayMPI;
    tDisplayHandle _handle;

    int            _top, _left, _width, _height;
    tRect           _dirtyRect;
    tRect           *_pDirtyRect;

    int            _frameBufferHeight, _frameBufferWidth;

    unsigned long _color;
    unsigned char _colorIntensity;

    char _dirty;

    U8  *_pFrameBuffer;     

//#define kBitStrip_MaxBitmaps    20
    U8  *_pBitmapBuffer24; //[kBitStrip_MaxBitmaps];     
    U32 *_pBitmapBuffer32; //[kBitStrip_MaxBitmaps];     
    int  _bitmapPixels, _bitmapFormat;     
    int  _bitmapHeight, _bitmapWidth;     

    int _bitsPerPixel;
    int _bytesPerPixel;
};
LF_END_BRIO_NAMESPACE()

#endif		// _BITSTRIP_H_


