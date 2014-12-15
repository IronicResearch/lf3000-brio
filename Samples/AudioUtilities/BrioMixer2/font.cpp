#include <AudioTypes.h>
#include <BrioOpenGLConfig.h>
#include <DisplayTypes.h>
#include <EmulationConfig.h>

#include <AudioMPI.h>
#include <ButtonMPI.h>
#include <DebugMPI.h>
#include <FontMPI.h>
#include <KernelMPI.h>
#include <SystemTypes.h>

#include <math.h>

LF_USING_BRIO_NAMESPACE()
#include "3d.h"
#include "font.h"

CPath GetAppRsrcFolder( void );
	
long draw3dText = false;
char *fontDisplayString = "BrioMixer";
CString gTitleS;
char *fontFileName = "COPRGTB.TTF"; //COPRGTL.TTF, Verdana.ttf
tFontSurf	gFontSurf;

// ============================================================================
// SetFont_DisplayString
// ============================================================================
    void 
SetFont_DisplayString(CFontMPI *pFont, char *s)
{
//printf("SetFont_DisplayString: set to '%s'\n", s);

fontDisplayString = s;

gTitleS = CString(fontDisplayString);

// Render font text string and download to texture
Boolean bWrap = true;
S32 x = 0;
S32 y = gFontSurf.height/3;
//CString titleS = CString(fontDisplayString);
pFont->DrawString(gTitleS, x, y, gFontSurf, bWrap);

#ifdef NEEDED
//draw3dText = true;
	if (draw3dText) 
	{
		// Set font drawing attributes 
		pFont->SetFontColor(0xFFFFFFFF); // A:R:G:B 8:8:8:8
//		attr.color = 0x0000FF00; // green
//		pFontMPI_->SelectFont(font1);
//		pFontMPI_->SetFontAttr(attr);
		
	// Render font text strings into draw surface buffer
//	CFontMPI			myFont;
//	tFontSurf			mySurf = {width, height, pitch, buffer, kPixelFormatARGB8888};
//	CString				myStr3 = CString("Jumps Over the Lazy Dog and everyone is here, everyone is here.");
	// Set font properties and drawing attributes
//	myFont.SetFontResourcePath(GetAppRsrcFolder());
//	myFont.LoadFont(myStr1, FONT_SIZE); // point size
//	myFont.SetFontColor(0xFF44CCFF); // AA:RR:GG:BB
//    long x = 0;
//    long y = HEIGHT/2+20;
//	myFont.DrawString(myStr3, x, y, mySurf, true);

		// Setup texture the OpenGL ES way, using fixed-point params as below  
		GLuint 		myTex;
		glGenTextures(1, &myTex);
		glBindTexture(GL_TEXTURE_2D, myTex);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP_TO_EDGE, GL_REPEAT
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glEnable(GL_TEXTURE_2D);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fontSurf.width, fontSurf.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontSurf.buffer);
	}
#endif

}   // ---- end SetFont_DisplayString() ----

// ============================================================================
// SetUpFont
// ============================================================================
void SetUpFont(CFontMPI *pFont)
{
//printf("SetUpFont(): start\n");

// Setup font via app resource path
CString		myFontName = CString(fontFileName);

//pFont->SetFontResourcePath(GetAppRsrcFolder());

CPath		*myFontPath;
myFontPath = pFont->GetFontResourcePath();
//printf("SetUpFont(): GetFontResourcePath='%s'\n", myFontPath->c_str());

if (kInvalidFontHndl == pFont->LoadFont(myFontName, FONT_POINT_SIZE))
    {
    printf("SetUpFont: unable to load font '%s'\n", fontFileName);
    }

// Create buffer for font rendering which will be used as texture
gFontSurf.width  = 320;
gFontSurf.height = 128;
gFontSurf.pitch  = gFontSurf.width * 4;
gFontSurf.buffer = static_cast<U8*>(malloc(gFontSurf.pitch * gFontSurf.height));
gFontSurf.format = kPixelFormatARGB8888;
memset(gFontSurf.buffer, 0, gFontSurf.pitch * gFontSurf.height);		

}   // ---- end SetUpFont() ----


