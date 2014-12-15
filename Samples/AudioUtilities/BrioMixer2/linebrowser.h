#ifndef _LINEBROWSER_H_
#define _LINEBROWSER_H_

#include <SystemErrors.h>
#include <StringTypes.h>
#include <EventMessage.h>

#include <BrioOpenGLConfig.h>
#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <SystemTypes.h>

#include <DisplayMPI.h>
#include <FontMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

#include "2d.h"
#include "font.h"

#define kLineBrowser_CellHeight 12
#define kLineBrowser_MaxLines (240/kLineBrowser_CellHeight)  // GK FIXX:  Calculate based on current font
#define kLineBrowser_LineDirty   1
#define kLineBrowser_LineUpdated 0

//" -+0123456789.|()RMSPeakxdB");

#define kLineBrowser_GlyphIndex_Space   0
#define kLineBrowser_GlyphIndex_Minus   1
#define kLineBrowser_GlyphIndex_Plus    2
#define kLineBrowser_GlyphIndex_0       3
#define kLineBrowser_GlyphIndex_1       4
#define kLineBrowser_GlyphIndex_2       5
#define kLineBrowser_GlyphIndex_3       6
#define kLineBrowser_GlyphIndex_4       7
#define kLineBrowser_GlyphIndex_5       8
#define kLineBrowser_GlyphIndex_6       9
#define kLineBrowser_GlyphIndex_7       10
#define kLineBrowser_GlyphIndex_8       11
#define kLineBrowser_GlyphIndex_9       12
#define kLineBrowser_GlyphIndex_Dot     13
#define kLineBrowser_GlyphIndex_Bar     14
#define kLineBrowser_GlyphIndex_R       15
#define kLineBrowser_GlyphIndex_M       16
#define kLineBrowser_GlyphIndex_S       17
#define kLineBrowser_GlyphIndex_P       18
#define kLineBrowser_GlyphIndex_e       19
#define kLineBrowser_GlyphIndex_a       20
#define kLineBrowser_GlyphIndex_k       21
#define kLineBrowser_GlyphIndex_x       22
#define kLineBrowser_GlyphIndex_d       23
#define kLineBrowser_GlyphIndex_B       24

//==============================================================================
// Class: LineBrowser
//==============================================================================
class CLineBrowser {
public:
    CLineBrowser(CDisplayMPI *displayMPI, tDisplayHandle handle,
                        int left, int top, int width, int height);
	~CLineBrowser();

    void GenerateBackground();
    void RedrawBackground(  );
    void ClearLineArea(int index, long update);

    void Update    ();
    void Clear    ();

    long GetLineCount();
     
    void SetTextFont (char *name, int size, unsigned long colorBytes_AARRGGBB);
    void SetTitleFont(char *name, int size, unsigned long colorBytes_AARRGGBB);

    void SetGradientDirection(char x);
    void SetEnableGradient   (char enable);
    void SetBackgroundColor  (unsigned long colorBytes_AARRGGBB);
    void SetBackgroundColor_Intensity(unsigned char x);

    void SetGlyphLine(long index, long update);
    void ClearLine (long index, long update);
    void DeleteLine(long index, long update);
    void SetLine   (long index, char *s, long update);
    void AppendLine(char *s, long update);

    void GenerateGlyphLibrary();

private:

	CDisplayMPI   *_pDisplayMPI;
    tDisplayHandle _handle;

    int _top, _left, _width, _height;
    tRect           _dirtyRect;
    tRect           *_pDirtyRect;
    tFontSurf     _fontSurf;

    CFontMPI	 *_pFontMPI;
    char         *_textFontName;
    tFontHndl     _textFontH;
    int           _textSize;
    unsigned long _textColor;
    S32           _textCellHeight;
    S32           _textCellWidth;

    char         *_titleFontName;
    tFontHndl     _titleFontH;
    int           _titleSize;
    unsigned long _titleColor;
    S32           _titleCellHeight;
    S32           _titleCellWidth;

    unsigned long _backgroundColor;
    unsigned char _backgroundColorIntensity;
    char _gradientDirection;
    char _gradientEnable;

    long _lineCount;
    char _lineUseGlyphLibrary[kLineBrowser_MaxLines];
    char _lineDirty[kLineBrowser_MaxLines];
    char _strings[kLineBrowser_MaxLines][500];

    U8  *_pFrameBuffer;     
    int  _frameBufferHeight, _frameBufferWidth;

    U32  *_pGlyphBuffer;     
    int  _glyphBufferHeight, _glyphBufferWidth;
    void DrawGlyphString(char *s, S32 x, S32 y);
    U32 *GetGlyphBase(char c);

    U32 *_pBackgroundBuffer32;     
    U32  _backgroundBufferBytes;     
    int  _needToRegenerateBackground;  

    int _pitch;
    int _bitsPerPixel;
    int _bytesPerPixel;
};
LF_END_BRIO_NAMESPACE()

#endif // _LINEBROWSER_H_



