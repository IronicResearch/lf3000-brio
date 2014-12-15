#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <SystemTypes.h>
#include <StringTypes.h>

#include <DebugMPI.h>
#include <FontMPI.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "linebrowser.h"


LF_BEGIN_BRIO_NAMESPACE()

char *gGlyphList = " -+0123456789.|()RMSPeakxdB_<>,%";

//CPath GetAppRsrcFolder( void );
// ============================================================================
// GetAppRsrcFolder
// ============================================================================
CPath GetAppRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir;
#else	
	return "/Didj/Data/BrioMixer/rsrc/";
#endif	// EMULATION
} // ---- end GetAppRsrcFolder() ----

// ==============================================================================
// CLineBrowser implementation
// ==============================================================================
CLineBrowser::CLineBrowser(CDisplayMPI *displayMPI, tDisplayHandle handle,
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
//printf("frameBuffer Height=%d Width=%d\n", _frameBufferHeight, _frameBufferWidth);

_pFrameBuffer  = _pDisplayMPI->GetBuffer(_handle);
_pitch         = _pDisplayMPI->GetPitch (_handle);
_bitsPerPixel  = _pDisplayMPI->GetDepth (_handle);
_bytesPerPixel = _bitsPerPixel / 8;

// Render font text strings into draw surface buffer
_fontSurf.width  = _width;
_fontSurf.height = _height;
_fontSurf.pitch  = _pitch;
_fontSurf.buffer = _pFrameBuffer;
_fontSurf.format = kPixelFormatARGB8888;

_backgroundBufferBytes = _width * _height * _bytesPerPixel;
_pBackgroundBuffer32   = (U32 *) malloc(_backgroundBufferBytes);
if (!_pBackgroundBuffer32)
    printf("CLineBrowser: Unabled to allocate backgroundBuffer of %ld bytes\n", _backgroundBufferBytes);

// LeftToRight, RightToLeft , TopToBottom , BottomToTop 
_gradientDirection = k2D_GradientDirection_TopToBottom;
_gradientEnable    = false;

_backgroundColor          = LF_COLOR_BLACK; // COLORS AA:RR:GG:BB
_backgroundColorIntensity = 0xff; // [0..255]
_needToRegenerateBackground = true;

Clear();
GenerateBackground();
RedrawBackground();

// Verdana.ttf impact.ttf cour.ttf courbd.ttf
// WEDGIE__.TTF COPRGTB.TTF COPRGTL.TTF
char *fName = "Verdana.ttf";

_pFontMPI = new CFontMPI();
_pFontMPI->SetFontResourcePath(GetAppRsrcFolder());
//printf("CLineBrowser(): GetAppRsrcFolder()='%s'\n", _pFontMPI->GetFontResourcePath()->c_str());
//dbgMPI_.GetDebugLevel();
//_pFontMPI->SetDebugLevel(kDbgLvlCritical); // kDbgLvlCritical, kDbgLvlVerbose

SetTitleFont(fName, FONT_POINT_SIZE, LF_COLOR_WHITE); //BLUE);
SetTextFont (fName, FONT_POINT_SIZE, LF_COLOR_WHITE);

//printf("CLineBrowser(): GetFontResourcePath='%s'\n", myFontPath->c_str());

// Setup font via app resource path
//CPath		*myFontPath = pFont->GetFontResourcePath();
//printf("InitBrowser(): GetFontResourcePath='%s'\n", myFontPath->c_str());

Update();
}   // ---- end CLineBrowser() ----

// ==============================================================================
// ~CLineBrowser :
// ==============================================================================
CLineBrowser::~CLineBrowser()
{
long i = 0;
}   // ---- end ~CLineBrowser() ----

// ============================================================================
// RedrawBackground:  Redraws background but not browser lines
// ============================================================================
    void 
CLineBrowser::RedrawBackground()
{
//printf("RedrawBackground: %ld bytes\n", backgroundBufferBytes);
bcopy(_pBackgroundBuffer32, _pFrameBuffer, _backgroundBufferBytes);
_pDisplayMPI->Invalidate(0, _pDirtyRect);
}   // ---- end RedrawBackground() ----

// ============================================================================
// ClearLineArea:  Clear single line area in frame buffer
// ============================================================================
    void 
CLineBrowser::ClearLineArea(int index, long update)
{
long i;
S32 x = 0;
S32 y = 0;
S32 lineWidth  = _width;
S32 lineHeight = _textCellHeight;
//printf("ClearLineArea: lineWidth=%ld lineHeight=%ld\n", _lineWidth, _lineHeight);

if (0 == index)
    {
    y = 0;
    lineHeight = _titleCellHeight;
    }
else
    {
    y = _titleCellHeight + (index-1)*_textCellHeight;
    lineHeight = _textCellHeight;
    }

long offset    = y*_frameBufferWidth + x;
long lineBytes = 4*(_frameBufferWidth*lineHeight);
bcopy(&_pBackgroundBuffer32[offset], &_pFrameBuffer[4*offset], lineBytes);
//printf("ClearLineArea: lineBytes=%ld backgroundBufferBytes=%ld\n", lineBytes, _backgroundBufferBytes);

//#define kTestBufSize (_frameBufferWidth*20*4)
//unsigned char testBuf[kTestBufSize];
//for (long i = 0; i < kTestBufSize; i++)
//    testBuf[i] = 0xff;
//bcopy(testBuf, framePtr, lineBytes);

if (update)
    _pDisplayMPI->Invalidate(0, _pDirtyRect);
}   // ---- end ClearLineArea() ----

// ============================================================================
// CLineBrowser::GenerateBackground:  Generate background buffer
// ============================================================================
    void 
CLineBrowser::GenerateBackground()
{
int i, j;
unsigned char xC[4], xK[4];
int mod = 0xFF;

// Fill with graduated pixel pattern
// GK FIXXX: this really only works with the 32-bit color bits
//xC[0] = (g2D_Background_Color & 0xFF000000)>>24; // Blue
//xC[1] = (g2D_Background_Color & 0x00FF0000)>>16; // Green
//xC[2] = (g2D_Background_Color & 0x0000FF00)>> 8; // Red
//xC[3] = (g2D_Background_Color & 0x000000FF);     // Alpha
xC[0] = (_backgroundColor & 0x000000FF);     // Alpha
xC[1] = (_backgroundColor & 0x0000FF00)>> 8; // Red
xC[2] = (_backgroundColor & 0x00FF0000)>>16; // Green
xC[3] = (_backgroundColor & 0xFF000000)>>24; // Blue

// Scale color components by intensity value
unsigned long k32, color32 = _backgroundColor;
for (i = 1; i < 4; i++)
   xC[i] = (xC[i]*_backgroundColorIntensity)>>8;
color32 = (xC[3]<<24) | (xC[2]<<16) | (xC[1]<<8) | (xC[0]);

//U32 *buf32 = (U32 *) _frameBuffer;
U32 *buf32 = _pBackgroundBuffer32;
//printf("buf32=%p bytesPerPixel=%d\n", (void*) buf32, _bytesPerPixel);

// Fill buffer with constant color
if (!_gradientEnable)    
    {
    for (i = 0; i < _height*_width; i++)
        *buf32++ = color32;
    }
// Fill buffer with graduated color
else
    {
    if (k2D_GradientDirection_TopToBottom == _gradientDirection)
        {
         for (i = 0; i < _height; i++)
            {
            mod = (_height-1)-i%_height; // HEIGHT-1 .. 0
            for (int k = 0; k < 4; k++)
               xK[k] = (xC[k]*mod)>>8;
            k32 = (xK[3]<<24) | (xK[2]<<16) | (xK[1]<<8) | (xK[0]);
           	for (int j = 0; j < _width; j++) 
                *buf32++ = k32;
            }
        }
    else if (k2D_GradientDirection_BottomToTop == _gradientDirection)
        {
         for (i = 0; i < _height; i++)
            {
            mod = i%_height;            // 0..HEIGHT-1
            for (int k = 0; k < 4; k++)
               xK[k] = (xC[k]*mod)>>8;
            k32 = (xK[3]<<24) | (xK[2]<<16) | (xK[1]<<8) | (xK[0]);
           	for (int j = 0; j < _width; j++) 
                *buf32++ = k32;
            }
        }
    else if (k2D_GradientDirection_LeftToRight == _gradientDirection)
        {
        for (i = _height-1; i >= 0; i--)
            {
            mod = i%_height;            // 0..HEIGHT-1
            for (int k = 0; k < 4; k++)
               xK[k] = (xC[k]*mod)>>8;
            k32 = (xK[3]<<24) | (xK[2]<<16) | (xK[1]<<8) | (xK[0]);
           	for (int j = 0; j < _width; j++) 
                *buf32++ = k32;
            }
        }
    } // end 
_needToRegenerateBackground = false;
}   // ---- end GenerateBackground() ----

// ============================================================================
// Clear:  Clears content of browser lines but doesn't redraw
// ============================================================================
    void 
CLineBrowser::Clear()
{
for (int i = 0; i < kLineBrowser_MaxLines; i++)
    {
    _lineUseGlyphLibrary[i] = false;
    _lineDirty[i] = kLineBrowser_LineDirty;
    _strings[i][0] = '\0';
    }
_lineCount = 0;
}   // ---- end Clear() ----

// ============================================================================
// GetLineCount:  
// ============================================================================
    long 
CLineBrowser::GetLineCount()
{
// Count non-empty lines
_lineCount = 0;
for (long i = 0; i < kLineBrowser_MaxLines; i++)
    _lineCount += ('\0' != _strings[i][0]);
return (_lineCount);
}   // ---- end GetLineCount() ----

// ============================================================================
// SetBackgroundColor:  
// ============================================================================
    void 
CLineBrowser::SetBackgroundColor(unsigned long colorBytes_AARRGGBB)
{
_backgroundColor = colorBytes_AARRGGBB;
_needToRegenerateBackground = true;
}   // ---- end SetBackgroundColor() ----

// ============================================================================
// SetBackgroundColor_Intensity:  
// ============================================================================
    void 
CLineBrowser::SetBackgroundColor_Intensity(unsigned char x)
{
_backgroundColorIntensity = x;
_needToRegenerateBackground = true;
}   // ---- end SetBackgroundColor_Intensity() ----

// ============================================================================
// SetTextFont:  
// ============================================================================
    void 
CLineBrowser::SetTextFont(char *name, int size, unsigned long colorBytes_AARRGGBB)
{
_textColor    = colorBytes_AARRGGBB;
_textSize     = size;
_textFontName = name;
//printf("TextFont: color=$%8X size=%d name='%s'\n", (unsigned int)colorBytes_AARRGGBB, size, name);

CString		myFontName = CString(name);
_textFontH = _pFontMPI->LoadFont(myFontName, size);
if (kInvalidFontHndl == _textFontH)
    printf("TextFont: unable to load font '%s' size=%d\n", name, size);

tFontMetrics *metrics = _pFontMPI->GetFontMetrics();
//printf("TextFont: name='%s' height=%d width=%d\n", name, metrics->height, metrics->advance);
_textCellHeight = metrics->height;
_textCellWidth  = metrics->advance;

_pFontMPI->SetFontColor(_textColor); // YELLOW, GREY, BLUE, RED: AA:RR:GG:BB
}   // ---- end SetTextFont() ----

// ============================================================================
// SetTitleFont:  
// ============================================================================
    void 
CLineBrowser::SetTitleFont(char *name, int size, unsigned long colorBytes_AARRGGBB)
{
_titleColor    = colorBytes_AARRGGBB;
_titleSize     = size;
_titleFontName = name;
//printf("SetTitleFont: color=$%8X size=%d name='%s'\n", (unsigned int)colorBytes_AARRGGBB, size, name);

// Setup font via app resource path
//pFont->SetFontResourcePath(GetAppRsrcFolder());
//CPath		*myFontPath = pFont->GetFontResourcePath();
//printf("SetUpFont(): GetFontResourcePath='%s'\n", myFontPath->c_str());

CString		myFontName = CString(name);
_titleFontH = _pFontMPI->LoadFont(myFontName, size);
if (kInvalidFontHndl == _titleFontH)
    printf("SetTitleFont: unable to load font '%s' size=%d\n", name, size);

_pFontMPI->SetFontColor(_titleColor); // YELLOW, GREY, BLUE, RED: AA:RR:GG:BB

tFontMetrics *metrics = _pFontMPI->GetFontMetrics();
//printf("SetTitleFont: name='%s' height=%d width=%d\n", name, metrics->height, metrics->advance);
_titleCellHeight = metrics->height;
_titleCellWidth  = metrics->advance;
//struct tFontMetrics {
//	S16		ascent;			// max vertical above baseline 
//	S16		descent;		// max vertical below baseline (negative)
//	S16		height;			// max per-line vertical advance
//	S16		advance;		// max per-glyph horizontal advance
//};

}   // ---- end :SetTitleFont() ----

// ============================================================================
// SetGradientDirection:  
// ============================================================================
    void 
CLineBrowser::SetGradientDirection(char x)
{
_gradientDirection = x;
}   // ---- end SetGradientDirection() ----

// ============================================================================
// SetEnableGradient:  
// ============================================================================
    void 
CLineBrowser::SetEnableGradient(char enable)
{
_gradientEnable = enable;
_needToRegenerateBackground = true;
}   // ---- end SetEnableGradient() ----

// ============================================================================
// ClearLine
// ============================================================================
    void 
CLineBrowser::ClearLine(long index, long update)
{
//if (index >= kLineBrowser_MaxLines)
//    return;

_strings[index][0] = ' ';
_strings[index][1] = '\0';
_lineDirty[index] = kLineBrowser_LineDirty;

if (update)
    Update();
}   // ---- end ClearLine() ----

// ============================================================================
// SetGlyphLine
// ============================================================================
    void 
CLineBrowser::SetGlyphLine(long index, long update)
{
_lineUseGlyphLibrary[index] = true;

if (update)
    Update();
}   // ---- end SetGlyphLine() ----

// ============================================================================
// CLineBrowser::DeleteLine
// ============================================================================
    void 
CLineBrowser::DeleteLine(long index, long update)
{
if (index >= kLineBrowser_MaxLines)
    return;

_lineDirty[index] = kLineBrowser_LineDirty;

_strings[index][0] = '\0';
if (update)
    Update();
}   // ---- end DeleteLine() ----

// ============================================================================
// SetLine
// ============================================================================
    void 
CLineBrowser::SetLine(long index, char *s, long update)
{
if (index >= kLineBrowser_MaxLines)
    return;

strcpy(_strings[index], s);
_lineDirty[index] = kLineBrowser_LineDirty;

if (update)
    Update();
}   // ---- end SetLine() ----

// ============================================================================
// AppendLine
// ============================================================================
    void 
CLineBrowser::AppendLine(char *s, long update)
{
long i;

// Find first empty line
for (i = 0; i < kLineBrowser_MaxLines; i++)
    {
    if ('\0' == _strings[i][0])
        break;
    }
// Set line
if (i >= kLineBrowser_MaxLines)
    return;

strcpy(_strings[i], s);
_lineDirty[i] = kLineBrowser_LineDirty;

if (update)
    Update();
}   // ---- end AppendLine() ----

// ============================================================================
// GenerateGlyphLibrary:  call this after setting the Text font attributes
// ============================================================================
    void 
CLineBrowser::GenerateGlyphLibrary()
{
//printf("CLineBrowser::GenerateGlyphLibrary: '%s'\n", gGlyphList);

_glyphBufferHeight = _textCellHeight;
_glyphBufferWidth  = _width;
_pGlyphBuffer = (U32 *) malloc(_glyphBufferHeight * _glyphBufferWidth * sizeof(U32));
if (!_pGlyphBuffer)
    printf("CLineBrowser::GenerateGlyphLibrary: Unable to allocate glyph buffer\n");
// Clear glyph buffer.  Though, this doesn't seem to solve the errant colored pixels problem.
else
    memset(_pGlyphBuffer, _glyphBufferHeight*_glyphBufferWidth * sizeof(U32), 0);
//printf("GenerateGlyphLibrary: glyphBuffer h=%d w=%d\n", _glyphBufferHeight, _glyphBufferWidth);

tFontSurf localFontSurf;
localFontSurf.width  = _glyphBufferWidth;
localFontSurf.height = _glyphBufferHeight;
localFontSurf.pitch  = _pitch;
localFontSurf.buffer = (U8 *) _pGlyphBuffer;
localFontSurf.format = kPixelFormatARGB8888;

_pFontMPI->SelectFont(  _textFontH);
_pFontMPI->SetFontColor(_textColor); 
CString cs = CString(gGlyphList);
S32 x = 0, y = 0;
_pFontMPI->DrawString(cs, x, y, localFontSurf, false);

// Weird color bits in rendering buffer.  So, erase them where possible
// Easy to erase first two lines and then some above the first few characters
for (long i = 0; i < _frameBufferWidth*2 + 3*_textCellWidth; i++)
    _pGlyphBuffer[i] = 0;

}   // ---- end GenerateGlyphLibrary() ----

// ============================================================================
// DrawGlyphString:  call this after setting the Text font attributes
// ============================================================================
    void 
CLineBrowser::DrawGlyphString(char *s, S32 x, S32 y)
{
// Not sure if we want to copy entire glyph or only non-black pixels
#ifdef EMULATION
int alpha = true;  
#else
int alpha = false;  
#endif
//printf("DrawGlyphString: (%ld, %ld) '%s'\n", x, y, s);

for (long i = 0; s[i] != '\0'; i++)
    {
    U32 *sPtr = GetGlyphBase(s[i]);
    U32 *dPtr = (U32 *) &_pFrameBuffer[(y*_frameBufferWidth + x)*sizeof(U32)];
    for (long j = 0; j < _textCellHeight; j++)
        {
        if (alpha)
            {
            for (long k = 0; k < _textCellWidth; k++)
                {
                if (sPtr[k] != 0)
                    dPtr[k] = sPtr[k];
                }
            }
        else
            {
            for (long k = 0; k < _textCellWidth; k++)
                 dPtr[k] = sPtr[k];
            }
        dPtr += _frameBufferWidth;
        sPtr += _glyphBufferWidth;
        }
    x += _textCellWidth;
    }
}   // ---- end DrawGlyphString() ----

// ============================================================================
// GetGlyphBase:  Get base address of glyph for specified character
// ============================================================================
    U32 * 
CLineBrowser::GetGlyphBase(char c)
{
long i;
char *s = gGlyphList;

// Find index of character
for (i = 0; s[i] != '\0'; i++)
    {
    if (c == s[i])
        break;
    }
if ('\0' == s[i])
    {
    printf("GetGlyphBase: unable to find character '%c'\n", c);
    return (_pGlyphBuffer);
    }
//printf("GetGlyphBase: looking for '%c' and found '%c'\n", c, s[i]);
char *gGlyphList = " -+0123456789.|()RMSPeakxdB_<>,";
return (&_pGlyphBuffer[i*_textCellWidth]);
}   // ---- end GetGlyphBase() ----

// ============================================================================
// Update
// ============================================================================
    void 
CLineBrowser::Update()
{
//{static long c=0; printf("CLineBrowser::Update %ld START\n", c++);}

//_pDisplayMPI->LockBuffer(_handle);

//_needToRegenerateBackground = true;
if (_needToRegenerateBackground)
    {
//{static long c=0; printf("CLineBrowser::Update%ld needToRegenerateBackground\n", c++);}
    GenerateBackground();
    RedrawBackground();
    for (long i = 0; i < kLineBrowser_MaxLines; i++)
         _lineDirty[i] = kLineBrowser_LineDirty;
    }

S32 y = 0;
long lineIndex = 0;
for (long i = 0; i < kLineBrowser_MaxLines; i++)
    {
//  WEIRD:  keep here as it's modified by DrawString()
    S32 x = 0;  

     // Render font text string
    if ('\0' != _strings[i][0])
        {
        if (kLineBrowser_LineDirty == _lineDirty[i])
            {
            CString cs = CString(&_strings[i][0]);
//printf("CLineBrowser::Update: Line%2ld (%3ld,%3ld) '%s'\n", lineIndex, x, y, _strings[i]);
            if (0 == lineIndex)
                {
                _pFontMPI->SelectFont(  _titleFontH);
                _pFontMPI->SetFontColor(_titleColor); 
                }
            else
                {
                _pFontMPI->SelectFont(  _textFontH);
                _pFontMPI->SetFontColor(_textColor); 
                }
            ClearLineArea(lineIndex, false);
            if (_lineUseGlyphLibrary[i])
                {
//printf("CLineBrowser::Update: drawing glyph line%ld '%s\n", i, _strings[i]);
                DrawGlyphString(_strings[i], x, y);
                }
            else
                _pFontMPI->DrawString(cs, x, y, _fontSurf, true);
            _lineDirty[i] = kLineBrowser_LineUpdated;
            }
        if (0 == lineIndex)
            y += _titleCellHeight;
        else
            y += _textCellHeight;
        lineIndex++;
        }
    }
_lineCount = lineIndex;

//_pDisplayMPI->UnlockBuffer(_handle);

// Update display
_pDisplayMPI->Invalidate(0, _pDirtyRect);
}   // ---- end Update() ----

LF_END_BRIO_NAMESPACE()


