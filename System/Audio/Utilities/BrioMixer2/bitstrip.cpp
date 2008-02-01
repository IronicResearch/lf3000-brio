#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <SystemTypes.h>
#include <StringTypes.h>

#include <math.h>

#include "bitstrip.h"

LF_BEGIN_BRIO_NAMESPACE()

CPath GetAppRsrcFolder( void );

// ==============================================================================
// CBitStrip implementation
// ==============================================================================
CBitStrip::CBitStrip(CDisplayMPI *displayMPI, tDisplayHandle handle,
                        int left, int top, int width, int height)
{
_pDisplayMPI = displayMPI;
_handle      = handle;

// Buffer configuration
_left   = left;   
_top    = top;    
_width  = width;  
_height = height; 

_dirtyRect.left   = left;
_dirtyRect.right  = left + width;
_dirtyRect.top    = top;
_dirtyRect.bottom = top + height;
_pDirtyRect = &_dirtyRect;

_frameBufferHeight = _pDisplayMPI->GetHeight(_handle);
_frameBufferWidth  = _pDisplayMPI->GetWidth (_handle);
_pFrameBuffer      = _pDisplayMPI->GetBuffer(_handle);
_bitsPerPixel      = _pDisplayMPI->GetDepth (_handle);
_bytesPerPixel     = _bitsPerPixel / 8;

_bitmapPixels    = 0;
_pBitmapBuffer24 = NULL;
_pBitmapBuffer32 = NULL;
_bitmapFormat    = kBitStrip_BitmapFormat_ARGB_8888;

//_bitmapBufferBytes = _width * _height * _bytesPerPixel;
//_pBitmapBuffer32   = (U32 *) malloc(_bitmapBufferBytes);
//if (!_pBitmapBuffer32)
//    printf("CBitStrip: Unabled to allocate backgroundBuffer of %ld bytes\n", _bitmapBufferBytes);

_color          = LF_COLOR_BLACK; // COLORS AA:RR:GG:BB
_colorIntensity = 0xff; // [0..255]

Clear(false);
//Redraw();
//Update();
}   // ---- end CBitStrip() ----

// ==============================================================================
// ~CBitStrip :
// ==============================================================================
CBitStrip::~CBitStrip()
{
//if (_pBitmapBuffer32)
//    {
//    free(_pBitmapBuffer32);
//    _pBitmapBuffer32 = NULL;
//    }
}   // ---- end ~CBitStrip() ----

// ============================================================================
// Redraw:  Redraws background but not browser lines
// ============================================================================
    void 
CBitStrip::Redraw()
{
printf("Redraw: bitmapPixels= %d bytes\n", _bitmapPixels);
//bcopy(_pBitmapBuffer32, _pFrameBuffer, _bitmapPixels);
_pDisplayMPI->Invalidate(0, _pDirtyRect); 
}   // ---- end Redraw() ----

// ============================================================================
// Clear:  Clear area in frame buffer
// ============================================================================
    void 
CBitStrip::Clear(long update)
{
int i;
//U32 *buf32 = (U32 *) &_pFrameBuffer[_left + _top*_frameBufferWidth];
U32 *buf32 = (U32 *) _pFrameBuffer;
buf32 += (_left + _top*_frameBufferWidth);

//printf("CBitStrip::Clear: buf32=%p bytesPerPixel=%d\n", (void*) buf32, _bytesPerPixel);
printf("CBitStrip::Clear:  left=%d top=%d width=%d height=%d \n", _left, _top, _width, _height);
//printf("CBitStrip::Clear: frameBuffer width=%d height=%d \n", _frameBufferWidth, _frameBufferHeight);

// Fill buffer with constant color
if (!_pBitmapBuffer32 && !_pBitmapBuffer24)
    {
printf("CBitStrip::Clear: filling with color %08X\n", (unsigned int)_color);
// GK FIXXX: this really only works with the 32-bit color bits
    unsigned char xC[4], xK[4];
    xC[0] = (_color & 0x000000FF);     // Alpha
    xC[1] = (_color & 0x0000FF00)>> 8; // Red
    xC[2] = (_color & 0x00FF0000)>>16; // Green
    xC[3] = (_color & 0xFF000000)>>24; // Blue

// Scale color components by intensity value
    unsigned long color32 = _color;
    for (i = 1; i < 4; i++)
       xC[i] = (xC[i]*_colorIntensity)>>8;
    color32 = (xC[3]<<24) | (xC[2]<<16) | (xC[1]<<8) | (xC[0]);

     for (i = 0; i < _height; i++)
        {
        for (long j = 0; j < _width; j++)
            buf32[j] = color32;
        buf32 += _frameBufferWidth;
        }
    }
// Fill with bitmap
else if (kBitStrip_BitmapFormat_ARGB_8888 == _bitmapFormat)
    {
printf("CBitStrip::Clear: filling with 32-bit ARGB_8888 bitmap h=%d w=%d\n", _bitmapHeight, _bitmapWidth);
    U32 *p32 = _pBitmapBuffer32;
    for (i = 0; i < _bitmapHeight; i++)
        {
        for (long j = 0; j < _bitmapWidth; j++)
            buf32[j] = *p32++;
        buf32 += _frameBufferWidth;
        }
    }
else if (kBitStrip_BitmapFormat_RGB_888 == _bitmapFormat)
    {
printf("CBitStrip::Clear: filling with 24-bit RGB_888 bitmap h=%d w=%d\n", _bitmapHeight, _bitmapWidth);
    U8  *p8 = (U8 *) _pBitmapBuffer24;
    for (i = 0; i < _bitmapHeight; i++)
        {
        for (long j = 0; j < _bitmapWidth; j++)
            {
            U8  *b8 = (U8 *) &buf32[j];
//          b8[0] = 0xFF; b8[1] = *p8++; b8[2] = *p8++; b8[3] = *p8++;
            b8[3] = 0xFF; b8[2] = *p8++; b8[1] = *p8++; b8[0] = *p8++;
//            b8[3] = 0xFF;
//            b8[2] = 0xFF;
//            b8[1] = 0xFF;
//            b8[0] = 0xFF;
            }
        buf32 += _frameBufferWidth;
        }
    }

//#define kTestBufSize (320*20*4)
//unsigned char testBuf[kTestBufSize];
//for (long i = 0; i < kTestBufSize; i++)
//    testBuf[i] = 0xff;
//bcopy(testBuf, framePtr, lineBytes);

if (update)
    _pDisplayMPI->Invalidate(0, _pDirtyRect);
}   // ---- end Clear() ----

// ============================================================================
// SetColor:  32-bit color as 4 bytes:  Alpha, Red, Green, Blue
// ============================================================================
    void 
CBitStrip::SetColor(unsigned long colorBytes_AARRGGBB)
{
_color = colorBytes_AARRGGBB;
_dirty = kBitStrip_Dirty;
//printf("CBitStrip::SetColor: %08X\n", (unsigned int)_color);
}   // ---- end SetColor() ----

// ============================================================================
// SetColor_Intensity:  
// ============================================================================
    void 
CBitStrip::SetColor_Intensity(unsigned char x)
{
_colorIntensity = x;
_dirty = kBitStrip_Dirty;
}   // ---- end SetColor_Intensity() ----

// ============================================================================
// SetBitmap
// ============================================================================
    void 
CBitStrip::SetBitmap(void *p, long width, long height, long format, long update)
{
printf("CBitStrip::SetBitmap: w=%ld h=%ld format=%ld\n", width, height, format);
_dirty = kBitStrip_Dirty;

_bitmapFormat = format;
if      (kBitStrip_BitmapFormat_ARGB_8888 == format)
    _pBitmapBuffer32 = (U32 *) p;
else if (kBitStrip_BitmapFormat_RGB_888   == format)
    _pBitmapBuffer24 = (U8 *) p;

_bitmapHeight = height; 
_bitmapWidth  = width;     
_bitmapPixels = height*width;     

if (update)
    Update();
}   // ---- end SetBitmap() ----

// ============================================================================
// Update
// ============================================================================
    void 
CBitStrip::Update()
{
//{static long c=0; printf("CBitStrip::Update : %ld START\n", c++);}

if (kBitStrip_Dirty == _dirty)
    {
     Clear(false);
    _dirty = kBitStrip_Updated;
    }

// Update display
_pDisplayMPI->Invalidate(0, _pDirtyRect); 
}   // ---- end Update() ----

LF_END_BRIO_NAMESPACE()


